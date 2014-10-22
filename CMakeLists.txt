cmake_minimum_required (VERSION 2.6)

project (radio-observer)

# include_directories("src")

#set(CMAKE_CXX_COMPILER    "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS_INIT  "-Wall -ggdb -Icppapp -rdynamic")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cppapp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cppapp)

#add_executable(littlevm
#	src/main.cpp
#	)

add_executable(radio-observer
	src/Agent.cpp
	src/App.cpp
	src/Backend.cpp
	src/BolidMessage.cpp
	src/BolidRecorder.cpp
	src/CsvLog.cpp
	src/FFTBackend.cpp
	src/FITSWriter.cpp
	src/Frontend.cpp
	src/JackFrontend.cpp
	src/main.cpp
	src/MessageDispatch.cpp
	src/MetadataAgent.cpp
	src/Pipeline.cpp
	src/Signal.cpp
	src/utils.cpp
	src/WaterfallBackend.cpp
	src/WAVStream.cpp
	src/WFTime.cpp
	)

target_link_libraries(radio-observer cppapp fftw3 cfitsio pthread jack)

#target_link_libraries(littlevm littlelang ${CMAKE_DL_LIBS})
#target_link_libraries(littlelang ${CMAKE_DL_LIBS})
#target_link_libraries(tests littlelang gtest gtest_main)

