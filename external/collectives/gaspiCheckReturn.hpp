/*****************************************************************************
                           gaspiCheckReturn.hpp  -  description
                             -------------------
    begin                : October 2019
    copyright            : (C) 2019 by Martin Kuehn
    email                : Martin.Kuehn@itwm.fraunhofer.de
*****************************************************************************/

#include <GASPI.h>
#include <string>
#include <cstdlib>
#include <stdexcept>

#ifndef _GASPI_CHECK_RETURN_H
#define _GASPI_CHECK_RETURN_H

inline void gaspiCheckReturn(const gaspi_return_t err,
                             const std::string prefix = "") {
  if (err != GASPI_SUCCESS) {
    gaspi_string_t raw;
    gaspi_print_error(err, &raw);
    std::string message = prefix + std::string(raw);
    free(raw);
    throw std::runtime_error(message);
  }
}

#endif
