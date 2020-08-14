/*****************************************************************************
                           allreduce.h  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#ifndef _ALLREDUCE_H
#define _ALLREDUCE_H

class allreduce {
public:
  enum reductionType {
    SUM = 0,
    AVERAGE = 1,
    NUM_RED = 2
  };
  enum dataType {
    FLOAT = 0,
    DOUBLE = 1,
    INT16 = 2,
    INT32 = 3,
    NUM_TYPE = 4
  };

  virtual int operator()() = 0;
  virtual void signal() = 0;
  virtual ~allreduce() {}
};


#endif
