# Используем установленный GoogleTest, если он доступен.
find_package(GTest CONFIG QUIET)

if(NOT TARGET GTest::gtest_main)
    # FetchContent скачивает и добавляет зависимость при конфигурации.
    include(FetchContent)

    # При смене архива распакованные файлы получают новые метки времени.
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()

    # На MSVC используем ту же динамическую библиотеку времени выполнения.
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "Use the shared MSVC runtime for GoogleTest")
    endif()

    # Фиксируем версию GoogleTest и проверяем контрольную сумму архива.
    FetchContent_Declare(googletest
        URL https://codeload.github.com/google/googletest/tar.gz/refs/tags/v1.17.0
        URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
    )
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(tests
    test_avl_tree.cpp
    test_serialization.cpp
    test_set.cpp
    test_containers.cpp
)

# GTest::gtest_main предоставляет точку входа main для тестов
target_link_libraries(tests PRIVATE nds_lab1_set GTest::gtest_main)

# Подключаем функцию обнаружения тестов GoogleTest.
include(GoogleTest)

# Регистрируем отдельные тесты в CTest.
gtest_discover_tests(tests)
