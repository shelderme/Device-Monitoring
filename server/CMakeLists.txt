cmake_minimum_required(VERSION 3.16)
project(Server LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Gui)



set(SERVER_SOURCES
    main.cpp
    mainwindow.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/ui_mainwindow.h
)

set(SERVER_HEADERS
    mainwindow.h
)


add_executable(Server
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
)

target_link_libraries(Server
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        Qt6::Gui
)


qt_wrap_ui(SERVER_UI_FORMS
    mainwindow.ui
)
target_sources(Server PRIVATE ${SERVER_UI_FORMS})


set_target_properties(Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Для Windows
if(WIN32)
    add_custom_command(TARGET Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::Core>"
            "$<TARGET_FILE_DIR:Server>"
    )
    add_custom_command(TARGET Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::Network>"
            "$<TARGET_FILE_DIR:Server>"
    )
    add_custom_command(TARGET Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::Widgets>"
            "$<TARGET_FILE_DIR:Server>"
    )
    add_custom_command(TARGET Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::Gui>"
            "$<TARGET_FILE_DIR:Server>"
    )
endif()
