cmake_minimum_required(VERSION 3.12)

# Can be one of thoese: address, thread, memory, undefined, leak
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

project(gitmaster VERSION 0.4 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(Qt6 COMPONENTS Gui Widgets Network REQUIRED)

set(PROJECT_HEADERS
    qgitmastermainwindow.h
    qgitrepository.h
    qnewrepositorydialog.h
    qgitclonerepositorydialog.h
    qgitpushdialog.h
    qgitpulldialog.h
    qgitfetchdialog.h

    qgitrepotreeitemdelegate.h
    qgitrepotreewidget.h
    qgitdiffwidget.h
    qadvplaintextedit.h
    qloghistoryitemdelegate.h
    qloghistorytablewidget.h
    qexttextbrowser.h

    combobox/qcomboboxgitdiffoptions.h
    combobox/qcomboboxgitstatusfiles.h
    combobox/qcomboboxgitviewoptions.h

    qgit/include/qgit.h
    qgit/include/qgitbranch.h
    qgit/include/qgitcommit.h
    qgit/include/qgitcommitdiffparent.h
    qgit/include/qgitdiffbinary.h
    qgit/include/qgitdiffbinaryfile.h
    qgit/include/qgitdifffile.h
    qgit/include/qgitdifffileitem.h
    qgit/include/qgitdiffhunk.h
    qgit/include/qgitdiffline.h
    qgit/include/qgiterror.h
    qgit/include/qgitsignature.h
)

set(PROJECT_SOURCES
    main.cpp

    qgitmastermainwindow.cpp      qgitmastermainwindow.ui
    qgitrepository.cpp            qgitrepository.ui
    qnewrepositorydialog.cpp      qnewrepositorydialog.ui
    qgitclonerepositorydialog.cpp qgitclonerepositorydialog.ui
    qgitpushdialog.cpp            qgitpushdialog.ui
    qgitpulldialog.cpp            qgitpulldialog.ui
    qgitfetchdialog.cpp           qgitfetchdialog.ui

    qgitrepotreeitemdelegate.cpp
    qgitrepotreewidget.cpp
    qgitdiffwidget.cpp
    qadvplaintextedit.cpp
    qloghistoryitemdelegate.cpp
    qloghistorytablewidget.cpp
    qexttextbrowser.cpp

    combobox/qcomboboxgitdiffoptions.cpp
    combobox/qcomboboxgitstatusfiles.cpp
    combobox/qcomboboxgitviewoptions.cpp

    qgit/src/qgit.cpp
    qgit/src/qgitbranch.cpp
    qgit/src/qgitcommit.cpp
    qgit/src/qgitcommitdiffparent.cpp
    qgit/src/qgitdiffbinary.cpp
    qgit/src/qgitdiffbinaryfile.cpp
    qgit/src/qgitdifffile.cpp
    qgit/src/qgitdifffileitem.cpp
    qgit/src/qgitdiffhunk.cpp
    qgit/src/qgitdiffline.cpp
    qgit/src/qgiterror.cpp
    qgit/src/qgitsignature.cpp

    resource.qrc

    ${PROJECT_HEADERS}
)

qt_add_executable(gitmaster
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

IF (WIN32)
    target_sources(gitmaster PRIVATE resource.rc)
ENDIF()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_precompile_headers(gitmaster PRIVATE
        ${PROJECT_HEADERS}
    )
ENDIF()

target_include_directories(gitmaster PRIVATE combobox qgit/include)

IF (WIN32)
    target_include_directories(gitmaster PRIVATE include)
ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(gitmaster PROPERTIES OUTPUT_NAME "gitmaster_d")
ELSE()
    set_target_properties(gitmaster PROPERTIES OUTPUT_NAME "gitmaster")
ENDIF()

if (APPLE)
    target_link_libraries(gitmaster PRIVATE -L/opt/homebrew/lib)
ENDIF()

target_link_libraries(gitmaster PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    git2
)

IF (WIN32)
    target_link_directories(gitmaster PRIVATE
        bin
    )
ENDIF()

add_custom_command(TARGET gitmaster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)

set_target_properties(gitmaster PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("release mode")
    set_target_properties(gitmaster PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()

qt_finalize_executable(gitmaster)

set(CMAKE_INSTALL_PREFIX /usr)
install(
    TARGETS gitmaster
    BUNDLE DESTINATION gitmaster
)
