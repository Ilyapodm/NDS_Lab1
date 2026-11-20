#include "console_ui.hpp"
#include "set/set_streams.hpp"
#include "ui_actions.hpp"

#include <array>
#include <exception>
#include <iostream>
#include <utility>

void run_set_ui(std::array<Set<int>, 3>& sets) {
    std::size_t selected = 0;
    const std::array<const char*, 3> names{"A", "B", "R"};
    while (true) {
        std::cout << "\nSet<int>, активное " << names[selected]
                  << ", элементов: " << sets[selected].size() << '\n';
        ui::print_common_menu();
        std::cout << "20 Выбрать A/B/R  21 R = A ∪ B  22 R = A ∩ B  23 R = A \\ B\n"
                  << "24 Проверить включение  25 Проверить равенство\n"
                  << "26 Показать A/B/R  27 Поменять A и B местами\n";
        const int action = ui::read_value<int>("Действие: ");
        if (action == 0) {
            return;
        }
        try {
            if (ui::handle_common_action(action, sets[selected])) {
                continue;
            }
            // binary operations always use a as the left operand and b as the right
            // store the result separately so the two operands remain available
            switch (action) {
            case 20:
                selected = static_cast<std::size_t>(ui::read_choice(
                    "1 A  2 B  3 R: ", 1, 3) - 1);
                break;
            case 21:
                sets[2] = sets[0].union_with(sets[1]);
                std::cout << "R = " << sets[2] << '\n';
                break;
            case 22:
                sets[2] = sets[0].intersection_with(sets[1]);
                std::cout << "R = " << sets[2] << '\n';
                break;
            case 23:
                sets[2] = sets[0].difference(sets[1]);
                std::cout << "R = " << sets[2] << '\n';
                break;
            case 24:
                std::cout << "A ⊆ B: " << (sets[0].is_subset_of(sets[1]) ? "да" : "нет")
                          << ", B ⊆ A: " << (sets[1].is_subset_of(sets[0]) ? "да" : "нет") << '\n';
                break;
            case 25:
                std::cout << "A == B: " << (sets[0] == sets[1] ? "да" : "нет") << '\n';
                break;
            case 26:
                for (std::size_t index = 0; index < sets.size(); ++index) {
                    std::cout << names[index] << " = " << sets[index] << '\n';
                }
                break;
            case 27:
                std::swap(sets[0], sets[1]);
                std::cout << "A и B поменяны местами\n";
                break;
            default:
                std::cout << "Неизвестное действие\n";
            }
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}
