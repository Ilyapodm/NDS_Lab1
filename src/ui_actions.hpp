#pragma once

#include "ui_input.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ui {

inline void print_common_menu() {
    std::cout
        << "1 Показать  2 Вставить  3 Найти  4 Удалить  5 Размер  6 Очистить\n"
        << "7 Ручное заполнение  8 Автозаполнение\n"
        << "9 map (заменить)  10 where (заменить)  11 reduce (сумма)\n"
        << "12 Сериализовать  13 Восстановить из строки\n"
        << "14 Сохранить в файл  15 Загрузить из файла  0 Назад\n";
}

inline int checked_map(int value, int operand, int operation) {
    // calculate in a wider type so signed int overflow cannot cause undefined behavior
    const long long result = operation == 1
        ? static_cast<long long>(value) + operand
        : static_cast<long long>(value) * operand;
    if (result < std::numeric_limits<int>::min()
        || result > std::numeric_limits<int>::max()) {
        throw std::overflow_error("результат map выходит за диапазон int");
    }
    return static_cast<int>(result);
}

template <typename Container>
void fill_manually(Container& container) {
    std::istringstream input(read_line("Новые значения через пробел (пустая строка очищает): "));
    // preserve the old container if any value in the line is malformed
    Container replacement;
    input >> std::ws;
    while (input.peek() != std::char_traits<char>::eof()) {
        int value;
        if (!(input >> value)) {
            throw std::invalid_argument("ожидался список целых чисел");
        }
        replacement.insert(value);
        input >> std::ws;
    }
    container = std::move(replacement);
    std::cout << "Загружено уникальных значений: " << container.size() << '\n';
}

template <typename Container>
void fill_automatically(Container& container) {
    const int count = read_choice("Количество (неотрицательное): ",
                                  0, std::numeric_limits<int>::max());
    const int order = read_choice("1 Случайный  2 Возрастающий  3 Убывающий: ", 1, 3);
    std::vector<int> values(static_cast<std::size_t>(count));
    std::iota(values.begin(), values.end(), 0);
    if (order == 1) {
        std::mt19937 generator(2026);
        std::shuffle(values.begin(), values.end(), generator);
    } else if (order == 3) {
        std::reverse(values.begin(), values.end());
    }

    Container replacement;
    for (int value : values) {
        replacement.insert(value);
    }
    container = std::move(replacement);
    std::cout << "Создано элементов: " << container.size() << '\n';
}

template <typename Container>
void save_file(const Container& container) {
    const std::string path = read_line("Путь к файлу (существующий будет перезаписан): ");
    const auto data = container.serialize();
    std::ofstream output;
    output.exceptions(std::ios::failbit | std::ios::badbit);
    output.open(path);
    output << data << '\n';
    output.close();
    std::cout << "Сохранено\n";
}

template <typename Container>
void load_file(Container& container) {
    std::ifstream input(read_line("Путь к файлу: "));
    if (!input) {
        throw std::runtime_error("не удалось открыть файл");
    }
    std::ostringstream data;
    data << input.rdbuf();
    if (input.bad()) {
        throw std::runtime_error("ошибка чтения файла");
    }
    // deserialize completes before assignment, preserving the old data on a parse error
    container = Container::deserialize(data.str());
    std::cout << "Загружено элементов: " << container.size() << '\n';
}

template <typename Container>
bool handle_common_action(int action, Container& container) {
    switch (action) {
    case 1:
        std::cout << container << '\n';
        break;
    case 2:
        std::cout << (container.insert(read_value<int>("Значение: "))
            ? "Добавлено\n" : "Такой ключ уже есть\n");
        break;
    case 3: {
        const auto found = container.find(read_value<int>("Ключ: "));
        if (found) {
            std::cout << "Найдено: " << found->get() << '\n';
        } else {
            std::cout << "Ключ отсутствует\n";
        }
        break;
    }
    case 4:
        std::cout << (container.erase(read_value<int>("Ключ: "))
            ? "Удалено\n" : "Ключ отсутствует\n");
        break;
    case 5:
        std::cout << "Размер: " << container.size()
                  << ", пуст: " << (container.empty() ? "да" : "нет") << '\n';
        break;
    case 6:
        container.clear();
        std::cout << "Очищено\n";
        break;
    case 7:
        fill_manually(container);
        break;
    case 8:
        fill_automatically(container);
        break;
    case 9: {
        const int operation = read_choice("1 Прибавить число  2 Умножить на число: ", 1, 2);
        const int operand = read_value<int>("Число: ");
        container = container.map([&](const int& value) {
            return checked_map(value, operand, operation);
        });
        std::cout << "Результат map: " << container << '\n';
        break;
    }
    case 10: {
        const int condition = read_choice("1 Больше границы  2 Меньше границы  3 Чётные: ", 1, 3);
        int boundary = 0;
        if (condition != 3) {
            boundary = read_value<int>("Граница: ");
        }
        container = container.where([&](const int& value) {
            if (condition == 1) {
                return value > boundary;
            }
            if (condition == 2) {
                return value < boundary;
            }
            return value % 2 == 0;
        });
        std::cout << "Результат where: " << container << '\n';
        break;
    }
    case 11:
        std::cout << "Сумма: " << container.reduce(0LL,
            [](long long total, const int& value) { return total + value; }) << '\n';
        break;
    case 12:
        std::cout << container.serialize() << '\n';
        break;
    case 13:
        container = Container::deserialize(read_line("Сериализованная строка: "));
        std::cout << "Восстановлено элементов: " << container.size() << '\n';
        break;
    case 14:
        save_file(container);
        break;
    case 15:
        load_file(container);
        break;
    default:
        return false;
    }
    return true;
}

}  // namespace ui
