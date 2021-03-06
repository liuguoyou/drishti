/*!
  @file   finder/Device.cpp
  @author David Hirvonen
  @brief Device settings.

  \copyright Copyright 2014-2016 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}

*/

#include <QtGlobal> // for Q_OS_*
#if !defined(Q_OS_IOS)
void adjustScreen(float value) {}
#endif
