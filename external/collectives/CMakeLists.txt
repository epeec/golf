############################################################################
#                          CMakeLists.txt  -  description
#                             -------------------
#    begin                : October 2019
#    copyright            : (C) 2019 by Martin Kuehn
#    email                : Martin.Kuehn@itwm.fraunhofer.de
############################################################################

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(libSources
    allreduceButterfly.cpp
    allreduceButterflyDoubleBuffer.cpp
    broadcast.cpp
    counter.cpp
    mailBoxGaspi.cpp
    mailBoxLocal.cpp
    queues.cpp
    reduce.cpp
    writer.cpp
   )

   
add_library(collectives SHARED ${libSources})
