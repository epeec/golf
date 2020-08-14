/*****************************************************************************
                           mailBoxGaspi.h  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include "mailBox.h"

#include <GASPI.h>


#ifndef _MAIL_BOX_GASPI_H
#define _MAIL_BOX_GASPI_H


class mailBoxGaspi : public mailBox {
public:
  mailBoxGaspi(const gaspi_segment_id_t segmentID_,
               const gaspi_notification_id_t mailID_);
  bool gotNotification() override;
  gaspi_segment_id_t getSegmentID() const;
  gaspi_notification_id_t getMailID() const;

private:

  const gaspi_segment_id_t segmentID;
  const gaspi_notification_id_t mailID;
};

#endif
