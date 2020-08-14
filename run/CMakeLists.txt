############################################################################
#                          CMakeLists.txt  -  description
#                             -------------------
#    begin                : November 2019
#    copyright            : (C) 2019 by Martin Kuehn
#    email                : Martin.Kuehn@itwm.fraunhofer.de
############################################################################

set(sources
    benchmark.cpp
   )

include_directories(../lib)


add_executable(testcoll ${sources})
link_directories(${COMMON_LIBRARIES})
target_link_libraries(testcoll hpdlf-collectives ${COMMON_LIBRARIES})

add_executable(broadcast benchmarkBroadcast.cpp)
target_link_libraries(broadcast hpdlf-collectives ${COMMON_LIBRARIES})

add_executable(simplering simplering.cpp)
target_link_libraries(simplering  ${COMMON_LIBRARIES})
