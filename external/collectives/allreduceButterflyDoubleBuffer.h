/*****************************************************************************
                           allreduceButterflyDoubleBuffer.h  -  description
                             -------------------
    begin                : December 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/


#ifndef _ALLREDUCE_BUTTERFLY_DOUBLE_BUFFER_H
#define  _ALLREDUCE_BUTTERFLY_DOUBLE_BUFFER_H

#include "allreduceButterfly.h"

class allreduceButterflyDoubleBuffer : public allreduce {
public:

  allreduceButterflyDoubleBuffer(
    const long len,
    const dataType data,
    const reductionType reduction,
    const allreduceButterfly::segmentBuffer segmentReduce0,
    const allreduceButterfly::segmentBuffer segmentReduce1,
    const allreduceButterfly::segmentBuffer segmentCommunicate,
    queues& queues,
    gaspi_group_t group = GASPI_GROUP_ALL);
  int operator()();
  void signal();
  void wait(); // use depreciated, consider comment in implementation

  gaspi_pointer_t getActiveReducePointer();
  gaspi_pointer_t getResultsPointer() const;
  static long getNumberOfElementsSegmentCommunicate(const long len,
                                                    const long numRanks);
  static unsigned long getNumberOfNotifications(const long numRanks);
  std::ostream& report(std::ostream& s) const;

private:

  inline allreduceButterfly& getReduce();
  static inline long stateToIndex(const long state);
  inline void flipReduce();
  inline const allreduceButterfly& getOtherReduce() const;
  static inline long invertIndex(const long state);

  static const long CACHE_LINE_SIZE = 64;

  char pad0[CACHE_LINE_SIZE];
  volatile long state;
  char pad1[CACHE_LINE_SIZE];

  allreduceButterfly reduceFirst;
  allreduceButterfly reduceSecond;
  allreduceButterfly* tableReduce[2];
};

#endif
