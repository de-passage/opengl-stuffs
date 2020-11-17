cmake_minimum_required(VERSION 3.0.0)
project(generalized_optional VERSION 0.1.0)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CPack)

set(SOURCES
    src/main.cpp
    src/glad.c)
add_executable(hello ${SOURCES})
target_link_libraries(hello ${CMAKE_SOURCE_DIR}/lib/glfw3.lib)
target_include_directories(hello PUBLIC include)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  target_compile_options(hello PRIVATE /W3 /WX)
else() 
  target_compile_options(hello PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

