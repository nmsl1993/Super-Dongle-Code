cmake_minimum_required(VERSION 2.8)
project(sub_code)

add_executable(heading heading.c kiss_fft.c kiss_fftr.c udp_receiver.c)
add_executable(sim_heading heading.c kiss_fft.c kiss_fftr.c simulator_receiver.cpp)
add_executable(tmi TrackModeImplementation.c)
target_link_libraries(tmi m)
target_link_libraries(heading m )#Link libmath
target_link_libraries(sim_heading m )#Link libmath
SET(CMAKE_C_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS "-O3")
#install(TARGETS myapp DESTINATION bin)
