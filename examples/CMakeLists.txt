

set(COMMON_SOURCES
    "${CMAKE_SOURCE_DIR}/src/stb_image.c"
    utils.cpp
    "${CMAKE_SOURCE_DIR}/src/glad.c")

set(glm_DIR "${PROJECT_SOURCE_DIR}/../glm/cmake/glm")
find_package(glm REQUIRED)

macro(make_example EXAMPLE_NAME)

add_executable(${EXAMPLE_NAME} WIN32 MACOSX_BUNDLE "${EXAMPLE_NAME}.cpp" ${COMMON_SOURCES})

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  target_compile_options(${EXAMPLE_NAME} PRIVATE /W3 /WX)
else() 
  target_compile_options(${EXAMPLE_NAME} PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

target_link_libraries(${EXAMPLE_NAME} ${CMAKE_SOURCE_DIR}/lib/glfw3.lib)
target_include_directories(${EXAMPLE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include" "${GLM_INCLUDE_DIRS}")

endmacro(make_example EXAMPLE_NAME)

make_example(triangle)
make_example(textures)
make_example(moving_polygon)
make_example(cube)
make_example(matrices)
make_example(rotation)
make_example(camera)
make_example(camera_class)