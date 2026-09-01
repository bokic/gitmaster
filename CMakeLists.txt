cmake_minimum_required(VERSION 3.16)

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

if(NOT GITMASTER_VERSION_TAG)
    message(FATAL_ERROR "GITMASTER_VERSION_TAG is not set and could not be determined from git repository. Please provide it via -DGITMASTER_VERSION_TAG=X.Y.Z")
endif()

if(NOT GITMASTER_VERSION_TAG MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Version tag '${GITMASTER_VERSION_TAG}' is not in format X.Y.Z")
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

if (WIN32 AND NOT DEFINED ZLIB_ROOT)
    set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}")
endif()

find_package(Qt6 COMPONENTS Gui Widgets Network Svg REQUIRED)
find_package(ZLIB REQUIRED)

set(PROJECT_HEADERS
    include/qgitmastermainwindow.h
    include/qgitrepository.h
    include/qnewrepositorydialog.h
    include/qgitclonerepositorydialog.h
    include/qgitpushdialog.h
    include/qgitpulldialog.h
    include/qgitfetchdialog.h
    include/qgitbranchdialog.h
    include/qgitsettingsdialog.h
    include/qgitconflictresolverdialog.h
    include/qgitcreatetagdialog.h
    include/qgitcleandialog.h
    include/qgitstashdialog.h
    include/qgitstashinspectdialog.h
    include/qgitreflogdialog.h
    include/qgitremotesdialog.h
    include/qgitblamedialog.h
    include/qgitworktreedialog.h
    include/qgitflowdialog.h
    include/qgitinteractiverebasedialog.h
    include/qgitaddsubmoduledialog.h
    include/qgitsubmodulepointerdialog.h
    include/qgitremovesubmoduledialog.h
    include/qgitexportpatchdialog.h
    include/qgitexportarchivedialog.h
    include/qgitmergedialog.h

    include/qgitrepotreeitemdelegate.h
    include/qgitrepotreewidget.h
    include/qgitbranchtreewidget.h
    include/qgitfilestatustreewidget.h
    include/qgitdiffwidget.h
    include/qadvplaintextedit.h
    include/qloghistoryitemdelegate.h
    include/qloghistorytablewidget.h
    include/qexttextbrowser.h

    include/combobox/qcomboboxgitbase.h
    include/combobox/qcomboboxgitdiffoptions.h
    include/combobox/qcomboboxgitstatusfiles.h
    include/combobox/qcomboboxgitviewoptions.h

    include/qgit/qgit.h
    include/qgit/qgitbranch.h
    include/qgit/qgittag.h
    include/qgit/qgitcommit.h
    include/qgit/qgitcommitdiffparent.h
    include/qgit/qgitdiffbinary.h
    include/qgit/qgitdiffbinaryfile.h
    include/qgit/qgitdifffile.h
    include/qgit/qgitdifffileitem.h
    include/qgit/qgitdiffhunk.h
    include/qgit/qgitdiffline.h
    include/qgit/qgiterror.h
    include/qgit/qgitsignature.h
    include/qgit/qgitworktree.h
    include/qgit/qgitrebasetodoitem.h
)

set(PROJECT_SOURCES
    src/main.cpp

    src/qgitmastermainwindow.cpp       src/qgitmastermainwindow.ui
    src/qgitrepository.cpp             src/qgitrepository.ui
    src/qnewrepositorydialog.cpp       src/qnewrepositorydialog.ui
    src/qgitclonerepositorydialog.cpp  src/qgitclonerepositorydialog.ui
    src/qgitpushdialog.cpp             src/qgitpushdialog.ui
    src/qgitpulldialog.cpp             src/qgitpulldialog.ui
    src/qgitfetchdialog.cpp            src/qgitfetchdialog.ui
    src/qgitbranchdialog.cpp           src/qgitbranchdialog.ui
    src/qgitmergedialog.cpp            src/qgitmergedialog.ui
    src/qgitsettingsdialog.cpp         src/qgitsettingsdialog.ui
    src/qgitconflictresolverdialog.cpp src/qgitconflictresolverdialog.ui
    src/qgitcreatetagdialog.cpp        src/qgitcreatetagdialog.ui
    src/qgitcleandialog.cpp            src/qgitcleandialog.ui
    src/qgitstashdialog.cpp            src/qgitstashdialog.ui
    src/qgitstashinspectdialog.cpp     src/qgitstashinspectdialog.ui
    src/qgitreflogdialog.cpp           src/qgitreflogdialog.ui
    src/qgitremotesdialog.cpp          src/qgitremotesdialog.ui
    src/qgitblamedialog.cpp            src/qgitblamedialog.ui
    src/qgitworktreedialog.cpp         src/qgitworktreedialog.ui
    src/qgitflowdialog.cpp             src/qgitflowdialog.ui
    src/qgitinteractiverebasedialog.cpp src/qgitinteractiverebasedialog.ui
    src/qgitaddsubmoduledialog.cpp     src/qgitaddsubmoduledialog.ui
    src/qgitsubmodulepointerdialog.cpp src/qgitsubmodulepointerdialog.ui
    src/qgitremovesubmoduledialog.cpp  src/qgitremovesubmoduledialog.ui
    src/qgitexportpatchdialog.cpp      src/qgitexportpatchdialog.ui
    src/qgitexportarchivedialog.cpp    src/qgitexportarchivedialog.ui

    src/qgitrepotreeitemdelegate.cpp
    src/qgitrepotreewidget.cpp
    src/qgitbranchtreewidget.cpp
    src/qgitfilestatustreewidget.cpp
    src/qgitdiffwidget.cpp
    src/qadvplaintextedit.cpp
    src/qloghistoryitemdelegate.cpp
    src/qloghistorytablewidget.cpp
    src/qexttextbrowser.cpp

    src/combobox/qcomboboxgitbase.cpp
    src/combobox/qcomboboxgitdiffoptions.cpp
    src/combobox/qcomboboxgitstatusfiles.cpp
    src/combobox/qcomboboxgitviewoptions.cpp

    src/qgit/qgit.cpp
    src/qgit/qgitbranch.cpp
    src/qgit/qgittag.cpp
    src/qgit/qgitcommit.cpp
    src/qgit/qgitcommitdiffparent.cpp
    src/qgit/qgitdiffbinary.cpp
    src/qgit/qgitdiffbinaryfile.cpp
    src/qgit/qgitdifffile.cpp
    src/qgit/qgitdifffileitem.cpp
    src/qgit/qgitdiffhunk.cpp
    src/qgit/qgitdiffline.cpp
    src/qgit/qgiterror.cpp
    src/qgit/qgitsignature.cpp
    src/qgit/qgitworktree.cpp

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


target_include_directories(gitmaster PRIVATE include include/combobox include/qgit)
target_compile_definitions(gitmaster PRIVATE GITMASTER_VERSION="${PROJECT_VERSION}")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(gitmaster PRIVATE -Wno-documentation-deprecated-sync)
endif()


if (WIN32)
    target_include_directories(gitmaster PRIVATE include)
endif()


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(gitmaster PROPERTIES OUTPUT_NAME "gitmaster_d")
else()
    set_target_properties(gitmaster PROPERTIES OUTPUT_NAME "gitmaster")
endif()

if (APPLE)
    find_library(LIBGIT2_LIB git2 HINTS /opt/homebrew/lib /usr/local/lib /opt/local/lib)
    if (LIBGIT2_LIB)
        target_link_libraries(gitmaster PRIVATE ${LIBGIT2_LIB})
    else()
        target_link_libraries(gitmaster PRIVATE git2)
    endif()
    find_path(LIBGIT2_INCLUDE_DIR git2.h HINTS /opt/homebrew/include /usr/local/include /opt/local/include)
    if (LIBGIT2_INCLUDE_DIR)
        target_include_directories(gitmaster PRIVATE ${LIBGIT2_INCLUDE_DIR})
    endif()
    target_sources(gitmaster PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/macos/gitmaster.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/macos/gitmaster.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
else()
    target_link_libraries(gitmaster PRIVATE git2)
endif()

target_link_libraries(gitmaster PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    Qt6::Svg
    ZLIB::ZLIB
)

if (WIN32)
    target_link_directories(gitmaster PRIVATE
        bin
        lib
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

if (APPLE)
    set_target_properties(gitmaster PROPERTIES
        MACOSX_BUNDLE_ICON_FILE gitmaster.icns
        MACOSX_BUNDLE_BUNDLE_NAME "Git Master"
    )
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("release mode")
    set_target_properties(gitmaster PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()

qt_finalize_executable(gitmaster)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Default install prefix" FORCE)
endif()
install(
    TARGETS gitmaster
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION gitmaster
)

install(
    FILES io.github.bokic.gitmaster.desktop
    DESTINATION share/applications
)

install(
    FILES io.github.bokic.gitmaster.svg
    DESTINATION share/icons/hicolor/scalable/apps
)

install(
    FILES io.github.bokic.gitmaster.metainfo.xml
    DESTINATION share/metainfo
)
