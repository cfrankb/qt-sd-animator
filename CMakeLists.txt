cmake_minimum_required(VERSION 3.16)
project(qt-sd-animator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SettingsManager.cpp
    src/SubprocessHandler.cpp
    src/dialogs/GeneralSettingsDialog.cpp
    src/dialogs/ModelManagerDialog.cpp
    #src/dialogs/PresetManagerDialog.cpp
    src/dialogs/NewModelDialog.cpp
    src/dialogs/SizeManagerDialog.cpp
)

set(HEADERS
    include/MainWindow.h
    include/SettingsManager.h
    include/SubprocessHandler.h
    include/dialogs/GeneralSettingsDialog.h
    include/dialogs/ModelManagerDialog.h
    #include/dialogs/PresetManagerDialog.h
    include/dialogs/NewModelDialog.h
    include/dialogs/SizeManagerDialog.h
    resources/resources.qrc
)

add_executable(qt-sd-animator
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(qt-sd-animator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(qt-sd-animator
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

install(TARGETS qt-sd-animator DESTINATION bin)
