/*****************************************************************************
                           counter.h  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#ifndef _counter_H
#define _counter_H

#include<atomic>

class counter {
public:
  counter(const unsigned long phasePeriod_ = 1);
  unsigned long increment();
  unsigned long get() const;
private:

  const unsigned long phasePeriod;
  std::atomic<unsigned long> value;
};


#endif
