#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace benchmark {

inline constexpr std::uint32_t seed = 2026;
inline constexpr std::size_t query_count = 100000;
inline constexpr std::size_t mutation_count = 10000;

struct InputData {
    std::vector<int> values;
    std::vector<int> successful_queries;
    std::vector<int> missing_queries;
    std::vector<int> inserted_values;
};

inline std::size_t parse_positive_number(const std::string& text) {
    // reject signs and partial numbers instead of accepting inputs such as "100abc"
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("expected a positive integer: " + text);
    }
    const auto value = std::stoull(text);
    if (value == 0 || value > 100000000) {
        throw std::invalid_argument("number must be between 1 and 100000000");
    }
    return static_cast<std::size_t>(value);
}

inline void apply_order(std::vector<int>& values, const std::string& order,
                        std::mt19937& generator) {
    if (order == "random") {
        std::shuffle(values.begin(), values.end(), generator);
    } else if (order == "descending") {
        std::reverse(values.begin(), values.end());
    }
    // ascending data is already sorted by iota
}

inline InputData make_input(std::size_t size, const std::string& order,
                            std::uint32_t repeat) {
    InputData input;
    // every repeat has a reproducible input and all data generation precedes timing
    std::mt19937 generator(seed + repeat);
    input.values.resize(size);
    std::iota(input.values.begin(), input.values.end(), 0);
    apply_order(input.values, order, generator);

    std::uniform_int_distribution<int> existing(0, static_cast<int>(size - 1));
    std::uniform_int_distribution<int> missing(static_cast<int>(size),
                                              static_cast<int>(2 * size - 1));
    input.successful_queries.reserve(query_count);
    input.missing_queries.reserve(query_count);
    for (std::size_t index = 0; index < query_count; ++index) {
        input.successful_queries.push_back(existing(generator));
        input.missing_queries.push_back(missing(generator));
    }

    // added keys lie above the original range and cannot already be present
    input.inserted_values.resize(std::min(size, mutation_count));
    std::iota(input.inserted_values.begin(), input.inserted_values.end(),
              static_cast<int>(size));
    apply_order(input.inserted_values, order, generator);
    return input;
}

template <typename Operation>
double measure_ms(Operation operation) {
    // steady_clock measures elapsed time without wall-clock adjustments
    const auto start = std::chrono::steady_clock::now();
    operation();
    const auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

}  // namespace benchmark
