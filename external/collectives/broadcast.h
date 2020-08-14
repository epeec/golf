/*****************************************************************************
                           broadcast.h  -  description
                             -------------------
    begin                : November 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/


#ifndef _BROADCAST_H
#define _BROADCAST_H

#include "writer.h"
#include "mailBox.h"
#include "mailBoxLocal.h"
#include "counter.h"
#include "queues.h"

#include <GASPI.h>
#include <iostream>

class broadcast {
public:
  broadcast(const gaspi_rank_t master_,
            const long len,
            const gaspi_segment_id_t segment_,
            const gaspi_offset_t offset_,
            const gaspi_notification_id_t firstNotification_,
            queues& queues_);
  ~broadcast();
  int operator()();
  void signal();
  static long getNumberOfNotifications(const long numRanks);
  std::ostream& report(std::ostream& s) const;

private:

  long getNumRanks() const;
  static long getRank();
  static long calcDepth(const long numRanks);
  static long getRankIndex(gaspi_rank_t rank,
                           const std::vector<gaspi_rank_t>& ranks);
  void setForward(const unsigned long rankIndex,
                  const std::vector<gaspi_rank_t>& ranks);
  inline unsigned long getFirstChunkIndex(const unsigned long rankIndex) const;
  void setBackward(const unsigned long rankIndex,
                   const unsigned long masterRankIndex,
                   const std::vector<gaspi_rank_t>& ranks);
  inline unsigned long getPartnerIndex(const unsigned long rankIndex,
                                       const long level) const;
  inline unsigned long getCunkIndexExtension(const unsigned long rankIndex,
                                             const long level) const;
  inline long getChunkLengthIndex(const long level) const;
  inline unsigned long chunkIndexToByte(const long chunkIndex) const;
  inline static char* getSegmentPointer(const gaspi_segment_id_t segment);
  inline unsigned long getChunkIndexBroadcast(
    const unsigned long chunkIndexCarry,
    const long level) const;

  const long totalLength;
  const gaspi_group_t group;
  const long numRanks;
  const gaspi_rank_t rank;
  const gaspi_rank_t masterRank;
  const long depth;
  const gaspi_segment_id_t segment;
  const gaspi_offset_t offset;
  const gaspi_notification_id_t firstNotification;

  mailBoxLocal trigger;
  std::vector<mailBox *> receiver;
  std::vector<writer::transferParameters> jobs;

  writer sender;
  counter status;
};

#endif
