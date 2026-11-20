# keep the main menu and the two container screens in separate source files
add_executable(lab_ui
    main.cpp
    console_ui.cpp
    tree_ui.cpp
    set_ui.cpp
)

# set transitively supplies the avl headers and the c++20 requirement
target_link_libraries(lab_ui PRIVATE nds_lab1_set)
