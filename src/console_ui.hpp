#pragma once

#include "set/set.hpp"
#include "tree/avl_tree.hpp"

#include <array>

int run_console_ui();
void run_tree_ui(AVLTree<int>& tree);
void run_set_ui(std::array<Set<int>, 3>& sets);
