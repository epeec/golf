/*****************************************************************************
                           broadcast.cpp  -  description
                             -------------------
    begin                : November 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "broadcast.h"
#include "gaspiCheckReturn.hpp"
#include "mailBoxGaspi.h"

#include <vector>
#include <algorithm>

broadcast::broadcast(
  const gaspi_rank_t master_,
  const long len,
  const gaspi_segment_id_t segment_,
  const gaspi_offset_t offset_,
  const gaspi_notification_id_t firstNotification_,
  queues& queues_ )
:  totalLength(len),
   group(GASPI_GROUP_ALL),
   numRanks(getNumRanks()),
   rank(getRank()),
   masterRank(master_),
   depth(calcDepth(numRanks)),
   segment(segment_),
   offset(offset_),
   firstNotification(firstNotification_),
   sender(queues_),
   status((rank == masterRank) ? 1 : (depth + 1)){

  std::vector<gaspi_rank_t> ranks(numRanks);
  gaspiCheckReturn(gaspi_group_ranks(group, &ranks[0]),
                   "gaspi_group_ranks failed with");
  const unsigned long rankIndex = getRankIndex(rank, ranks);
  const unsigned long masterIndex = getRankIndex(masterRank, ranks);

  if (rank == masterRank) {
    setForward(rankIndex, ranks);
  }
  setBackward(rankIndex, masterIndex, ranks);
}

long broadcast::getNumRanks() const {
  gaspi_number_t size;
  gaspiCheckReturn(gaspi_group_size(group, &size),
                   "gaspi_group_size failed with ");
  return size;
}

long broadcast::getRank() {
  gaspi_rank_t rank;
  gaspiCheckReturn(gaspi_proc_rank(&rank),
                   "gaspi_proc_rank failed with ");
  return rank;
}

long broadcast::calcDepth(const long numRanks) {
  unsigned long n;
  for (n=0; (unsigned long)numRanks > (((unsigned long)(1)) << n); n++);

  if ((((unsigned long)(1)) << n) != (unsigned long)numRanks) {
    throw std::runtime_error("number of ranks must be power of 2");
  }

  return n;
}

long broadcast::getRankIndex(gaspi_rank_t rank,
                             const std::vector<gaspi_rank_t>& ranks) {
  unsigned long rankIndex;
  if (find(ranks.begin(), ranks.end(), rank) == ranks.end()) {
    throw std::runtime_error("rank not member of group");
  } else {
    rankIndex = find(ranks.begin(), ranks.end(), rank)
              - ranks.begin();
  }
  return rankIndex;
}

void broadcast::setForward(
  const unsigned long rankIndex,
  const std::vector<gaspi_rank_t>& ranks) {

  for (unsigned long i=0; i < (unsigned long)(numRanks); i++) {
    if (i == rankIndex) continue;

    const unsigned long chunk = getFirstChunkIndex(i);
   writer::transferParameters job(
      true,
      ranks[i],
      segment,
      offset + chunkIndexToByte(chunk),
      segment,
      offset + chunkIndexToByte(chunk),
      chunkIndexToByte(chunk + 1) - chunkIndexToByte(chunk),
      firstNotification + depth);
    jobs.push_back(job);
  }
}

void broadcast::setBackward(
  const unsigned long rankIndex,
  const unsigned long masterRankIndex,
  const std::vector<gaspi_rank_t>& ranks) {
  const unsigned long chunkIndexFull = getFirstChunkIndex(rankIndex);

  for (long level=depth - 1; level >= -1; level--) {
    const unsigned long partnerIndex = getPartnerIndex(rankIndex, level);

    if (rank != masterRank) {
      receiver.push_back(
        new mailBoxGaspi(segment, firstNotification + level + 1));
    } else if (level == (depth - 1)) {
      receiver.push_back(&trigger);
    }

    if ((level < 0) || (partnerIndex == masterRankIndex)) {
      jobs.push_back(writer::transferParameters());
    } else {
      const long transferStartIndex
        = getChunkIndexBroadcast(chunkIndexFull, level);
      const long transferStart = chunkIndexToByte(transferStartIndex);
      const long transferLength =
        chunkIndexToByte(transferStartIndex + getChunkLengthIndex(level + 1))
        - transferStart;
      writer::transferParameters transfer(
        true,
        ranks[partnerIndex],
        segment,
        offset + transferStart,
        segment,
        offset + transferStart,
        transferLength,
        firstNotification + level);
      jobs.push_back(transfer);
    }
  }
}

inline unsigned long broadcast::getFirstChunkIndex(
  const unsigned long rankIndex) const {
  unsigned long r = 0;
  for (long i=0; i < depth; i++) {
    r |= getCunkIndexExtension(rankIndex, i);
  }
  return r;
}

inline unsigned long broadcast::getPartnerIndex(
  const unsigned long rankIndex,
  const long level) const {
  return rankIndex ^ (((unsigned long) 1) << level);
}

unsigned long broadcast::getCunkIndexExtension(
  const unsigned long rankIndex,
  const long level) const {
  return ((((unsigned long) rankIndex) >> level) & 1) << (depth - 1 - level);
}

inline long broadcast::getChunkLengthIndex(const long level) const {
  return ((unsigned long) numRanks) >> level;
}

inline unsigned long broadcast::chunkIndexToByte(
  const long chunkIndex) const {
  return ((totalLength * chunkIndex + numRanks - 1) / numRanks);
}

inline char* broadcast::getSegmentPointer(
  const gaspi_segment_id_t segment) {
  gaspi_pointer_t p;
  gaspiCheckReturn(gaspi_segment_ptr(segment, &p),
                   "failed getting segment pointer");
  return (char*) p;
}

unsigned long broadcast::getChunkIndexBroadcast(
  const unsigned long chunkIndexCarry,
  const long level) const {
  return chunkIndexCarry & ((~((unsigned long) 0)) << (depth - 1 - level));
}

broadcast::~broadcast() {
  for (unsigned long i=0; i < receiver.size(); i++) {
    if ((i == 0) && (rank == masterRank)) continue;
    delete receiver[i];
  }
}

int broadcast::operator()() {
  const unsigned long phase = status.get();
  // could be a problem if we overtake one iteration?
  if (!receiver[phase]->gotNotification()) {
    return -1;
  }

  if (rank == masterRank) {
    for (unsigned long i=0; i < jobs.size(); i++) {
      sender(jobs[i]);
    }
  } else {
    sender(jobs[phase]);
  }

  return (status.increment() == 0) ? 0 : -1;
}

void broadcast::signal() {
  trigger.notify();
}

long broadcast::getNumberOfNotifications(const long numRanks) {
  return calcDepth(numRanks) + 1;
}

std::ostream& broadcast::report(std::ostream& s) const {
  const unsigned long phase = status.get();
  s << "total length: " << totalLength << std::endl
    << "numRanks: " << numRanks  << std::endl
    << "rank: " << rank << std::endl
    << "masterRank: " << masterRank << std::endl
    << "depth: " << depth << std::endl
    << "segment: " << long(segment) << std::endl
    << "offset: " << offset << std::endl
    << "firstNotification: " << firstNotification << std::endl
    << std::endl
    << "pointer segment: "
    << (void*)getSegmentPointer(segment) << std::endl
    << "phase " << phase << std::endl;
  for (unsigned long i=0; i < jobs.size(); i++) {
    s << ".........................." << std::endl;
    s << "phase " << i << std::endl;
    if ((i==0) && (rank == masterRank)) {
      s << "Receiver: " << "user" << std::endl;
    } else {
      if (i < receiver.size()) {
        mailBoxGaspi* m = (mailBoxGaspi*) receiver[i];
        s << "Receiver: segment " << long(m->getSegmentID())
          << " notification ID " << m->getMailID() << std::endl;
      } else {
        s << "Receiver: idle" << std::endl;
      }
    }

    s << "Send    : ";
    jobs[i].report(s) << std::endl;
  }
  s << ".........................." << std::endl;

  return s;
}

