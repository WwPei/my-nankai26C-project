cmake_minimum_required(VERSION 3.16)

project(emoji_dungeon VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

qt_standard_project_setup(REQUIRES 6.10)

set(SHARED_SOURCES
    src/pages/emoji_dungeon_window.cpp
    src/pages/emoji_dungeon_window.h
    src/pages/class_select_page.cpp
    src/pages/class_select_page.h
    src/pages/game_main_page.cpp
    src/pages/game_main_page.h
    src/pages/upgrade_page.cpp
    src/pages/upgrade_page.h
    src/systems/game_factory.cpp
    src/systems/game_factory.h
    src/core/game_enums.h
    src/core/game_structs.h
    src/core/game_tables.h
    src/core/game_data.h
    src/systems/wave_manager.cpp
    src/systems/wave_manager.h
    src/core/combat_utils.h
    src/core/combat_utils.cpp
    src/views/dash_cooldown_widget.h
    src/views/dash_cooldown_widget.cpp
    src/models/basic/basic_bullet_data.h
    src/models/basic/basic_bullet_data.cpp
    src/views/basic_bullet_view.h
    src/views/basic_bullet_view.cpp
    src/models/basic/basic_enemy_data.h
    src/models/basic/basic_enemy_data.cpp
    src/views/basic_enemy_view.h
    src/views/basic_enemy_view.cpp
    src/models/basic/basic_player.h
    src/models/basic/basic_player.cpp
    src/models/basic/basic_trait.h
    src/models/basic/basic_trait.cpp
    src/models/basic/basic_weapon.h
    src/models/basic/basic_weapon.cpp
    src/views/battle_arena_view.h
    src/views/battle_arena_view.cpp
    src/views/player_avatar_item.h
    src/views/player_avatar_item.cpp
    src/models/interfaces/game_view.h
    src/models/interfaces/player.h
    src/models/interfaces/weapon.h
    src/models/interfaces/enemy_data.h
    src/views/interfaces/enemy.h
    src/models/interfaces/bullet_data.h
    src/views/interfaces/bullet.h
    src/models/interfaces/trait.h
    src/systems/upgrade_resolver.h
    src/systems/upgrade_resolver.cpp
    src/systems/combat_coordinator.h
    src/systems/combat_coordinator.cpp
    src/systems/enemy_director.h
    src/systems/enemy_director.cpp
    src/bullets/special_bullet_data.h
    src/bullets/special_bullet_data.cpp
    src/bullets/special_bullet_view.h
    src/bullets/special_bullet_view.cpp
    src/bullets/special_bullet_config.h
    resources/resources.qrc
)

qt_add_executable(emoji_dungeon
    MANUAL_FINALIZATION
    src/main.cpp
    ${SHARED_SOURCES}
)

target_include_directories(emoji_dungeon PRIVATE
    src/core
    src/models/interfaces
    src/models/basic
    src/views/interfaces
    src/views
    src/pages
    src/systems
    src/bullets
)

target_link_libraries(emoji_dungeon
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/app_icon_res.o"
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
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

qt_finalize_executable(emoji_dungeon)