cmake_minimum_required(VERSION 3.21)
project(QuizCreator LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Sql Multimedia)

# (optionally) include(cmake/QtWarnings.cmake)

file(GLOB_RECURSE SRC_FILES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HDR_FILES
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

set(RES_FILE "${PROJECT_SOURCE_DIR}/resources/resources.qrc")

qt_add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${HDR_FILES}
    ${RES_FILE}
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::Sql
        Qt6::Multimedia
)

# Linux rpath fix (optionally)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN"
)
