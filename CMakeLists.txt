cmake_minimum_required(VERSION 3.10)
project(app VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(nlohmann_json REQUIRED)

set(UTILS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/utils")
set(UTILS_SOURCES
        "${UTILS_DIR}/json_parser.cpp"
        "${UTILS_DIR}/problem_arguments.cpp"
)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SRC_SOURCES
        "${SRC_DIR}/first_step.cpp"
        "${SRC_DIR}/algorithm.cpp"
        "${SRC_DIR}/crossover.cpp"
        "${SRC_DIR}/init_population.cpp"
        "${SRC_DIR}/vnd.cpp"
        "${SRC_DIR}/vns.cpp"
)

add_library(utils STATIC ${UTILS_SOURCES})
target_include_directories(utils PUBLIC utils)
target_link_libraries(utils PUBLIC nlohmann_json::nlohmann_json)


add_library(algo STATIC ${SRC_SOURCES})
target_include_directories(algo PUBLIC algo src)
target_link_libraries(algo PUBLIC utils)

set(SOURCES main.cpp)

add_executable(app ${SOURCES})
target_link_libraries(app PRIVATE utils algo)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()