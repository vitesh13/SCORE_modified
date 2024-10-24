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

#include "OPENSSLInterface.h"

string SHA256(const char* const path)
{
	ifstream fp(path, ios::in | ios::binary);

	if (not fp.good()) {
		ostringstream os;
		cerr << "Can not open path " << path << "\n";
		return "";
	}

	constexpr const size_t buffer_size { 1 << 12 };
	char buffer[buffer_size];

	unsigned char hash[SHA256_DIGEST_LENGTH] = { 0 };

	SHA256_CTX ctx;
	SHA256_Init(&ctx);

	while (fp.good()) {
		fp.read(buffer, buffer_size);
		SHA256_Update(&ctx, buffer, fp.gcount());
	}

	SHA256_Final(hash, &ctx);
	fp.close();

	ostringstream os;
	os << hex << setfill('0');

	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		os << setw(2) << static_cast<unsigned int>(hash[i]);
	}

	return os.str();
}
