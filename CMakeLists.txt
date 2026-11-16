cmake_minimum_required(VERSION 3.21)

project(nds_lab1 LANGUAGES CXX)

# Библиотеки доступны и при самостоятельной сборке, и при подключении проекта.
add_subdirectory(include)

# При подключении через add_subdirectory доступны только библиотеки.
if(PROJECT_IS_TOP_LEVEL)
    # Консольное приложение и бенчмарк настраиваются в своих каталогах.
    add_subdirectory(src)
    add_subdirectory(benchmarks)

    # CTest добавляет опцию BUILD_TESTING, включённую по умолчанию.
    include(CTest)

    # При BUILD_TESTING=OFF GoogleTest не требуется.
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
