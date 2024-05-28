cmake_minimum_required(VERSION 3.0)
project(raylib_template C)

set(CMAKE_C_STANDARD 99)

if (WIN32)
    INCLUDE_DIRECTORIES(
        C:/dev/vcpkg/installed/x64-windows/include
    )
    LINK_DIRECTORIES(
        C:/dev/vcpkg/installed/x64-windows/lib
    ) 
endif (WIN32)

if (UNIX)
    find_package(raylib 2.5.0 REQUIRED)
endif (UNIX)

# Setting parameters for raylib
# set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
# set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games

# add_subdirectory(libs/raylib)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h")

add_executable(${PROJECT_NAME} "${SOURCE_FILES}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc/")
# find_package(raylib CONFIG REQUIRED)
# if (NOT TARGET raylib)
#   find_package(raylib 2.5.0 REQUIRED)
# endif()
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
# target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
#target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="relative-path-to-assets-in-the-game-package") # Set the asset path macro in release mode