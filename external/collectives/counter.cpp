/*****************************************************************************
                           counter.cpp  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "counter.h"


counter::counter(const unsigned long phasePeriod_)
:  phasePeriod(phasePeriod_),
   value(0) {}

unsigned long counter::increment() {
  return (++value) % phasePeriod;
}

unsigned long counter::get() const {
  return value % phasePeriod;
}
