cmake_minimum_required(VERSION 3.0)
project(CandyFactory)

set(SOURCE_FILES
  src/bbuff.c
  src/stats.c
  src/candykids.c
)

set(HEADER_FILES
  include/bbuff.h
  include/stats.h
)

include_directories(include)

add_executable(candykids ${SOURCE_FILES} ${HEADER_FILES})

find_package(Threads REQUIRED)

target_link_libraries(candykids Threads::Threads)

find_library(RT_LIBRARY rt)
if(RT_LIBRARY)
  target_link_libraries(candykids ${RT_LIBRARY})
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -g -O1")
