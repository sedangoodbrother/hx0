cmake_minimum_required(VERSION 3.10.0)

project(Fencing)

# Add the include directories
include_directories(${SDL2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)
# Collect all source files (main.cpp and others in src directory)
set(SourceFiles 
    main.cpp
    menu.cpp
    character.cpp
    common.cpp
    moveset.cpp
    # Add other .cpp files here
)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}  # Set the working directory to the root of the project
)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Find required SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED) # Add SDL2_mixer

# Create the executable from the source files
add_executable(Fencing main.cpp character.cpp menu.cpp common.cpp moveset.cpp)

# Link the SDL2, SDL2_image, SDL2_ttf, and SDL2_mixer libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} SDL2_image::SDL2_image)
target_link_libraries(${PROJECT_NAME} SDL2_ttf::SDL2_ttf)
target_link_libraries(${PROJECT_NAME} SDL2_mixer::SDL2_mixer)