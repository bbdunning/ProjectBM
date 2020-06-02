cmake_minimum_required(VERSION 3.4)
include("${CMAKE_SOURCE_DIR}/FindGfxLibs.cmake")

# Name of the project
project(Super_Trash_Bros)

# Enforce use of C++14
set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_CXX_STANDARD 14)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/ext/*/*.cpp" "${CMAKE_SOURCE_DIR}/ext/glad/src/*.c" "${CMAKE_SOURCE_DIR}/ext/glad/src/*/*.c")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "ext/*/*.h" "ext/glad/*/*.h" "src/*/*.h")
file(GLOB_RECURSE GLSL "resources/*.glsl")

include_directories("ext")
include_directories("ext/irrklang")
include_directories("ext/glad/include")
include_directories("ext/imgui")
# link_directories("ext/imgui")

include_directories("src/Animation")
link_directories("src/Animation")
include_directories("src/Physics")
link_directories("src/Physics")
include_directories("src/GameObjects")
link_directories("src/GameObjects")
link_directories("lib/")

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

# Helper function included from FindGfxLibs.cmake
findGLFW3(${CMAKE_PROJECT_NAME})
findGLM(${CMAKE_PROJECT_NAME})
#findAssimp(${CMAKE_PROJECT_NAME})
#findBullet(${CMAKE_PROJECT_NAME})
include_directories("C:/bins/vcpkg-master/packages/bullet3_x86-windows/include/bullet")
# link_libraries("D:/source/ProjectBM/lib/irrKlang.lib")
# link_libraries("D:/source/ProjectBM/lib/irrKlang.dll")
# link_libraries("D:/source/ProjectBM/lib/irrKlang.exp")
# target_link_libraries(${target} "C:/bins/assimp-4.1.0/build/code/libassimp.dll")

# OS specific options and libraries
if(NOT WIN32)
  message(STATUS "Adding GCC style compiler flags")
  add_compile_options("-Wall")
  add_compile_options("-pedantic")
  add_compile_options("-Werror=return-type")
else()
  # Link OpenGL on Windows
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)
endif()

add_compile_options("-ggdb")