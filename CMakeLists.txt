cmake_minimum_required(VERSION 3.5)
#set( CMAKE_VERBOSE_MAKEFILE on )

project(mwc-qt-wallet)


####################
#
# Qt configuration
# Please specify here path to your Qt installation
#
set (Qt5_DIR "~/Qt/5.9.9/clang_64/lib/cmake/Qt5")
#set (Qt5_DIR "/usr/local/opt/qt/lib/cmake/Qt5")
#set (Qt5_DIR "C:/Qt/5.9.8/msvc2015_64/lib/cmake/Qt5")

find_package(Qt5 COMPONENTS Core Gui Widgets Network Svg REQUIRED)


####################
#
# Compiler options
#

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_definitions( -DWALLET_DESKTOP )

####################
#
# Input files
#

file(GLOB SOURCE_FILES ./*.cpp control_desktop/*.cpp ./core/*.cpp ./core_desktop/*.cpp ./state/*.cpp ./bridge/*.cpp ./bridge/wnd/*.cpp  ./tries/*.cpp ./util/*.cpp ./util_desktop/*.cpp ./wallet/*.cpp ./node/*.cpp  ./wallet/tasks/*.cpp  windows_desktop/*.cpp  dialogs_desktop/*.cpp ./tests/*.cpp ./misk/*.cpp)
file(GLOB HEADER_FILES         control_desktop/*.h   ./core/*.h   ./core_desktop/*.h   ./state/*.h   ./bridge/*.h   ./bridge/wnd/*.h    ./tries/*.h   ./util/*.h   ./util_desktop/*.h   ./wallet/*.h   ./node/*.h    ./wallet/tasks/*.h    windows_desktop/*.h    dialogs_desktop/*.h   ./tests/*.h   ./misk/*.h)

# Generate necessary headers from .ui files. (qmake lets `uic` do this job.)
# hint from [Cross-platform Qt5 project using cmake](http://stackoverflow.com/questions/21174586/)
file(GLOB UI_SOURCES ./*/*.ui )
qt5_wrap_ui(UI_GENERATED_HEADERS ${UI_SOURCES})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Handle .qrc files automatically
set(CMAKE_AUTORCC ON)

IF(APPLE)
    # For mac OS we have a cocoa file that customize the title
    find_library(AppKit AppKit)

    #target_link_libraries(${AppKit})

    set (Cocoa_SRCS
            macos/changetitlebarcolor.mm)

    set_source_files_properties(changetitlebarcolor.mm
            PROPERTIES COMPILE_FLAGS "-fobjc-arc")


ENDIF(APPLE)

####################
#
# Outputs
#

add_executable(mwc-qt-wallet ${SOURCE_FILES} ${HEADER_FILES} ${UI_GENERATED_HEADERS} ${Cocoa_SRCS} resources_desktop.qrc)
target_link_libraries(mwc-qt-wallet Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Network Qt5::Svg ${AppKit})

# Copy Qt runtime libraries to build directory
# [Copying Qt DLLs to executable directory on Windows using CMake](https://stackoverflow.com/questions/40564443/)
add_custom_command(
    TARGET mwc-qt-wallet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Core>
        $<TARGET_FILE:Qt5::Gui>
        $<TARGET_FILE:Qt5::Widgets>
        $<TARGET_FILE:Qt5::Network>
        $<TARGET_FILE:Qt5::Svg>
        $<TARGET_FILE_DIR:mwc-qt-wallet>
)


####################
#
# Project settings
#

# Set Visual Studio start-up project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mwc-qt-wallet)

