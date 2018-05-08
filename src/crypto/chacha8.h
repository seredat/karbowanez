#pragma once

#include <stdint.h>
#include <stddef.h>

#define CHACHA8_KEY_SIZE 32
#define CHACHA8_IV_SIZE 8

#if defined(__cplusplus)
#include <memory.h>
#include <string>

#include "hash.h"
#include "cn_slow_hash.hpp"

namespace Crypto {
  extern "C" {
#endif
    void chacha8(const void* data, size_t length, const uint8_t* key, const uint8_t* iv, char* cipher);
#if defined(__cplusplus)
  }

#pragma pack(push, 1)
  struct chacha8_key {
    uint8_t data[CHACHA8_KEY_SIZE];

    ~chacha8_key()
    {
      memset(data, 0, sizeof(data));
    }
  };

  // MS VC 2012 doesn't interpret `class chacha8_iv` as POD in spite of [9.0.10], so it is a struct
  struct chacha8_iv {
    uint8_t data[CHACHA8_IV_SIZE];
  };
#pragma pack(pop)

  static_assert(sizeof(chacha8_key) == CHACHA8_KEY_SIZE && sizeof(chacha8_iv) == CHACHA8_IV_SIZE, "Invalid structure size");

  inline void chacha8(const void* data, size_t length, const chacha8_key& key, const chacha8_iv& iv, char* cipher) {
    chacha8(data, length, reinterpret_cast<const uint8_t*>(&key), reinterpret_cast<const uint8_t*>(&iv), cipher);
  }

  inline void generate_chacha8_key(const void *data, size_t size, chacha8_key& key) {
    static_assert(sizeof(chacha8_key) <= sizeof(Hash), "Size of hash must be at least that of chacha8_key");
	uint8_t pwd_hash[HASH_SIZE];
	cn_pow_hash_v1 kdf_hash;
	kdf_hash.hash(data, size, pwd_hash);
    memcpy(&key, &pwd_hash, sizeof(key));
    memset(&pwd_hash, 0, sizeof(pwd_hash));
  }

  inline void generate_chacha8_key(std::string password, chacha8_key& key) {
	  return generate_chacha8_key(password.data(), password.size(), key);
  }

  inline void generate_chacha8_key(Crypto::cn_context &context, const std::string& password, chacha8_key& key) {
	  return generate_chacha8_key(password.data(), password.size(), key);
  }
}

#endif
