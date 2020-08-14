/*****************************************************************************
                           benchmark.cpp  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include <mpi.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

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


int main(int argc, char** argv) {

  MPI_Init(&argc, &argv);

  typedef double dataType;
  const MPI_Datatype data = MPI_DOUBLE;


  if ((argc < 3) || (argc > 4)) {
    std::cerr << argv[0] << ": Usage " << argv[0] << " <length bytes>"
              << " <num iterations> [check]"
              << std::endl;
    return -1;
  }

  const long len = atol(argv[1]) / sizeof(dataType);
  const long numIter = atol(argv[2]);
  const bool checkResults = (argc==4)?true:false;



  dataType* a0 = new dataType[len];
  dataType* a1 = new dataType[len];
  dataType* b0 = new dataType[len];
  dataType* b1 = new dataType[len];
  memset(a0, 0, len * sizeof(dataType));
  memset(a1, 0, len * sizeof(dataType));

  int numRanks;
  MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    std::cout << "rank 0 of " << numRanks << " has finished setup."
              << std::endl;
  }

  MPI_Barrier(MPI_COMM_WORLD);

  timeval timeStart, timeStop;
  gettimeofday(&timeStart, NULL);

  int state = 0;
  for (long iter=0; iter < numIter; iter++) {
    if (checkResults) {
      fill(a0, dataType(rank), len);
    }
    MPI_Allreduce(a0, b0, len, data, MPI_SUM, MPI_COMM_WORLD);
    if (checkResults) {
      state |= check(b0, rank, numRanks, len);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

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
  MPI_Barrier(MPI_COMM_WORLD);

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

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}
