#include "benchmark_utils.hpp"
#include "tree/avl_tree.hpp"

#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

namespace {

void write_measurement(std::ostream& output, std::size_t size,
                       const std::string& order, std::size_t repeat,
                       const std::string& operation, std::size_t operations,
                       double elapsed_ms) {
    // store raw repetitions so the median can be independently recomputed in python
    // normalization makes batches with different counts comparable on operation plots
    output << size << ',' << order << ',' << repeat << ',' << operation << ','
           << operations << ',' << elapsed_ms << ','
           << elapsed_ms * 1000000.0 / static_cast<double>(operations) << '\n';
}

void run_repeat(std::ostream& output, std::size_t size,
                const std::string& order, std::size_t repeat) {
    const auto input = benchmark::make_input(size, order, static_cast<std::uint32_t>(repeat));
    AVLTree<int> tree;
    std::size_t completed = 0;

    const double build_ms = benchmark::measure_ms([&] {
        for (int value : input.values) {
            completed += tree.insert(value);
        }
    });
    // validate outside the timer, and consume results so searches cannot be discarded
    if (completed != size || tree.size() != size) {
        throw std::runtime_error("invalid tree after construction");
    }
    write_measurement(output, size, order, repeat, "build", size, build_ms);

    completed = 0;
    const double successful_ms = benchmark::measure_ms([&] {
        for (int key : input.successful_queries) {
            completed += tree.contains(key);
        }
    });
    if (completed != input.successful_queries.size()) {
        throw std::runtime_error("an existing key was not found");
    }
    write_measurement(output, size, order, repeat, "find_hit",
                      input.successful_queries.size(), successful_ms);

    completed = 0;
    const double missing_ms = benchmark::measure_ms([&] {
        for (int key : input.missing_queries) {
            completed += tree.contains(key);
        }
    });
    if (completed != 0) {
        throw std::runtime_error("an absent key was found");
    }
    write_measurement(output, size, order, repeat, "find_miss",
                      input.missing_queries.size(), missing_ms);

    // measure insertion into an already populated tree separately from construction
    completed = 0;
    const double insert_ms = benchmark::measure_ms([&] {
        for (int value : input.inserted_values) {
            completed += tree.insert(value);
        }
    });
    const auto batch_size = input.inserted_values.size();
    if (completed != batch_size || tree.size() != size + batch_size) {
        throw std::runtime_error("invalid tree after insertion");
    }
    write_measurement(output, size, order, repeat, "insert", batch_size, insert_ms);

    // erase original keys in dataset order, covering old nodes rather than just new leaves
    // the tree starts this batch with n + batch_size elements and finishes with n
    completed = 0;
    const double erase_ms = benchmark::measure_ms([&] {
        for (std::size_t index = 0; index < batch_size; ++index) {
            completed += tree.erase(input.values[index]);
        }
    });
    if (completed != batch_size || tree.size() != size) {
        throw std::runtime_error("invalid tree after erasure");
    }
    write_measurement(output, size, order, repeat, "erase", batch_size, erase_ms);
    output.flush();

    // tree destruction and input cleanup take place after all five timed batches
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc > 1 && std::string(argv[1]) == "--help") {
            std::cout << "usage: bench [output.csv] [repeats] [sizes...]\n"
                      << "default: benchmarks/results/avl.csv, 3 repeats, "
                      << "10000 30000 100000 300000 1000000 3000000 10000000\n";
            return 0;
        }
#ifndef NDEBUG
        // debug builds would measure disabled optimizations and distort the experiment
        throw std::runtime_error("configure a Release build before running bench");
#endif
        const std::filesystem::path output_path =
            argc > 1 ? argv[1] : "benchmarks/results/avl.csv";
        const std::size_t repeats = argc > 2
            ? benchmark::parse_positive_number(argv[2]) : 3;
        std::vector<std::size_t> sizes;
        for (int index = 3; index < argc; ++index) {
            sizes.push_back(benchmark::parse_positive_number(argv[index]));
        }
        if (sizes.empty()) {
            sizes = {10000, 30000, 100000, 300000, 1000000, 3000000, 10000000};
        }

        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path());
        }
        std::ofstream output;
        // turn file errors into exceptions instead of silently losing measurements
        output.exceptions(std::ios::failbit | std::ios::badbit);
        output.open(output_path);
        output.imbue(std::locale::classic());
        output << std::setprecision(12);
        output << "n,order,repeat,operation,operations,total_ms,ns_per_operation\n";

        std::cout << "sizeof(Node<int>)=" << sizeof(Node<int>)
                  << ", seed=" << benchmark::seed << ", repeats=" << repeats << '\n';
        const std::array<std::string, 3> orders{"random", "ascending", "descending"};
        for (std::size_t size : sizes) {
            for (const auto& order : orders) {
                for (std::size_t repeat = 0; repeat < repeats; ++repeat) {
                    std::cout << "n=" << size << " order=" << order
                              << " repeat=" << repeat + 1 << '/' << repeats << std::endl;
                    run_repeat(output, size, order, repeat);
                }
            }
        }
        output.close();
        std::cout << "saved " << output_path << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "benchmark failed: " << error.what() << '\n';
        return 1;
    }
}
