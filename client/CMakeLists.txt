cmake_minimum_required(VERSION 3.16)
project(Client LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)


set(CLIENT_SOURCES
    client.cpp
    main.cpp
)

add_executable(Client ${CLIENT_SOURCES})


target_link_libraries(Client
    PRIVATE
        Qt6::Core
        Qt6::Network
)

set_target_properties(Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# при сборке под Windows
if(WIN32)
    add_custom_command(TARGET Client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::Core>"
            "$<TARGET_FILE_DIR:TelecomClient>"
    )
    add_custom_command(TARGET Client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::Network>"
            "$<TARGET_FILE_DIR:TelecomClient>"
    )
endif()
