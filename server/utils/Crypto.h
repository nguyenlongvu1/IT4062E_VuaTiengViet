#pragma once
#include <string>

namespace Crypto {
    // Compute SHA-256 hex string of input
    std::string sha256(const std::string &input);
    // Generate random hex token of given byte length (bytes -> hex length = bytes*2)
    std::string randomHex(size_t bytes = 32);
}
