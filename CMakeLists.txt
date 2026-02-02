cmake_minimum_required(VERSION 3.12)

# Can be one of thoese: address, thread, memory, undefined, leak
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

SET(GITMASTER_VERSION_TAG CACHE STRING "")
SET(GITMASTER_VERSION_REVISION CACHE STRING "0")

if(NOT GITMASTER_VERSION_TAG)
    find_package(Git QUIET)

    set(GIT_DIR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    if(EXISTS ${GIT_DIR_PATH})
        if(Git_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GITMASTER_VERSION_TAG
                OUTPUT_STRIP_TRAILING_WHITESPACE # Removes trailing newline/whitespace
                RESULT_VARIABLE GIT_DESCRIBE_RESULT
                ERROR_QUIET
            )
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-list ${GITMASTER_VERSION_TAG}..HEAD --count
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GITMASTER_VERSION_REVISION
                OUTPUT_STRIP_TRAILING_WHITESPACE # Removes trailing newline/whitespace
                RESULT_VARIABLE GIT_REV_LIST_RESULT
                ERROR_QUIET
            )
            execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GITMASTER_VERSION_COMMIT_NAME
                OUTPUT_STRIP_TRAILING_WHITESPACE # Removes trailing newline/whitespace
                RESULT_VARIABLE GIT_DESCRIBE_RESULT
                ERROR_QUIET
            )
        endif()
    endif()
endif()

if(NOT GITMASTER_VERSION_TAG MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Git last tag '${GITMASTER_VERSION_TAG}' is not in format X.Y.Z")
endif()

string(REGEX REPLACE "^([0-9]+\\.[0-9]+)\\..*$" "\\1" GITMASTER_VERSION_TAG_SHORT ${GITMASTER_VERSION_TAG})
string(REGEX REPLACE "^([0-9]+)\\..*$" "\\1" GITMASTER_VERSION_MAJOR ${GITMASTER_VERSION_TAG})
string(REGEX REPLACE "^[0-9]+\\.(.*)\\..*$" "\\1" GITMASTER_VERSION_MINOR ${GITMASTER_VERSION_TAG})
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.(.*)$" "\\1" GITMASTER_VERSION_PATCH ${GITMASTER_VERSION_TAG})

project(gitmaster VERSION ${GITMASTER_VERSION_TAG} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(Qt6 COMPONENTS Gui Widgets Network Svg REQUIRED)

set(PROJECT_HEADERS
    qgitmastermainwindow.h
    qgitrepository.h
    qnewrepositorydialog.h
    qgitclonerepositorydialog.h
    qgitpushdialog.h
    qgitpulldialog.h
    qgitfetchdialog.h
    qgitbranchdialog.h

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
    qgit/include/qgittag.h
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
    qgitbranchdialog.cpp          qgitbranchdialog.ui

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
    qgit/src/qgittag.cpp
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

if (WIN32)
    if("${PROJECT_VERSION_PATCH}" STREQUAL "")
        set(PROJECT_VERSION_PATCH 0)
    endif()
    if("${PROJECT_VERSION_TWEAK}" STREQUAL "")
        set(PROJECT_VERSION_TWEAK 0)
    endif()
    configure_file(resource.rc.in resource.rc @ONLY)
    target_sources(gitmaster PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_precompile_headers(gitmaster PRIVATE
        ${PROJECT_HEADERS}
    )
endif()

target_include_directories(gitmaster PRIVATE combobox qgit/include)

if (WIN32)
    target_include_directories(gitmaster PRIVATE include)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(gitmaster PROPERTIES OUTPUT_NAME "gitmaster_d")
else()
    set_target_properties(gitmaster PROPERTIES OUTPUT_NAME "gitmaster")
endif()

if (APPLE)
    target_link_libraries(gitmaster PRIVATE -L/opt/homebrew/lib)
endif()

target_link_libraries(gitmaster PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    Qt6::Svg
    git2
)

if (WIN32)
    target_link_directories(gitmaster PRIVATE
        bin
    )
endif()

if (NOT MSVC)
add_custom_command(TARGET gitmaster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)
endif()

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
