project(pa-switch-on-port-available)
cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PA REQUIRED libpulse)

add_definitions(-std=c11)

aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${PA_LIBRARIES})
