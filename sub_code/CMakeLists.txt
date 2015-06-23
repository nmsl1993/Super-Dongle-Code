cmake_minimum_required(VERSION 2.8)
project(sub_code)

add_executable(heading heading.c udp_receiver.c)
add_executable(sim_heading heading.c libcsv.c simulator_receiver.c)

#install(TARGETS myapp DESTINATION bin)
