cmake_minimum_required(VERSION 2.8)
project(sub_code)

add_executable(heading heading.c kiss_fft.c udp_receiver.c)
add_executable(sim_heading heading.c kiss_fft.c simulator_receiver.cpp)
target_link_libraries(heading m )#Link libmath
target_link_libraries(sim_heading m )#Link libmath

#install(TARGETS myapp DESTINATION bin)
