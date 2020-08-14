/*****************************************************************************
                           simplering.cpp  -  description
                             -------------------
    begin                : November 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "gaspiCheckReturn.hpp"

#include <GASPI.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>





int main(int argc, char** argv) {




  if (argc != 3) {
    std::cerr << argv[0] << ": Usage " << argv[0] << " <length bytes>"
              << " <num iterations>"
              << std::endl;
    return -1;
  }

  const long len = atol(argv[1]);
  const long numIter = atol(argv[2]);


  const gaspi_segment_id_t segmentReduce = 1;
  const gaspi_notification_id_t firstNotification = 2;
  gaspi_queue_id_t queue = 0;

  gaspiCheckReturn(gaspi_proc_init(GASPI_BLOCK), "gaspi proc init");

//  gaspiCheckReturn(gaspi_group_commit(GASPI_GROUP_ALL, GASPI_BLOCK),
//                   "gaspi group commit");

  gaspi_rank_t numRanks;
  gaspiCheckReturn(gaspi_proc_num(&numRanks), "get number of ranks");
  gaspi_rank_t rank;
  gaspiCheckReturn(gaspi_proc_rank(&rank), "get rank");
  gaspi_rank_t nextRank = (rank + 1) % numRanks;

  const long lengthSegment = len * numRanks;


  gaspiCheckReturn(gaspi_segment_create(
    segmentReduce,
    lengthSegment,
    GASPI_GROUP_ALL,
    GASPI_BLOCK,
    GASPI_MEM_UNINITIALIZED),
    "create segment");

  gaspi_pointer_t ptr;
  gaspiCheckReturn(gaspi_segment_ptr(segmentReduce, &ptr), "get segment pointer");

  char* a = ((char*)ptr);
  memset(a, 0, lengthSegment);

  if (rank == 0) {
    std::cout << "rank 0 of " << numRanks << " has finished setup."
              << std::endl;
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  timeval timeStart, timeStop;
  gettimeofday(&timeStart, NULL);

  long queueStatus = 0;
  for (long iter=0; iter < numIter; iter++) {
    for (long phase=0; phase < numRanks; phase++) {
      const long index = (rank - phase + numRanks) % numRanks;
      const long previousIndex = (index - 1 + numRanks) % numRanks;
      gaspiCheckReturn(gaspi_write_notify(
        segmentReduce, index * len, nextRank,
        segmentReduce, index * len, len, firstNotification + index,
        1, queue, GASPI_BLOCK), "write notify");
      queueStatus++;

      gaspi_notification_id_t event;
      gaspiCheckReturn(gaspi_notify_waitsome(segmentReduce,
                                             firstNotification + previousIndex,
                                             1,
                                             &event,
                                             GASPI_BLOCK), "waitsome");
      gaspi_notification_t value;
      gaspiCheckReturn(gaspi_notify_reset(segmentReduce,
                                          event,
                                          &value), "reset");
      if (value != 1) {
        std::cerr << "wrong message id" << std::endl;
        exit(-1);
      }

      if (queueStatus > 500) {
        queueStatus = 0;
        queue = (queue + 1) % 2;
        gaspiCheckReturn(gaspi_wait(queue, GASPI_BLOCK), "gaspi wait");
      }
    }
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  gettimeofday(&timeStop, NULL);


  std::cout.flush();
  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  if (rank == 0) {

    const double numGigaBytes
      = double(numRanks) * len * numIter
      / 1024. / 1024. / 1024.;
    const double seconds = timeStop.tv_sec - timeStart.tv_sec
                         + 1e-6 * (timeStop.tv_usec - timeStart.tv_usec);
    std::cout << "Total runtime " << seconds << " seconds, "
              << numGigaBytes / seconds << " GiB/s." << std::endl;
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
  gaspiCheckReturn(gaspi_proc_term(GASPI_BLOCK), "gaspi proc term");

  return 0;
}
