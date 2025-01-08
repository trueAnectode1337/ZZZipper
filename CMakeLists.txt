cmake_minimum_required(VERSION 3.16)

project(untitled3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
# Указание пути к libarchive, установленной через vcpkg
set(LIBARCHIVE_DIR "C:/Users/SuperDoomer/vcpkg/installed/x64-windows")

# Поиск библиотеки libarchive
find_library(ARCHIVE_LIBRARY NAMES archive HINTS ${LIBARCHIVE_DIR}/lib)

# Поиск заголовочного файла archive.h
find_path(ARCHIVE_INCLUDE_DIR NAMES archive.h HINTS ${LIBARCHIVE_DIR}/include)

if(ARCHIVE_LIBRARY AND ARCHIVE_INCLUDE_DIR)

message(STATUS "libarchive found: ${ARCHIVE_LIBRARY}")
    include_directories(${ARCHIVE_INCLUDE_DIR})
else()
    message(FATAL_ERROR "libarchive not found at specified location")
endif()

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(untitled3
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resor.qrc
        archivedialog.h archivedialog.cpp archivedialog.ui











    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET untitled3 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(untitled3 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(untitled3
            ${PROJECT_SOURCES}
        )
    endif()
endif()
# Линковка с libarchive
target_link_libraries(untitled3 PRIVATE ${ARCHIVE_LIBRARY})
target_link_libraries(untitled3 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.untitled3)
endif()
set_target_properties(untitled3 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS untitled3
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(untitled3)
endif()
