#include "console_ui.hpp"
#include "set/set_streams.hpp"
#include "tree/tree_streams.hpp"
#include "ui_input.hpp"

#include <exception>
#include <iostream>

namespace {

void run_demo() {
    AVLTree<int> tree;
    for (int value : {30, 20, 10}) {
        tree.insert(value);
    }
    std::cout << "\nВставка 30, 20, 10: " << tree << "\nПосле балансировки BFS: ";
    tree.for_each_bfs([](const int& value) { std::cout << value << ' '; });
    std::cout << "\nСериализация: " << tree.serialize() << '\n';
    auto restored = AVLTree<int>::deserialize(tree.serialize());
    restored.erase(20);
    std::cout << "После восстановления и удаления 20: " << restored << '\n';

    Set<int> left;
    Set<int> right;
    for (int value : {1, 2, 3}) {
        left.insert(value);
    }
    for (int value : {3, 4}) {
        right.insert(value);
    }
    std::cout << "A = " << left << ", B = " << right
              << "\nA ∪ B = " << left.union_with(right)
              << "\nA ∩ B = " << left.intersection_with(right)
              << "\nA \\ B = " << left.difference(right) << '\n';
    const auto mapped = left.map([](const int& value) { return value * 2; });
    const auto filtered = mapped.where([](const int& value) { return value > 2; });
    std::cout << "map (*2): " << mapped << "\nwhere (>2): " << filtered
              << "\nreduce (сумма): " << filtered.reduce(0LL,
                    [](long long total, const int& value) { return total + value; }) << '\n';
}

}  // namespace

int run_console_ui() {
    // own data outside the menus so switching modes preserves the containers
    AVLTree<int> tree;
    std::array<Set<int>, 3> sets;
    try {
        while (true) {
            std::cout << "\nЛР №1 — AVLTree и Set, тип элементов int\n"
                      << "1 Дерево  2 Множества  3 Автоматический пример  0 Выход\n";
            const int action = ui::read_choice("Режим: ", 0, 3);
            try {
                switch (action) {
                case 0:
                    return 0;
                case 1:
                    run_tree_ui(tree);
                    break;
                case 2:
                    run_set_ui(sets);
                    break;
                case 3:
                    run_demo();
                    break;
                }
            } catch (const std::exception& error) {
                std::cout << "Ошибка: " << error.what() << '\n';
            }
        }
    } catch (const ui::InputEnded&) {
        std::cout << "\nВвод завершён\n";
        return 0;
    }
}
