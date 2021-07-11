cmake_minimum_required(VERSION 3.5 FATAL_ERROR)#2.8 originally

#add_definitions(-std=c++14)#c++11 originally

set(CXX_FLAGS "-Wall" "-pedantic")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")

project(camera_fusion)

# Removed 4.1 requirement for compilation
find_package(OpenCV REQUIRED)
find_package(PCL 1.2 REQUIRED)#NR

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

include_directories(${PCL_INCLUDE_DIRS})#NR
link_directories(${PCL_LIBRARY_DIRS})#NR
add_definitions(${PCL_DEFINITIONS})#NR

# Executables for exercises
add_executable (show_lidar_top_view src/show_lidar_top_view.cpp src/structIO.cpp)
target_link_libraries (show_lidar_top_view ${OpenCV_LIBRARIES})
target_link_libraries (show_lidar_top_view ${PCL_LIBRARIES})#NR

add_executable (project_lidar_to_camera src/project_lidar_to_camera.cpp src/structIO.cpp)
target_link_libraries (project_lidar_to_camera ${OpenCV_LIBRARIES})
target_link_libraries (project_lidar_to_camera ${PCL_LIBRARIES})#NR
