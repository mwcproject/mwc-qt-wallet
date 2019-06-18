cmake_minimum_required(VERSION 3.5)

project(mwc-qt-wallet)


####################
#
# Qt configuration
# Please specify here path to your Qt installation
#
#set (Qt5_DIR "~/Qt/5.9/clang_64/lib/cmake/Qt5")
#set (Qt5_DIR "C:/Qt/5.9.8/msvc2015_64/lib/cmake/Qt5")

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)


####################
#
# Compiler options
#

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


####################
#
# Input files
#

file(GLOB SOURCE_FILES ./*.cpp ./control/*.cpp ./core/*.cpp ./state/*.cpp ./test/*.cpp ./tries/*.cpp ./util/*.cpp ./wallet/*.cpp ./wallet/tasks/*.cpp ./windows/*.cpp )
file(GLOB HEADER_FILES         ./control/*.h   ./core/*.h   ./state/*.h   ./test/*.h   ./tries/*.h   ./util/*.h   ./wallet/*.h   ./wallet/tasks/*.h   ./windows/*.h   )

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


####################
#
# Outputs
#

add_executable(mwc-gui ${SOURCE_FILES} ${HEADER_FILES} ${UI_GENERATED_HEADERS} resources.qrc)
target_link_libraries(mwc-gui Qt5::Widgets Qt5::Gui Qt5::Core)

# Copy Qt runtime libraries to build directory
# [Copying Qt DLLs to executable directory on Windows using CMake](https://stackoverflow.com/questions/40564443/)
add_custom_command(
    TARGET mwc-gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Core>
        $<TARGET_FILE:Qt5::Gui>
        $<TARGET_FILE:Qt5::Widgets>
        $<TARGET_FILE_DIR:mwc-gui>
)


####################
#
# Project settings
#

# Set Visual Studio start-up project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mwc-gui)
