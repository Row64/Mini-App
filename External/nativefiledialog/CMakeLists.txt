# Modifications Copyright (C) 2020 Row64
# Under the MIT License in LICENSE.txt


include(CheckIncludeFile)
set(SOURCES src/nfd_common.c)

macro(REQUIRE_INCLUDE_FILE path name)
    CHECK_INCLUDE_FILE(${path} ${name})
    if (NOT ${name})
        message(FATAL_ERROR "${path} not found")
    endif ()
endmacro()

# add specific implementations
if (WIN32)

    REQUIRE_INCLUDE_FILE(windows.h HAS_WINDOWS)
    list(APPEND SOURCES src/nfd_win.cpp)

elseif (APPLE)

    # -------- DRC modifications -------
    #REQUIRE_INCLUDE_FILE(AppKit/AppKit.h HAS_APPKIT)
    find_library(AppKit AppKit)
    #target_link_libraries(${PROJECT_NAME} ${AppKit})
    # -------- end modifications -------

    list(APPEND SOURCES src/nfd_cocoa.m)

elseif (UNIX)
    # -------- DRC modifications -------
    #REQUIRE_INCLUDE_FILE(gtk/gtk.h HAS_GTK)
    
    # Use the package PkgConfig to detect GTK+ headers/library files
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

    # Setup CMake to use GTK+, tell the compiler where to look for headers 
    # and to the linker where to look for libraries
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})

    # Add other flags to the compiler
    add_definitions(${GTK3_CFLAGS_OTHER})
    # -------- end modifications -------

    list(APPEND SOURCES src/nfd_gtk.c)

elseif (UNIX)
    message(FATAL_ERROR "Cannot detect your system, please report to https://github.com/aarcangeli/nativefiledialog-cmake/issues")
endif ()

add_library(nativefiledialog ${SOURCES})

# -------- DRC modifications -------
if ( APPLE )
    target_link_libraries(nativefiledialog ${AppKit})
elseif ( UNIX AND NOT APPLE )
    target_link_libraries(nativefiledialog ${GTK3_LIBRARIES})
endif()
# -------- end modifications -------

target_include_directories(nativefiledialog PUBLIC src/include)
