find_package(GTest CONFIG REQUIRED)

add_executable(tests
    test_avl_tree.cpp
    test_set.cpp
    test_containers.cpp
)

# GTest::gtest_main предоставляет точку входа main для тестов
target_link_libraries(tests PRIVATE nds_lab1_set GTest::gtest_main)

# Подключаем функцию обнаружения тестов GoogleTest.
include(GoogleTest)

# Регистрируем отдельные тесты в CTest.
gtest_discover_tests(tests)
