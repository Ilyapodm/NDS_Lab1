#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace ui {

// eof must leave every nested menu instead of being treated as an invalid number
class InputEnded {};

inline std::string read_line(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw InputEnded{};
    }
    return line;
}

template <typename T>
T read_value(const std::string& prompt) {
    while (true) {
        std::istringstream input(read_line(prompt));
        T value{};
        if (input >> value) {
            input >> std::ws;
            if (input.eof()) {
                return value;
            }
        }
        std::cout << "Введите одно целое число в допустимом диапазоне\n";
    }
}

inline int read_choice(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        const int choice = read_value<int>(prompt);
        if (choice >= minimum && choice <= maximum) {
            return choice;
        }
        std::cout << "Допустимые значения: " << minimum << "–" << maximum << '\n';
    }
}

}  // namespace ui
