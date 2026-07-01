// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __TIMEANDDATEHANDLING__
#define __TIMEANDDATEHANDLING__

#include "../STL_Interface.h"

//time management
//https://github.com/HowardHinnant/date
//https://stackoverflow.com/questions/9527960/how-do-i-construct-an-iso-8601-datetime-in-c
//interface to Howard Hinnart's customized datetime library building on chrono
#include "../date/include/date/date.h"


template <class Precision>
string getISOCurrentTimestamp();


string timestamp_now_iso8601();


#endif
