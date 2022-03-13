cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories("../")
link_libraries("/usr/lib/aarch64-linux-gnu/libboost_program_options.so.1.74.0")
link_libraries("/usr/local/lib/aarch64-linux-gnu/libcamera.so.0")
link_libraries("/usr/local/lib/aarch64-linux-gnu/libcamera-base.so.0")
link_libraries("/lib/aarch64-linux-gnu/libpthread.so.0")

project(libcamera-still)
add_executable(libcamera-still libcamera_still.cpp)
target_link_libraries(libcamera-still camera_app images)

project(libcamera-vid)
add_executable(libcamera-vid libcamera_vid.cpp)
target_link_libraries(libcamera-vid camera_app encoders outputs)

project(libcamera-hello)
add_executable(libcamera-hello libcamera_hello.cpp)
target_link_libraries(libcamera-hello camera_app)

project(libcamera-raw)
add_executable(libcamera-raw libcamera_raw.cpp)
target_link_libraries(libcamera-raw camera_app encoders outputs)

project(libcamera-jpeg)
add_executable(libcamera-jpeg libcamera_jpeg.cpp)
target_link_libraries(libcamera-jpeg camera_app images)

project(talk_to_camera)
add_executable(talk_to_camera talk_to_camera.cpp)
target_link_libraries(talk_to_camera camera)
 