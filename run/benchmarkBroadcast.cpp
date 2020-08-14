/*****************************************************************************
                           benchmarkBroadcast.cpp  -  description
                             -------------------
    begin                : November 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "broadcast.h"
#include "gaspiCheckReturn.hpp"

#include <GASPI.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstring>
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
          const long n) {
  int state = 0;
  for (long i=0; i < n; i++) {
    const T expect = i + rank + 1;
    if (a[i] != expect) {
      std::stringstream s;
      s << "Bad result in rank " << rank << ", at position " << i
        << ", "<< double(a[i]) << " expected " << double(expect) << std::endl;
      std::cerr << s.str();
      state = 1;
    }
  }
  return state;
}

void report(broadcast& r,
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

  typedef unsigned char dataType;


  if ((argc < 3) || (argc > 4)) {
    std::cerr << argv[0] << ": Usage " << argv[0] << " <length bytes>"
              << " <num iterations> [check]"
              << std::endl;
    return -1;
  }

  const long len = atol(argv[1]) / sizeof(dataType);
  const long numIter = atol(argv[2]);
  const bool checkResults = (argc==4)?true:false;

  const gaspi_segment_id_t segment = 1;
  queues queueStock(2);
  const gaspi_offset_t offset = 64;
  const gaspi_notification_id_t firstNotification = 2;

  gaspiCheckReturn(gaspi_proc_init(GASPI_BLOCK), "gaspi proc init");

//  gaspiCheckReturn(gaspi_group_commit(GASPI_GROUP_ALL, GASPI_BLOCK),
//                   "gaspi group commit");

  gaspi_rank_t numRanks;
  gaspiCheckReturn(gaspi_proc_num(&numRanks), "get number of ranks");
  gaspi_rank_t rank;
  gaspiCheckReturn(gaspi_proc_rank(&rank), "get rank");
  const long depth = log(numRanks) / log(2);
  if (1 << depth != numRanks) {
    std::cerr << "Not power of 2" << std::endl;
    return -1;
  }
  const gaspi_rank_t masterRank = numRanks - 1;

  gaspiCheckReturn(gaspi_segment_create(
    segment,
    2 * len * sizeof(dataType) + offset,
    GASPI_GROUP_ALL,
    GASPI_BLOCK,
    GASPI_MEM_UNINITIALIZED),
    "create segment");

  broadcast bc0(masterRank , len, segment, offset, firstNotification, queueStock);
  broadcast bc1(
    masterRank,
    len,
    segment,
    offset + len * sizeof(dataType),
    firstNotification + broadcast::getNumberOfNotifications(numRanks),
    queueStock);

  gaspi_pointer_t ptr;
  gaspiCheckReturn(gaspi_segment_ptr(segment, &ptr), "get segment pointer");

  dataType* b0 = (dataType*)((char*)ptr + offset);
  dataType* b1 = (dataType*)((char*)ptr + offset + len * sizeof(dataType));

  memset(b0, 0, len * sizeof(dataType));
  memset(b1, 0, len * sizeof(dataType));

//  report(bc0, rank, numRanks);

  if (rank == 0) {
    std::cout << "rank 0 of " << numRanks << " has finished setup."
              << std::endl;
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  timeval timeStart, timeStop;
  gettimeofday(&timeStart, NULL);

  int state = 0;
  for (long iter=0; iter < numIter; iter++) {
    if (checkResults) {
      if (rank == masterRank) {
        fill(b0, dataType(rank), len);
      } else {
        memset(b0, 0, len * sizeof(dataType));
      }
      gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
    }

    if (rank == masterRank) {
      bc0.signal();
    }
    while (bc0());

    if (checkResults) {
      state |= check(b0, masterRank, len);
    }

    if (checkResults) {
      if (rank == masterRank) {
        fill(b1, dataType(rank), len);
      } else {
        memset(b1, 0, len * sizeof(dataType));
      }
      gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
    }

    if (rank == masterRank) {
      bc1.signal();
    }
    while (bc1());

    if (checkResults) {
      state |= check(b1, masterRank, len);
    }

    gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
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
      = 2. * len * sizeof(dataType) * (numRanks - 1) / numRanks  * 2 * numIter
      / 1024. / 1024. / 1024.;
    const double seconds = timeStop.tv_sec - timeStart.tv_sec
                         + 1e-6 * (timeStop.tv_usec - timeStart.tv_usec);
    std::cout << "Total runtime " << seconds << " seconds, "
              << seconds / 2 / numIter << " seconds per broadcast, "
              << numGigaBytes / seconds << " GiB/s." << std::endl;
  }

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
  gaspiCheckReturn(gaspi_proc_term(GASPI_BLOCK), "gaspi proc term");

  return 0;
}
