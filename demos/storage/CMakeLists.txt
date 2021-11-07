cmake_minimum_required(VERSION 3.13)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(media2_demo_storage)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(PkgConfig REQUIRED)
find_package(cereal REQUIRED)
pkg_check_modules(FFLIBS REQUIRED libavutil libavcodec libavdevice libswscale libavformat libswresample)
pkg_check_modules(UUID REQUIRED uuid)
find_package(OpenCV REQUIRED)
pkg_check_modules(EXR REQUIRED OpenEXR)
pkg_check_modules(ZMQ REQUIRED libzmq)
find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system serialization REQUIRED)
find_package(cppzmq)
add_definitions( -DBOOST_ALL_DYN_LINK )
add_compile_definitions(__STDC_CONSTANT_MACROS)

set(ALL_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/../router/include
  ${CEREAL_INCLUDE_DIRS}
  ${FFLIBS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${ZMQ_INCLUDE_DIRS}
  ${cppzmq_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
)

set(ALL_LINK_LIBS
  ${FFLIBS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${cppzmq_LIBRARIES}
  ${ZMQ_LIBRARIES}
  ${UUID_LIBRARIES}
  media2.so
  json_archive.a
  Threads::Threads
  )
set(ALL_LINK_DIRS
  ${CMAKE_BINARY_DIR}
  ${FFLIBS_LIBRARY_DIRS}
  ${Boost_LIBRARY_DIRS}
  ${OpenCV_LIBRARY_DIRS}
  ${cppzmq_LIBRARY_DIRS}
  ${ZMQ_LIBRARY_DIRS}
  ${UUID_LIBRARY_DIRS}
)

add_executable(media2_storage
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/Storage.cpp
  ${CMAKE_SOURCE_DIR}/../router/src/Job.cpp
)

target_include_directories(media2_storage PUBLIC ${ALL_INCLUDE_DIRS})
target_link_libraries(media2_storage PUBLIC ${ALL_LINK_LIBS})
target_link_directories(media2_storage PUBLIC ${ALL_LINK_DIRS})

