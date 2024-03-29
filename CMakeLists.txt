cmake_minimum_required(VERSION 3.5)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

project(gitmaster VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(QT NAMES Qt6 COMPONENTS Gui Widgets Network REQUIRED)
find_package(Qt6 COMPONENTS Gui Widgets Network REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    qgitmastermainwindow.cpp
    qgitmastermainwindow.h
    qgitmastermainwindow.ui
    qgitrepository.cpp
    qgitrepository.h
    qgitrepository.ui
    qnewrepositorydialog.cpp
    qnewrepositorydialog.h
    qnewrepositorydialog.ui
    qgitclonerepositorydialog.cpp
    qgitclonerepositorydialog.h
    qgitclonerepositorydialog.ui

    qgitrepotreeitemdelegate.cpp
    qgitrepotreeitemdelegate.h
    qgitrepotreewidget.cpp
    qgitrepotreewidget.h
    qgitdiffwidget.cpp
    qgitdiffwidget.h
    qadvplaintextedit.cpp
    qadvplaintextedit.h
    qloghistoryitemdelegate.cpp
    qloghistoryitemdelegate.h
    qloghistorytablewidget.cpp
    qloghistorytablewidget.h
    qexttextbrowser.cpp
    qexttextbrowser.h

    combobox/qcustomcombobox.cpp
    combobox/qcustomcombobox.h
    combobox/qcomboboxgitdiffoptions.cpp
    combobox/qcomboboxgitdiffoptions.h
    combobox/qcomboboxgitstatusfiles.cpp
    combobox/qcomboboxgitstatusfiles.h
    combobox/qcomboboxgitviewoptions.cpp
    combobox/qcomboboxgitviewoptions.h

    qgit/src/qgit.cpp
    qgit/include/qgit.h
    qgit/src/qgitbranch.cpp
    qgit/include/qgitbranch.h
    qgit/src/qgitcommit.cpp
    qgit/include/qgitcommit.h
    qgit/src/qgitcommitdiffparent.cpp
    qgit/include/qgitcommitdiffparent.h
    qgit/src/qgitdiffbinary.cpp
    qgit/include/qgitdiffbinary.h
    qgit/src/qgitdiffbinaryfile.cpp
    qgit/include/qgitdiffbinaryfile.h
    qgit/src/qgitdifffile.cpp
    qgit/include/qgitdifffile.h
    qgit/src/qgitdifffileitem.cpp
    qgit/include/qgitdifffileitem.h
    qgit/src/qgitdiffhunk.cpp
    qgit/include/qgitdiffhunk.h
    qgit/src/qgitdiffline.cpp
    qgit/include/qgitdiffline.h
    qgit/src/qgiterror.cpp
    qgit/include/qgiterror.h
    qgit/src/qgitsignature.cpp
    qgit/include/qgitsignature.h

    resource.qrc
)

IF (WIN32)
    target_sources(foobar PRIVATE resource.rc)
ENDIF()

qt_add_executable(gitmaster
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

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
    link_directories(${CMAKE_SOURCE_DIR}/bin)
ENDIF()

set_target_properties(gitmaster PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(gitmaster)
endif()

set(CMAKE_INSTALL_PREFIX /usr)
install(
    TARGETS gitmaster
    BUNDLE DESTINATION gitmaster
)
