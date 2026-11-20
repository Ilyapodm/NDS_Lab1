#include "console_ui.hpp"
#include "tree/tree_streams.hpp"
#include "ui_actions.hpp"

#include <array>
#include <exception>
#include <iostream>
#include <string>

namespace {

TraversalOrder read_order() {
    const std::array orders{
        TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LPK,
        TraversalOrder::LKP, TraversalOrder::PLK, TraversalOrder::PKL
    };
    const int choice = ui::read_choice(
        "1 КЛП  2 КПЛ  3 ЛПК  4 ЛКП  5 ПЛК  6 ПКЛ: ", 1, 6);
    return orders[static_cast<std::size_t>(choice - 1)];
}

}  // namespace

void run_tree_ui(AVLTree<int>& tree) {
    while (true) {
        std::cout << "\nAVLTree<int>, элементов: " << tree.size() << '\n';
        ui::print_common_menu();
        std::cout << "20 DFS  21 BFS  22 reduce (строка в выбранном DFS-порядке)\n";
        const int action = ui::read_value<int>("Действие: ");
        if (action == 0) {
            return;
        }
        try {
            if (ui::handle_common_action(action, tree)) {
                continue;
            }
            switch (action) {
            case 20:
                tree.for_each(read_order(), [](const int& value) {
                    std::cout << value << ' ';
                });
                std::cout << '\n';
                break;
            case 21:
                std::cout << "BFS: ";
                tree.for_each_bfs([](const int& value) {
                    std::cout << value << ' ';
                });
                std::cout << '\n';
                break;
            case 22: {
                const auto order = read_order();
                const auto joined = tree.reduce(std::string{},
                    [](std::string text, const int& value) {
                        return text + std::to_string(value) + " ";
                    }, order);
                std::cout << "Свёртка: " << joined << '\n';
                break;
            }
            default:
                std::cout << "Неизвестное действие\n";
            }
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}
