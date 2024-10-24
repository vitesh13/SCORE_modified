/*
* This file is part of paraprobe-toolbox.
*
* paraprobe-toolbox is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License,
*  or (at your option) any later version.
*
* paraprobe-toolbox is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with paraprobe-toolbox. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __OPENSSLINTERFACE__
#define __OPENSSLINTERFACE__

#include "../STLInterface.h"

//##MK::https://gist.github.com/arrieta/7d2e196c40514d8b5e9031f2535064fc
//#include <cstdlib>


// OpenSSL Library
extern "C" {
#include <openssl/sha.h>
}


string SHA256(const char* const path);

#endif
