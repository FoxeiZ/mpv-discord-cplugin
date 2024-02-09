cmake_minimum_required(VERSION 3.9)
project(DiscordRPC)

include_directories(${CMAKE_SYSTEM_INCLUDE_PATH})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/discord-rpc/include)

set(BUILD_EXAMPLES OFF)
set (BUILD_SHARED_LIBS ON)
add_subdirectory(discord-rpc)

add_library(
    main SHARED
    main.c
)

set_target_properties(main PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Send Presence"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.discordapp.examples.main"
)

target_link_libraries(main discord-rpc)
target_compile_options(main PRIVATE -fPIC)

install(
    TARGETS main
    EXPORT "main"
    LIBRARY
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)