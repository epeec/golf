/*****************************************************************************
                           mailBoxLocal.cpp  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "mailBoxLocal.h"

mailBoxLocal::mailBoxLocal()
: status(0),
  target(0) {}

bool mailBoxLocal::gotNotification() {
  unsigned long statusOld = status;
  return (statusOld < target) && status.compare_exchange_strong(statusOld, statusOld + 1);
}

void mailBoxLocal::notify() {
  ++target;
}
