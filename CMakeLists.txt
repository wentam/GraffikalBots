cmake_minimum_required(VERSION 2.8)
project(graffikal_bots)
set(CMAKE_BUILT_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)

file(GLOB base_SRC
  "src/*.c"
)

add_executable(graffikalbots ${base_SRC})

find_library(GRAFFIKS_LIB graffiks)
find_library(BOTS_LIB bots)
target_link_libraries(graffikalbots ${GRAFFIKS_LIB} ${BOTS_LIB})
