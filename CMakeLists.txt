cmake_minimum_required(VERSION 2.8.11)
# Name of the project
set(PROJECT_NAME LookAtDemos)
project(${PROJECT_NAME})

add_subdirectory(${PROJECT_SOURCE_DIR}/SimpleLookAt/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/MultipleViews/ )
