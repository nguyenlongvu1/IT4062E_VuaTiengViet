#include "Crypto.h"
#include <openssl/sha.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace Crypto {

std::string sha256(const std::string &input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

std::string randomHex(size_t bytes) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist(0, std::numeric_limits<unsigned long long>::max());
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    size_t generated = 0;
    while (generated < bytes) {
        unsigned long long v = dist(gen);
        ss << std::setw(16) << (v);
        generated += 8; // 8 bytes produced
    }
    std::string out = ss.str();
    // trim to desired length (bytes*2 hex chars)
    if (out.size() > bytes * 2) out.resize(bytes * 2);
    return out;
}

}
