// Run me `clang++-14 -std=c++17 -g -O0 file_ideas.cpp -obuild/fi && ./build/fi foo`

#include <iostream>
#include <ostream>
#include <vector>
#include <string>
#include <thread>
#include <map>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <memory>

std::atomic<uint32_t> global_time = { 0 };

struct Write {
    uint32_t time;
    uint32_t pos;
    std::string text;

    Write(const std::string& s, uint32_t p) : time(0), pos(p), text(s) {
        time = global_time.fetch_add(1, std::memory_order_relaxed);
    }

    bool operator<(const Write& b) const & {
        return time < b.time;
    }

    friend std::ostream& operator<<(std::ostream& os, const Write& self) {
        os << "  Write { time: " << self.time <<
            ", pos: " << self.pos <<
            ", text: \"" << std::string(self.text).replace(self.text.find("\n"), std::string("\n").size(), "\\n") << "\" }";
        return os;
    }
};


struct Str {
    // We would need locking around inserting "writes" into a sorted list
    // They would be sorted based on when they happened/position
    std::mutex write_lock;
    std::vector<Write> ordered_writes;

    Str() : write_lock(), ordered_writes() {}

    void write(const std::string& s, uint32_t pos) {
        auto wr = Write(s, pos);
        {
            std::lock_guard<std::mutex> scope(write_lock);
            auto idx = std::lower_bound(ordered_writes.begin(), ordered_writes.end(), wr);
            ordered_writes.insert(idx, wr);
        }
    }

    std::string to_string() {
        std::string concat;
        for (const auto& wr : ordered_writes) {
            concat.append(wr.text);
        }
        return concat;
    }

    friend std::ostream& operator<<(std::ostream& os, const Str& self) {
        os << "Str {\n";
        for (const auto& wr : self.ordered_writes) {
            os << "  " << wr << "\n";
        }
        os << "}";

        return os;
    }
};


struct Policy {
    std::map<std::thread::id, Str*> str_tree;

    Policy() : str_tree() {}

    void open_file(std::thread::id pid) {
        str_tree.insert_or_assign(pid, new Str());
    }

    void write_file(std::thread::id pid, const std::string& txt, uint32_t pos) {
        auto s = str_tree.find(pid);
        if (s != str_tree.end()) {
            s->second->write(txt, pos);
        } else {
            std::cout << "Write failed\n" << pid << " " << txt << "at " << pos;
        }
    }

    void close_file(std::thread::id pid) {
        std::cout << *this;
        auto s = str_tree.find(pid);
        if (s != str_tree.end()) {
            std::cout << s->second->to_string();
        } else {
            std::cout << "Close failed for " << pid;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Policy& self) {
        os << "Policy {\n";
        for (const auto& p : self.str_tree) {
            const auto& [k, v] = p;
            os << "  " << k << ": " << *v << "\n";
        }
        os << "}";

        return os;
    }
};


struct Tuple {
    int chunks;
    int start;

    std::thread::id pid;
    std::string file;
};

std::map<std::string, Policy> policies = { {"foo", Policy()} };

constexpr int FACTOR = 4;
constexpr int NUM_THREADS = 32;

const std::vector<std::string> TO_WRITE = {
    "yo 01\n", "yo 02\n", "yo 03\n", "yo 04\n",
    "yo 05\n", "yo 06\n", "yo 07\n", "yo 08\n",
    "yo 09\n", "yo 10\n", "yo 11\n", "yo 12\n",
    "yo 13\n", "yo 14\n", "yo 15\n", "yo 16\n",
    "yo 17\n", "yo 18\n", "yo 19\n", "yo 20\n",
    "yo 21\n", "yo 22\n", "yo 23\n", "yo 24\n",
    "yo 25\n", "yo 26\n", "yo 27\n", "yo 28\n",
    "yo 29\n", "yo 30\n", "yo 31\n", "yo 32\n"
};



void thread_func(Tuple* pair) {
    int cnt = pair->chunks;

    int start = pair->start * cnt;
    int end = start + cnt;

    for (int i = start; i < end; ++i) {
        auto pol = policies.find(pair->file);
        if (pol != policies.end()) {
            pol->second.write_file(pair->pid, TO_WRITE[i], i * TO_WRITE[0].size());
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Must pass a valid file name to run, got " << argv[0] << "\n";
        return -1;
    }

    char* path = argv[1];
    std::vector<std::thread> threads;

    int num_cpus = std::thread::hardware_concurrency();
    int div = (num_cpus <= FACTOR) ? 1 : FACTOR;
    int threads_to_spawn = num_cpus / div;

    int num_chunks = NUM_THREADS / threads_to_spawn;
    std::cout << "chunks = " << num_chunks << " threads = " << threads_to_spawn << "\n";

    auto curr_id = std::this_thread::get_id();
    // This is the open on one thread and write on different threads
    policies.find(path)->second.open_file(curr_id);

    for (int i = 0; i < threads_to_spawn; ++i) {
        Tuple* foo = new Tuple;
        foo->chunks = num_chunks;
        foo->start = i;
        foo->pid = curr_id;
        foo->file = path;

        threads.emplace_back(thread_func, foo);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // We verify the file on close it looks like no writes have happened
    // since they are all on different PIDs
    policies.find(path)->second.close_file(curr_id);

    return 0;
}
