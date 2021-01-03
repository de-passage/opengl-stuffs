set(COMMON_SOURCES
    "${CMAKE_SOURCE_DIR}/src/stb_image.c"
    utils.cpp
    "${CMAKE_SOURCE_DIR}/src/glad.c"
    "${CMAKE_SOURCE_DIR}/src/nuklear.c")

set(glm_DIR "${PROJECT_SOURCE_DIR}/../glm/cmake/glm")
find_package(glm REQUIRED)

function(set_compile_options TARGET_NAME)
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    target_compile_options(${TARGET_NAME} PRIVATE /W3 /WX)
  else() 
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic)
  endif(MSVC)
  target_compile_definitions(${TARGET_NAME} PUBLIC -D_CRT_SECURE_NO_WARNINGS)
  target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include" "${GLM_INCLUDE_DIRS}")
endfunction(set_compile_options TARGET_NAME)

add_library(external_libs STATIC ${COMMON_SOURCES})
set_compile_options(external_libs)

macro(make_example EXAMPLE_NAME)
  add_executable(${EXAMPLE_NAME} WIN32 MACOSX_BUNDLE "${EXAMPLE_NAME}.cpp" ${ARGN})
  set_compile_options(${EXAMPLE_NAME})

  target_link_libraries(${EXAMPLE_NAME} ${CMAKE_SOURCE_DIR}/lib/glfw3.lib external_libs)
endmacro(make_example EXAMPLE_NAME)

make_example(triangle)
make_example(textures)
make_example(moving_polygon)
make_example(cube)
make_example(matrices)
make_example(rotation)
make_example(camera)
make_example(camera_class)
make_example(hierarchy)
make_example(nuklear)
make_example(lighting)