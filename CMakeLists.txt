cmake_minimum_required(VERSION 3.13)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(fr_media2 CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include(FetchContent)

FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
  )

FetchContent_Declare(
  boostsml
  GIT_REPOSITORY https://github.com/boost-ext/sml.git
  GIT_TAG v1.1.8
  )

FetchContent_Declare(
  ZeroMQ
  GIT_REPOSITORY https://github.com/zeromq/libzmq.git
  GIT_TAG v4.3.4
  )

FetchContent_Declare(
  cppzmq
  GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
  GIT_TAG v4.10.0
  )

FetchContent_MakeAvailable(gtest boostsml ZeroMQ cppzmq)

find_package(PkgConfig REQUIRED)
include(GoogleTest)
pkg_check_modules(FFLIBS REQUIRED libavutil libavcodec libavdevice libswscale libavformat libswresample)
pkg_check_modules(UUID REQUIRED uuid)
find_package(OpenCV REQUIRED)
# Todo: Do I still need EXR? Comment out sometime and see
pkg_check_modules(EXR REQUIRED OpenEXR)

# Set up boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system serialization REQUIRED)
add_definitions( -DBOOST_ALL_DYN_LINK )

# Definitions required by ffmpeg
add_compile_definitions(__STDC_CONSTANT_MACROS)

# Library

add_library(media2 SHARED
  ${CMAKE_SOURCE_DIR}/src/Decoder.cpp
  ${CMAKE_SOURCE_DIR}/src/Encoder.cpp
  ${CMAKE_SOURCE_DIR}/src/Frame.cpp
  ${CMAKE_SOURCE_DIR}/src/FrameSubscriber.cpp
  ${CMAKE_SOURCE_DIR}/src/Packet.cpp
  ${CMAKE_SOURCE_DIR}/src/PacketReader.cpp
  ${CMAKE_SOURCE_DIR}/src/PacketSubscriber.cpp
  ${CMAKE_SOURCE_DIR}/src/Scaler.cpp
  ${CMAKE_SOURCE_DIR}/src/Segment.cpp
  ${CMAKE_SOURCE_DIR}/src/Segmenter.cpp
  ${CMAKE_SOURCE_DIR}/src/SegmentSubscriber.cpp
  ${CMAKE_SOURCE_DIR}/src/SegmentUnpacker.cpp
  ${CMAKE_SOURCE_DIR}/src/Stream.cpp
  ${CMAKE_SOURCE_DIR}/src/StreamCache.cpp
  ${CMAKE_SOURCE_DIR}/src/StreamData.cpp
  ${CMAKE_SOURCE_DIR}/src/Muxer.cpp
  ${CMAKE_SOURCE_DIR}/src/ZmqSegmentPublisher.cpp
  ${CMAKE_SOURCE_DIR}/src/ZmqSegmentSubscriber.cpp
  )
set(ALL_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  ${FFLIBS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
  ${boostsml_SOURCE_DIR}/include
  ${gtest_SOURCE_DIR}/googletest/include
  ${libzmq_SOURCE_DIR}/include
  ${cppzmq_SOURCE_DIR}
)
set(ALL_LINK_LIBS
  ${FFLIBS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${ZMQ_LIBRARIES}
  ${UUID_LIBRARIES}
  libzmq
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

target_include_directories(media2 PUBLIC
  ${ALL_INCLUDE_DIRS}
)
target_link_libraries(media2 PUBLIC
  ${ALL_LINK_LIBS}
)
target_link_directories(media2 PUBLIC
  ${ALL_LINK_DIRS}
)

#
# Python module support will be compiled if you have pybind11 installed
#

find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG)
if (pybind11_FOUND)
   pybind11_add_module(fr_media2 MODULE ${CMAKE_SOURCE_DIR}/python/media2.cpp)

   target_include_directories(fr_media2 PUBLIC
      ${ALL_INCLUDE_DIRS}
     )
   target_link_libraries(fr_media2 PUBLIC
      ${ALL_LINK_LIBS}
      media2
     )
   target_link_directories(fr_media2 PUBLIC
      ${ALL_LINK_DIRS}
     )
endif()

# Test Executables
set(TEST_DATA_DIR "${CMAKE_BINARY_DIR}/test_data")
file(MAKE_DIRECTORY "${TEST_DATA_DIR}")
if (NOT EXISTS "${TEST_DATA_DIR}/testvideo.mp4")
  message("Generating test data in ${CMAKE_BINARY_DIR}")
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/generate_test_video.sh"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
  
add_executable(PacketTest ${CMAKE_SOURCE_DIR}/test/PacketTest.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(PacketTest PUBLIC
  gtest
  ${ALL_INCLUDE_DIRS}
  )
target_link_libraries(PacketTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2)
target_link_directories(PacketTest PUBLIC
  ${ALL_LINK_DIRS}
  )

add_executable(PacketSubscriberTest ${CMAKE_SOURCE_DIR}/test/PacketSubscriberTest.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(PacketSubscriberTest PUBLIC
  gtest
  ${ALL_INCLUDE_DIRS}
)
target_link_libraries(PacketSubscriberTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2
)
target_link_directories(PacketSubscriberTest PUBLIC
  ${ALL_LINK_DIRS}
)

add_executable(PacketReaderTest ${CMAKE_SOURCE_DIR}/test/PacketReaderTest.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(PacketReaderTest PUBLIC
  gtest
  ${ALL_INCLUDE_DIRS}
  )
target_link_libraries(PacketReaderTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2)
target_link_directories(PacketReaderTest PUBLIC
  ${ALL_LINK_DIRS}
  )
# Since this one needs a test file
target_compile_definitions(PacketReaderTest PRIVATE TEST_FILE=\"${TEST_DATA_DIR}/testvideo.mp4\")


add_executable(DecoderTest ${CMAKE_SOURCE_DIR}/test/DecoderTest.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(DecoderTest PUBLIC
  gtest
  ${ALL_INCLUDE_DIRS}
  )
target_link_libraries(DecoderTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2)
target_link_directories(DecoderTest PUBLIC
  ${ALL_LINK_DIRS}
  )
target_compile_definitions(DecoderTest PRIVATE TEST_FILE=\"${TEST_DATA_DIR}/testvideo.mp4\")


add_executable(EncoderTest ${CMAKE_SOURCE_DIR}/test/EncoderTest.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(EncoderTest PUBLIC
  gtest
  ${ALL_INCLUDE_DIRS}
  )
target_link_libraries(EncoderTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2
  )
target_link_directories(EncoderTest PUBLIC
  ${ALL_LINK_DIRS}
  )
target_compile_definitions(EncoderTest PRIVATE TEST_FILE=\"${TEST_DATA_DIR}/testvideo.mp4\")

add_executable(MuxerTest ${CMAKE_SOURCE_DIR}/test/MuxerTest.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(MuxerTest PUBLIC
  gtest
  ${ALL_INCLUDE_DIRS}
  )
target_link_libraries(MuxerTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2
  )
target_link_directories(MuxerTest PUBLIC
  ${ALL_LINK_DIRS}
  )
target_compile_definitions(MuxerTest PRIVATE TEST_FILE=\"${TEST_DATA_DIR}/testvideo.mp4\")

add_executable(SerializationTest ${CMAKE_SOURCE_DIR}/test/SerializationTest.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(SerializationTest PUBLIC
  gtest
  ${ALL_INCLUDE_DIRS}
  )
target_link_libraries(SerializationTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2
  )
target_link_directories(SerializationTest PUBLIC
  ${ALL_LINK_DIRS}
  )
target_compile_definitions(SerializationTest PRIVATE TEST_FILE=\"${TEST_DATA_DIR}/testvideo.mp4\")

add_executable(TransportTest ${CMAKE_SOURCE_DIR}/test/Transport.cpp ${CMAKE_SOURCE_DIR}/test/main.cpp)
target_include_directories(TransportTest PUBLIC
  ${ALL_INCLUDE_DIRS}
  )
target_link_libraries(TransportTest PUBLIC
  gtest
  ${ALL_LINK_LIBS}
  media2
  )
target_link_directories(TransportTest PUBLIC
  gtest
  ${ALL_LINK_DIRS}
  )
target_compile_definitions(TransportTest PRIVATE TEST_FILE=\"${TEST_DATA_DIR}/testvideo.mp4\")

enable_testing()
add_test(NAME PacketTest COMMAND PacketTest)
add_test(NAME PacketSubscriberTest COMMAND PacketSubscriberTest)
add_test(NAME PacketReaderTest COMMAND PacketReaderTest)
add_test(NAME DecoderTest COMMAND DecoderTest)
add_test(NAME EncoderTest COMMAND EncoderTest)
add_test(NAME MuxerTest COMMAND MuxerTest)
add_test(NAME SerializationTest COMMAND SerializationTest)
add_test(NAME TransportTest COMMAND TransportTest)

include(GNUInstallDirs)
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/fr")

set(HEADER_INSTALL_LIST
  ${INCLUDE_DIR}/media2/Decoder.h
  ${INCLUDE_DIR}/media2/Encoder.h
  ${INCLUDE_DIR}/media2/Frame.h
  ${INCLUDE_DIR}/media2/Frame2Mat.h
  ${INCLUDE_DIR}/media2/FrameSource.h
  ${INCLUDE_DIR}/media2/FrameSubscriber.h
  ${INCLUDE_DIR}/media2/Muxer.h
  ${INCLUDE_DIR}/media2/Packet.h
  ${INCLUDE_DIR}/media2/PacketReaderBase.h
  ${INCLUDE_DIR}/media2/PacketReader.h
  ${INCLUDE_DIR}/media2/PacketSubscriber.h
  ${INCLUDE_DIR}/media2/Resampler.h
  ${INCLUDE_DIR}/media2/Scaler.h
  ${INCLUDE_DIR}/media2/Segmenter.h
  ${INCLUDE_DIR}/media2/Segment.h
  ${INCLUDE_DIR}/media2/SegmentSubscriber.h
  ${INCLUDE_DIR}/media2/SegmentUnpacker.h
  ${INCLUDE_DIR}/media2/Serialization.h
  ${INCLUDE_DIR}/media2/StreamCache.h
  ${INCLUDE_DIR}/media2/StreamData.h
  ${INCLUDE_DIR}/media2/Stream.h
  ${INCLUDE_DIR}/media2/ZmqSegmentPublisher.h
  ${INCLUDE_DIR}/media2/ZmqSegmentSubscriber.h
  )

install(FILES ${INCLUDE_DIR}/media2.h
  DESTINATION include/fr
)

install(FILES ${HEADER_INSTALL_LIST}
  DESTINATION include/fr/media2
)

install(TARGETS media2
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if (pybind11_FOUND)

  install(TARGETS fr_media2
    COMPONENT python
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
  
endif()
