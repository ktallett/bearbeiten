cmake_minimum_required(VERSION 3.16)
project(bearbeiten VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt6_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/codeeditor.cpp
    src/codeeditor.h
    src/languageloader.cpp
    src/languageloader.h
    src/jsonsyntaxhighlighter.cpp
    src/jsonsyntaxhighlighter.h
    src/finddialog.cpp
    src/finddialog.h
    src/gotolinedialog.cpp
    src/gotolinedialog.h
    src/symbolsearchdialog.cpp
    src/symbolsearchdialog.h
    src/symbolextractor.cpp
    src/symbolextractor.h
    src/outlinepanel.cpp
    src/outlinepanel.h
    src/projectpanel.cpp
    src/projectpanel.h
    src/minimap.cpp
    src/minimap.h
    src/breadcrumbbar.cpp
    src/breadcrumbbar.h
    src/characterinspector.cpp
    src/characterinspector.h
    src/encodingmanager.cpp
    src/encodingmanager.h
)

qt6_add_executable(bearbeiten ${SOURCES})

qt6_add_resources(bearbeiten "resources"
    PREFIX "/"
    FILES
        src/stylesheet.qss
        src/stylesheet-dark.qss
)

target_link_libraries(bearbeiten PRIVATE Qt6::Core Qt6::Widgets)

# Translation support
set(TS_FILES
    translations/bearbeiten_de.ts
    translations/bearbeiten_fr.ts
    translations/bearbeiten_es.ts
    translations/bearbeiten_it.ts
    translations/bearbeiten_ja.ts
    translations/bearbeiten_zh.ts
    translations/bearbeiten_ru.ts
)

qt6_add_translations(bearbeiten TS_FILES ${TS_FILES})

# Create translations directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/translations)

set_target_properties(bearbeiten PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)