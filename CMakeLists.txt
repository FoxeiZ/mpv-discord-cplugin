include_directories(${PROJECT_SOURCE_DIR}/discord-rpc/include)

set(BUILD_EXAMPLES OFF)
add_subdirectory(discord-rpc)

add_executable(
    main
    MACOSX_BUNDLE
    main.c
)
set_target_properties(main PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Send Presence"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.discordapp.examples.main"
)
target_link_libraries(main discord-rpc)

install(
    TARGETS main
    RUNTIME
        DESTINATION "bin"
        CONFIGURATIONS Release
    BUNDLE
        DESTINATION "bin"
        CONFIGURATIONS Release
)