// SPDX-License-Identifier: GPL-3.0-or-later

#include "OpenSslHandling.h"

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
