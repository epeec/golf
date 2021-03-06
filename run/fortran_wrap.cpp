#include "allreduceButterflyDoubleBuffer.h"
#include <GASPI.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include <sys/time.h>
#include <memory>


typedef double dataType;
static const allreduce::dataType data = allreduce::DOUBLE;
static const gaspi_segment_id_t segmentReduce = 1;
static const gaspi_segment_id_t segmentCommunicate = 2;
static const gaspi_offset_t offsetReduce = 64;
static const gaspi_offset_t offsetCommunicate = 32;
static const gaspi_notification_id_t firstNotificationReduce = 2;
static const gaspi_notification_id_t firstNotificationCommunicate = 5;
static queues queueStock(2);
static std::unique_ptr<allreduceButterflyDoubleBuffer> reduce_ptr;
static gaspi_config_t default_conf;


extern "C" {
  void init_gaspi_f();
  void setup_double_buffers_f(int *lenIn);
  void gaspi_allreduce_sum_f(double *my_buffer, int *lenIn, int *checkBit);
  void close_gaspi_f();
  void gaspi_barrier_f();
}


inline void gaspiCheckReturn(const gaspi_return_t err,
                             const std::string prefix = "") {
  if (err != GASPI_SUCCESS) {
    gaspi_string_t raw;
    gaspi_print_error(err, &raw);
    std::string message = prefix + std::string(raw);
    free(raw);
    throw std::runtime_error(message);
  }
}


template <class T>
void fill(T a[],
         const T rank,
         const long n) {
  for (long i=0; i < n; i++)
    a[i] = i + rank + 1;
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


void
setup_double_buffers_f(int *lenIn) {
  auto len = *lenIn;
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

  reduce_ptr =
  std::unique_ptr<allreduceButterflyDoubleBuffer>
  (
    new allreduceButterflyDoubleBuffer(
      len, data, allreduce::SUM,
      bufferReduce0, bufferReduce1, bufferCommunication,
      queueStock
    )
  );

  gaspi_pointer_t ptr;
  gaspiCheckReturn(gaspi_segment_ptr(segmentReduce, &ptr),
                   "get segment pointer");

  dataType* bptr = (dataType*)((char*)ptr + offsetReduce);
  memset(bptr, 0, 2 * len * sizeof(dataType));

}

void
free_double_buffer() {
  auto manual_ptr = reduce_ptr.release();
  delete [] manual_ptr;
}

int clean_runner(
  int *lenIn,
  int *checkBit
) {
  auto len = *lenIn;
  auto checkResults = (*checkBit == 0) ? false : true;
  gaspi_rank_t numRanks;
  gaspiCheckReturn(gaspi_proc_num(&numRanks), "get number of ranks");
  gaspi_rank_t rank;
  gaspiCheckReturn(gaspi_proc_rank(&rank), "get rank");

  auto reduce = reduce_ptr.get();

  if (rank == 0)
    std::cout << "rank 0 of " << numRanks << " has finished setup."
              << std::endl;

  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");

  timeval timeStart, timeStop;
  gettimeofday(&timeStart, NULL);

  int state = 0;

  dataType* buffer = NULL;

  if (checkResults) {
    buffer = (dataType*) reduce->getActiveReducePointer();
    fill(buffer, dataType(rank), len);
  }

  reduce->signal();
  while ( (*reduce)() );

  if (checkResults)
    state |= check(buffer, rank, numRanks, len);

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
      = 2. * len * sizeof(dataType) * (numRanks - 1) / numRanks / 1024. / 1024. / 1024.;
    const double seconds = timeStop.tv_sec - timeStart.tv_sec
                         + 1e-6 * (timeStop.tv_usec - timeStart.tv_usec);
    std::cout << "Total runtime " << seconds << " seconds, "
              << seconds << " seconds per reduce, "
              << numGigaBytes / seconds << " GiB/s." << std::endl;
  }

  return 0;
}

void gaspi_allreduce_sum_f(
  double *my_buffer,
  int *lenIn,
  int *checkBit
) {
  std::cout << "Start of AllReduce\n";  
  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
  auto len = *lenIn;
  auto checkResults = (*checkBit == 0) ? false : true;
  auto reduce = reduce_ptr.get();
  dataType* buffer = NULL;
  buffer = (dataType*) reduce->getActiveReducePointer();
  std::copy(my_buffer, my_buffer + len, buffer);
  reduce->signal();
  while ( (*reduce)() );
  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
  std::copy(buffer, buffer + len, my_buffer);
  std::cout << "End of AllReduce\n";
}


void init_gaspi_f() {
  gaspi_config_get (&default_conf);
  default_conf.build_infrastructure = GASPI_TOPOLOGY_DYNAMIC;
  gaspi_config_set(default_conf);
  gaspiCheckReturn(gaspi_proc_init(GASPI_BLOCK), "gaspi proc init");
}


void close_gaspi_f() {
  gaspiCheckReturn(gaspi_proc_term(GASPI_BLOCK), "gaspi proc term");
}

void gaspi_barrier_f() {
  gaspiCheckReturn(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK), "gaspi barrier");
}


// int main(int argc, char** argv) {

//   int len = 4;
//   int checkResults = 1;
//   std::vector<double> buffer(len, 1.0);

//   init_gaspi_f();
//   setup_double_buffers_f(&len);
//   gaspi_allreduce_sum_f( buffer.data(), &len, &checkResults);
//   gaspi_barrier_f();
//   std::cout << "Result = ";
//   for(int i=0;i <len; ++i)
//     std::cout << buffer[i] << "  ";
//   std::cout << "\n";  
//   close_gaspi_f();
//   return 0;
// }
