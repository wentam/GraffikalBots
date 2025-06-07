cmake_minimum_required(VERSION 2.8)
project(graffikal_bots)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)

file(GLOB base_SRC
  "src/*.c"
)

set(CMAKE_C_FLAGS "-fcommon")
#set(CMAKE_EXE_LINKER_FLAGS "-g -fsanitize=address")
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DLINUX)
endif()


add_executable(graffikalbots ${base_SRC})

find_library(GRAFFIKS_LIB graffiks)
find_library(BOTS_LIB bots)
find_library(M_LIB m)
find_library(SDL_LIB SDL2)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(M_LIB "")
endif()
target_link_libraries(graffikalbots ${GRAFFIKS_LIB} ${BOTS_LIB} ${M_LIB} ${SDL_LIB})

install(TARGETS graffikalbots DESTINATION bin)
