add_library(nds_lab1_avl_tree INTERFACE)

target_include_directories(nds_lab1_avl_tree INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_features(nds_lab1_avl_tree INTERFACE cxx_std_20)

add_library(nds_lab1_set INTERFACE)

target_link_libraries(nds_lab1_set INTERFACE nds_lab1_avl_tree)
