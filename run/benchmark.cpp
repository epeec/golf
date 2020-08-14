/*****************************************************************************
                           benchmark.cpp  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "allreduceButterflyDoubleBuffer.h"
#include "gaspiCheckReturn.hpp"

#include <GASPI.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include <sys/time.h>

template <class T>
void fill(T a[],
         const T rank,
         const long n) {
  for (long i=0; i < n; i++) {
    a[i] = i + rank + 1;
  }
}

template <class T>
int check(const T a[],
          const long rank,
          const long numRanks,
          const long n) {
  int state = 0;
  for (long i=0; i < n; i++) {
    const T expect = (numRanks * (numRanks + 1)) / 2 + numRanks * i;
    if (a[i] != expect) {
      std::stringstream s;
      s << "Bad result in rank " << rank << ", at position " << i
        << ", "<< a[i] << " expected " << expect << std::endl;
      std::cerr << s.str();
      state = 1;
    }
  }
  return state;
}

void report(allreduceButterflyDoubleBuffer& r,
            const long rank,
            const long numRanks) {
  for (long i=0; i < numRanks; i++) {
    gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK),
                     "gaspi barrier");
    if (i == rank) {

      std::cout << "****************************" << std::endl;
      std::cout << "report for rank " << rank << std::endl;
      std::cout << "****************************" << std::endl;
      r.report(std::cout);
      std::cout.flush();
    }
  }
  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK),
                   "gaspi barrier");
}


int main(int argc, char** argv) {

  typedef double dataType;
  const allreduce::dataType data = allreduce::DOUBLE;


  if ((argc < 3) || (argc > 4)) {
    std::cerr << argv[0] << ": Usage " << argv[0] << " <length bytes>"
              << " <num iterations> [check]"
              << std::endl;
    return -1;
  }

  const long len = atol(argv[1]) / sizeof(dataType);
  const long numIter = atol(argv[2]);
  const bool checkResults = (argc==4)?true:false;

  const gaspi_segment_id_t segmentReduce = 1;
  const gaspi_segment_id_t segmentCommunicate = 2;
  const gaspi_offset_t offsetReduce = 64;
  const gaspi_offset_t offsetCommunicate = 32;
  const gaspi_notification_id_t firstNotificationReduce = 2;
  const gaspi_notification_id_t firstNotificationCommunicate = 5;
  queues queueStock(2);

  gaspiCheckReturn(gaspi_proc_init(GASPI_BLOCK), "gaspi proc init");

//  gaspiCheckReturn(gaspi_group_commit(GASPI_GROUP_ALL, GASPI_BLOCK),
//                   "gaspi group commit");

  gaspi_rank_t numRanks;
  gaspiCheckReturn(gaspi_proc_num(&numRanks), "get number of ranks");
  gaspi_rank_t rank;
  gaspiCheckReturn(gaspi_proc_rank(&rank), "get rank");


  const long lengthCommunicationBuffer =
    allreduceButterflyDoubleBuffer::getNumberOfElementsSegmentCommunicate(
    len, numRanks);

  gaspiCheckReturn(gaspi_segment_create(
    segmentReduce,
    2 * len * sizeof(dataType) + offsetReduce,
    GASPI_GROUP_ALL,
    GASPI_BLOCK,
    GASPI_MEM_UNINITIALIZED),
    "create segment");
  gaspiCheckReturn(gaspi_segment_create(
    segmentCommunicate,
    lengthCommunicationBuffer * sizeof(dataType) + offsetCommunicate,
    GASPI_GROUP_ALL,
    GASPI_BLOCK,
    GASPI_MEM_UNINITIALIZED),
    "create segment");

  const gaspi_notification_id_t numberNotifications
    = allreduceButterflyDoubleBuffer::getNumberOfNotifications(numRanks);
  allreduceButterfly::segmentBuffer bufferReduce0 =
    {segmentReduce, offsetReduce,
     firstNotificationReduce};
  const gaspi_notification_id_t firstNoteReduce1
    = firstNotificationReduce + numberNotifications;
  allreduceButterfly::segmentBuffer bufferReduce1 = {
    segmentReduce,
    offsetReduce + len * sizeof(dataType), firstNoteReduce1};
  allreduceButterfly::segmentBuffer bufferCommunication = {
    segmentCommunicate, offsetCommunicate,
    firstNotificationCommunicate};

  allreduceButterflyDoubleBuffer reduce(
    len, data, allreduce::SUM,
    bufferReduce0, bufferReduce1, bufferCommunication,
    queueStock);

  gaspi_pointer_t ptr;
  gaspiCheckReturn(gaspi_segment_ptr(segmentReduce, &ptr),
                   "get segment pointer");

  dataType* bptr = (dataType*)((char*)ptr + offsetReduce);
  memset(bptr, 0, 2 * len * sizeof(dataType));

  //report(reduce, rank, numRanks);

  if (rank == 0) {
    std::cout << "rank 0 of " << numRanks << " has finished setup."
              << std::endl;
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  timeval timeStart, timeStop;
  gettimeofday(&timeStart, NULL);

  int state = 0;
  for (long iter=0; iter < numIter; iter++) {
    dataType* buffer = NULL;

    if (checkResults) {
      buffer = (dataType*) reduce.getActiveReducePointer();
      fill(buffer, dataType(rank), len);
    }

    reduce.signal();
    while (reduce());

    if (checkResults) {
      state |= check(buffer, rank, numRanks, len);
    }
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  gettimeofday(&timeStop, NULL);

  if (checkResults) {
    if (state == 0) {
      std::stringstream s;
      s << "rank " << rank << " finished without errors" << std::endl;
      std::cout << s.str();
    } else {
      std::stringstream s;
      s << "rank " << rank << " finished with errors" << std::endl;
      std::cout << s.str();
      return -1;
    }
  }

  std::cout.flush();
  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  if (rank == 0) {

    const double numGigaBytes
      = 2. * len * sizeof(dataType) * (numRanks - 1) / numRanks  * numIter
      / 1024. / 1024. / 1024.;
    const double seconds = timeStop.tv_sec - timeStart.tv_sec
                         + 1e-6 * (timeStop.tv_usec - timeStart.tv_usec);
    std::cout << "Total runtime " << seconds << " seconds, "
              << seconds / numIter << " seconds per reduce, "
              << numGigaBytes / seconds << " GiB/s." << std::endl;
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
  gaspiCheckReturn(gaspi_proc_term(GASPI_BLOCK), "gaspi proc term");

  return 0;
}
