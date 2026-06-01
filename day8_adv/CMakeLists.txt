cmake_minimum_required(VERSION 3.10)
project(FaceDetectDNN)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(face_detect_dnn video_demo_dnn.cpp)
target_link_libraries(face_detect_dnn ${OpenCV_LIBS})
