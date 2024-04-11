# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.12.0)

# Define your project name and the programming language used
project(cascade_web_server C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog 1.3.1 REQUIRED)
find_package(derecho CONFIG REQUIRED)
find_package(cascade CONFIG REQUIRED)

# Option to enable Mongoose build options
option(ENABLE_MONGOOSE "Enable Mongoose build options" ON)

# Source files to include in the build
set(SOURCES main.cpp mongoose.c)

# Compiler flags
# set(CMAKE_C_FLAGS "-W -Wall -Wextra -g -I.")

# Add Mongoose build options if enabled
if(ENABLE_MONGOOSE)
    add_definitions(-DMG_ENABLE_LINES=1) # Add more definitions as needed
endif()

# On Windows, additional settings are needed
if(WIN32)
    # Add Windows-specific compile definitions and libraries
    add_definitions(-DWINDOWS=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lws2_32")
endif()

# Add an executable target based on the source files
add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(cascade_web_server PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${cascade_INCLUDE_DIRS}
)
target_link_libraries(cascade_web_server derecho::cascade)

# Target-specific compile options
target_compile_options(${PROJECT_NAME} PRIVATE ${CMAKE_C_FLAGS})

# For custom commands like cleaning extra files you could use the add_custom_target() function
# Note: CMake automatically handles the cleaning of standard build files through "make clean" or the equivalent command in other generators.
add_custom_target(clean-extra
    COMMAND ${CMAKE_COMMAND} -E remove_directory mbedtls
    COMMAND ${CMAKE_COMMAND} -E remove *.o *.obj *.exe *.dSYM
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Handling the mbedtls dependency
# This is a simplified version, assuming you have the mbedtls source available and it can be built with CMake.
find_package(Git QUIET)
if(GIT_FOUND AND NOT EXISTS "${PROJECT_SOURCE_DIR}/mbedtls/CMakeLists.txt")
    execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth 1 -b v2.28.2 https://github.com/mbed-tls/mbedtls mbedtls
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
if(EXISTS "${PROJECT_SOURCE_DIR}/mbedtls/CMakeLists.txt")
    add_subdirectory(mbedtls)
endif()
