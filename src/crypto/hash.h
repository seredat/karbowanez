// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2019, Karbo developers
//
// This file is part of Karbo.
//
// Karbo is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Karbo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Karbo.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <stddef.h>

#include <CryptoTypes.h>
#include "generic-ops.h"
#include "balloon_b.h"
#include "balloon_c.h"
#include "balloon_g.h"
#include "balloon_j.h"
#include "balloon_s.h"

namespace Crypto {

  extern "C" {
#include "hash-ops.h"
  }

  /*
    Cryptonight hash functions
  */

  inline void cn_fast_hash(const void *data, size_t length, Hash &hash) {
    cn_fast_hash(data, length, reinterpret_cast<char *>(&hash));
  }

  inline Hash cn_fast_hash(const void *data, size_t length) {
    Hash h;
    cn_fast_hash(data, length, reinterpret_cast<char *>(&h));
    return h;
  }

  class cn_context {
  public:

    cn_context();
    ~cn_context();
#if !defined(_MSC_VER) || _MSC_VER >= 1800
    cn_context(const cn_context &) = delete;
    void operator=(const cn_context &) = delete;
#endif

  private:

    void *data;
    friend inline void cn_slow_hash(cn_context &, const void *, size_t, Hash &);
  };

  inline void cn_slow_hash(cn_context &context, const void *data, size_t length, Hash &hash) {
    cn_slow_hash(data, length, reinterpret_cast<char *>(&hash));
  }

  inline void balloon_slow_hash(uint8_t* input, Hash &output) {
    balloon_c(input, reinterpret_cast<uint8_t *>(&output));
  }

  inline void balloon_hash_blake(const unsigned char* input, Hash &output, int length, const unsigned char* salt, int salt_length) {
    balloon_b(input, reinterpret_cast<char *>(&output), length, salt, salt_length);
  }
  
  inline void balloon_hash_groestl(const unsigned char* input, Hash &output, int length, const unsigned char* salt, int salt_length) {
    balloon_g(input, reinterpret_cast<char *>(&output), length, salt, salt_length);
  }
  
  inline void balloon_hash_jh(const unsigned char* input, Hash &output, int length, const unsigned char* salt, int salt_length) {
    balloon_j(input, reinterpret_cast<char *>(&output), length, salt, salt_length);
  }
  
  inline void balloon_hash_skein(const unsigned char* input, Hash &output, int length, const unsigned char* salt, int salt_length) {
    balloon_s(input, reinterpret_cast<char *>(&output), length, salt, salt_length);
  }

  void (*const pump[4])(const unsigned char *, Hash &, int, const unsigned char *, int) =
  {
    balloon_hash_blake, balloon_hash_groestl, balloon_hash_jh, balloon_hash_skein
  };

  inline void tree_hash(const Hash *hashes, size_t count, Hash &root_hash) {
    tree_hash(reinterpret_cast<const char (*)[HASH_SIZE]>(hashes), count, reinterpret_cast<char *>(&root_hash));
  }

  inline void tree_branch(const Hash *hashes, size_t count, Hash *branch) {
    tree_branch(reinterpret_cast<const char (*)[HASH_SIZE]>(hashes), count, reinterpret_cast<char (*)[HASH_SIZE]>(branch));
  }

  inline void tree_hash_from_branch(const Hash *branch, size_t depth, const Hash &leaf, const void *path, Hash &root_hash) {
    tree_hash_from_branch(reinterpret_cast<const char (*)[HASH_SIZE]>(branch), depth, reinterpret_cast<const char *>(&leaf), path, reinterpret_cast<char *>(&root_hash));
  }

}

CRYPTO_MAKE_HASHABLE(Hash)
