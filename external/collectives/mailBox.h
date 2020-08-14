/*****************************************************************************
                           mailBox.h  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#ifndef _MAIL_BOX_H
#define _MAIL_BOX_H


class mailBox {
public:
  virtual bool gotNotification() = 0;
  virtual ~mailBox() = default;
};

#endif
