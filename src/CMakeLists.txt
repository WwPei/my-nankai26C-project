set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(emoji_dungeon
    MANUAL_FINALIZATION
    main.cpp
    emoji_dungeon_window.cpp
    emoji_dungeon_window.h
    class_select_page.cpp
    class_select_page.h
    game_main_page.cpp
    game_main_page.h
    upgrade_page.cpp
    upgrade_page.h
    game_factory.cpp
    game_factory.h
    game_data.h
    wave_manager.h
    game_view.h
    player.h
    weapon.h
    enemy_data.h
    enemy.h
    bullet_data.h
    bullet.h
    trait.h
    resources.qrc
)

qt_add_executable(stage1_system_test
    MANUAL_FINALIZATION
    stage1_system_test.cpp
    emoji_dungeon_window.cpp
    emoji_dungeon_window.h
    class_select_page.cpp
    class_select_page.h
    game_main_page.cpp
    game_main_page.h
    upgrade_page.cpp
    upgrade_page.h
    game_factory.cpp
    game_factory.h
    game_data.h
    wave_manager.h
    game_view.h
    player.h
    weapon.h
    enemy_data.h
    enemy.h
    bullet_data.h
    bullet.h
    trait.h
    resources.qrc
)

target_link_libraries(emoji_dungeon
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_link_libraries(stage1_system_test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
)

set_target_properties(emoji_dungeon PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

include(GNUInstallDirs)
install(TARGETS emoji_dungeon
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_TESTING)
    add_test(NAME stage1_system_test COMMAND stage1_system_test)
endif()

qt_finalize_executable(emoji_dungeon)
qt_finalize_executable(stage1_system_test)