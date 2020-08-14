/*****************************************************************************
                           mailBoxLocal.h  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "mailBox.h"
#include <atomic>

#ifndef _MAIL_BOX_LOCAL_H
#define _MAIL_BOX_LOCAL_H


class mailBoxLocal : public mailBox {
public:
  mailBoxLocal();
  bool gotNotification() override;
  void notify();

private:

  std::atomic<unsigned long> status;
  std::atomic<unsigned long> target;
};

#endif
