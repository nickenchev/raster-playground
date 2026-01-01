find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
add_executable (sdl-demos "sdl-demos.cpp" "sdl-demos.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sdl-demos PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(sdl-demos PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)