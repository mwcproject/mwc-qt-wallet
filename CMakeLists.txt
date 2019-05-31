cmake_minimum_required(VERSION 3.5)
project(gui-wallet)
set(CMAKE_CXX_STANDARD 14)

# Please specify here path to your QT installation
set (CMAKE_PREFIX_PATH "~/Qt/5.9/clang_64/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

file(GLOB SOURCE_FILES ./*.cpp ./control/*.cpp ./core/*.cpp ./state/*.cpp ./test/*.cpp ./tries/*.cpp ./util/*.cpp ./wallet/*.cpp ./wallet/tasks/*.cpp ./windows/*.cpp )
file(GLOB HEADER_FILES         ./control/*.h   ./core/*.h   ./state/*.h   ./test/*.h   ./tries/*.h   ./util/*.h   ./wallet/*.h   ./wallet/tasks/*.h   ./windows/*.h   )

#
# Generate necessary headers from .ui files. (qmake lets `uic` do this job.)
# hint from [Cross-platform Qt5 project using cmake](http://stackoverflow.com/questions/21174586/cross-platform-qt5-project-using-cmake)
#
file(GLOB UI_SOURCES ./*/*.ui )
qt5_wrap_ui(UI_GENERATED_HEADERS ${UI_SOURCES})


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


add_executable(gui-wallet ${SOURCE_FILES} ${HEADER_FILES} ${UI_GENERATED_HEADERS} )

target_link_libraries(gui-wallet Qt5::Widgets Qt5::Gui Qt5::Core)
