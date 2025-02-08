cmake_minimum_required(VERSION 3.0)

project(scanAPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(OrbbecSDK_DIR /home/john/lib/OrbbecSDK)
find_package(OrbbecSDK REQUIRED)
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(
    include
)

add_executable(recorder src/record.cpp)

target_include_directories(recorder PUBLIC
    ${OpenCV_INCLUDE_DIRS}
    ${OrbbecSDK_INCLUDE_DIRS}
)

target_link_libraries(recorder
    ${OpenCV_LIBS}
    yaml-cpp
    OrbbecSDK::OrbbecSDK
    pthread
)

add_executable(capture src/capture.cpp)

target_include_directories(capture PUBLIC
    ${OpenCV_INCLUDE_DIRS}
    ${OrbbecSDK_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(capture
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    yaml-cpp
    OrbbecSDK::OrbbecSDK
    pthread
)