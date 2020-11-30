cmake_minimum_required(VERSION 3.10)

# Project Details
project(GASPI)

enable_language(Fortran C CXX)

# Enable c++11 standard since we use Lambdas
set(CMAKE_CXX_STANDARD 11)

link_directories(external/DIST/lib64)

set(libSources
   external/collectives/allreduceButterfly.cpp
   external/collectives/allreduceButterflyDoubleBuffer.cpp
   external/collectives/broadcast.cpp
   external/collectives/counter.cpp
   external/collectives/mailBoxGaspi.cpp
   external/collectives/mailBoxLocal.cpp
   external/collectives/queues.cpp
   external/collectives/reduce.cpp
   external/collectives/writer.cpp
   )


add_library(collectives SHARED ${libSources})
target_include_directories(collectives PUBLIC 
                           external/DIST/include
                           external/collectives) 
target_link_libraries(collectives GPI2)


add_executable(testcoll run/benchmark.cpp)
target_include_directories(testcoll PUBLIC
                           external/DIST/include
                           external/collectives) 
target_link_libraries(testcoll collectives GPI2)


add_executable(fwrap run/benchmark.f90
                     run/fortran_gaspi.f90
                     run/fortran_wrap.cpp)

target_include_directories(fwrap PUBLIC
                           external/DIST/include
                           external/collectives) 
target_link_libraries(fwrap collectives GPI2)


add_executable(broadcast run/benchmarkBroadcast.cpp)
target_include_directories(broadcast PUBLIC
                           external/DIST/include
                           external/collectives)
target_link_libraries(broadcast collectives GPI2)

add_executable(simplering run/simplering.cpp)
target_include_directories(simplering PUBLIC
                           external/DIST/include
                           external/collectives)
target_link_libraries(simplering collectives GPI2)
