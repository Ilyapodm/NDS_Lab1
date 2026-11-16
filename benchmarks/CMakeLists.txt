add_executable(bench benchmark.cpp)

# оба контейнера через зависимость Set от AVLTree
target_link_libraries(bench PRIVATE nds_lab1_set)
