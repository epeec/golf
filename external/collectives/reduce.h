/*****************************************************************************
                           reduce.hpp  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#ifndef _REDUCE_H
#define _REDUCE_H


#include "allreduce.h"

#include <string.h>


class reduce {
public:
  struct task {
    const void* source;
    void* destination;
    long len;
    unsigned long scaling;
    task(const void* s = NULL,
         void* d = NULL,
         long n = 0,
         unsigned long sc = 0)
    : source(s), destination(d), len(n), scaling(sc) {}
  };

  virtual void operator()(const task& t) const = 0;
  virtual ~reduce() {}
};

reduce * getReduce(const allreduce::dataType data,
                   const allreduce::reductionType reduction);

size_t getDataTypeSize(const allreduce::dataType d);

#endif
