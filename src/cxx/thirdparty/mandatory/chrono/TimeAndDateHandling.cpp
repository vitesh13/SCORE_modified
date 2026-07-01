// SPDX-License-Identifier: GPL-3.0-or-later

#include "TimeAndDateHandling.h"

template <class Precision>
string getISOCurrentTimestamp()
{
    auto now = chrono::system_clock::now();
    return date::format("%FT%TZ", date::floor<Precision>(now));
}


string timestamp_now_iso8601()
{
	stringstream sstream;
	sstream << getISOCurrentTimestamp<chrono::microseconds>();
	return sstream.str();
}
