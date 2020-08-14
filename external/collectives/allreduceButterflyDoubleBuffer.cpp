/*****************************************************************************
                           allreduceButterflyDoubleBuffer.cpp  -  description
                             -------------------
    begin                : December 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "allreduceButterflyDoubleBuffer.h"

allreduceButterflyDoubleBuffer::allreduceButterflyDoubleBuffer(
  const long len,
  const dataType data,
  const reductionType reduction,
  const allreduceButterfly::segmentBuffer segmentReduce0,
  const allreduceButterfly::segmentBuffer segmentReduce1,
  const allreduceButterfly::segmentBuffer segmentCommunicate,
  queues& queues,
  gaspi_group_t group)
: state(0),
  reduceFirst(len, data, reduction, segmentReduce0,
              segmentCommunicate, queues, group),
  reduceSecond(len, data, reduction, segmentReduce1,
               segmentCommunicate, queues, group) {
  tableReduce[0] = &reduceFirst;
  tableReduce[1] = &reduceSecond;
}

int allreduceButterflyDoubleBuffer::operator()() {
  const int result = getReduce()();

  if (!result) {
    flipReduce();
  }

  return result;
}

inline allreduceButterfly& allreduceButterflyDoubleBuffer::getReduce() {
  return tableReduce[stateToIndex(state)][0];
}

inline long allreduceButterflyDoubleBuffer::stateToIndex(const long state) {
  return state & 1l;
}

inline void allreduceButterflyDoubleBuffer::flipReduce() {
  __sync_fetch_and_add(&state, 1l);
}

void allreduceButterflyDoubleBuffer::signal() {
  getReduce().signal();
}

void allreduceButterflyDoubleBuffer::wait() {
  // Warning: Use of this routine is expensive because it has a barrier.
  // And it is dangerous because it is not thread save.
  // It can especially not be used at the same time as operator()
  // No thread safety for this function with other methods. Use at own risk.
  getReduce().wait();
  flipReduce();
}

gaspi_pointer_t allreduceButterflyDoubleBuffer::getActiveReducePointer() {
  return getReduce().getReducePointer();
}

gaspi_pointer_t allreduceButterflyDoubleBuffer::getResultsPointer() const {
  return getOtherReduce().getReducePointer();
}

inline const allreduceButterfly&
  allreduceButterflyDoubleBuffer::getOtherReduce() const {
  return tableReduce[invertIndex(stateToIndex(state))][0];
}

inline long allreduceButterflyDoubleBuffer::invertIndex(const long state) {
  return state ^ 1l;
}

long allreduceButterflyDoubleBuffer::getNumberOfElementsSegmentCommunicate(
  const long len,
  const long numRanks) {
  return allreduceButterfly::getNumberOfElementsSegmentCommunicate(len,
                                                                  numRanks);
}

unsigned long allreduceButterflyDoubleBuffer::getNumberOfNotifications(
  const long numRanks) {
  return allreduceButterfly::getNumberOfNotifications(numRanks);
}

std::ostream& allreduceButterflyDoubleBuffer::report(std::ostream& s) const {
  s << "stateExecute: " << state << std::endl
    << "***** reduceFirst *****" << std::endl;
  reduceFirst.report(s);
  s << "***** reduceSecond *****" << std::endl;
  reduceSecond.report(s);

  return s;
}

