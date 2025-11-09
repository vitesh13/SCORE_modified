// SPDX-License-Identifier: GNU General Public License v3.0 or later

#ifndef __OPENSSLINTERFACE__
#define __OPENSSLINTERFACE__

#include "../STL_Interface.h"

//##MK::https://gist.github.com/arrieta/7d2e196c40514d8b5e9031f2535064fc
//#include <cstdlib>


// OpenSSL Library
extern "C" {
#include <openssl/sha.h>
}


string SHA256(const char* const path);

#endif
