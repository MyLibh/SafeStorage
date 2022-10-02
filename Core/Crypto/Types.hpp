#pragma once

#include <array>

#include <openssl/sha.h>

namespace core {

using sha256_t = std::array<uint8_t, SHA256_DIGEST_LENGTH>;

} // namespace core