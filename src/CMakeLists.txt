cmake_minimum_required(VERSION 3.4)
project(main_test)

add_definitions("-O2")
set(CMAKE_BUILD_TYPE "Release -NDEBUG")

aux_source_directory(. DIR_SRCS)

add_subdirectory(mini-heap)
add_subdirectory(timer)
add_subdirectory(evio)
add_subdirectory(net)
#add_subdirectory(hash)
add_subdirectory(nanomsg)

add_executable(main_test ${DIR_SRCS})
target_link_libraries(main_test net-func)
target_link_libraries(main_test evio-func)
target_link_libraries(main_test timer-func)
target_link_libraries(main_test miniheap-func)
#target_link_libraries(main_test hash-func)
target_link_libraries(main_test nanomsg)
target_link_libraries(main_test pthread)