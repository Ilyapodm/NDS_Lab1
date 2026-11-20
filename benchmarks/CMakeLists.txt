# build the benchmark as a separate executable
add_executable(bench benchmark.cpp)

# inherit the container headers and the c++20 requirement from its library target
target_link_libraries(bench PRIVATE nds_lab1_avl_tree)
