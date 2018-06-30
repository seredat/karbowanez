// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin developers
// Copyright (c) 2018, The Karbo developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <iterator>
#include <boost/regex.hpp>

#include "Checkpoints.h"
#include "Common/StringTools.h"
#include "Common/DnsTools.h"

using namespace Logging;

namespace CryptoNote {
//---------------------------------------------------------------------------
Checkpoints::Checkpoints(Logging::ILogger &log) : logger(log, "checkpoints") {}
//---------------------------------------------------------------------------
bool Checkpoints::add_checkpoint(uint32_t height, const std::string &hash_str) {
  Crypto::Hash h = NULL_HASH;

  if (!Common::podFromHex(hash_str, h)) {
    logger(WARNING) << "Wrong hash in checkpoint for height " << height;
    return false;
  }

  if (!(0 == m_points.count(height))) {
    logger(WARNING) << "Checkpoint already exists.";
    return false;
  }

  m_points[height] = h;
  return true;
}
//---------------------------------------------------------------------------
const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");
const boost::regex fieldsregx(",(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");
bool Checkpoints::load_checkpoints_from_file(const std::string& fileName) {
  std::string buff;
  if (!Common::loadFileToString(fileName, buff)) {
    logger(Logging::ERROR, BRIGHT_RED) << "Could not load checkpoints file: " << fileName;
    return false;
  }
  const char* data = buff.c_str();
  unsigned int length = strlen(data);

  boost::cregex_token_iterator li(data, data + length, linesregx, -1);
  boost::cregex_token_iterator end;

  int count = 0;
  while (li != end) {
    std::string line = li->str();
    ++li;
 
    boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
    boost::sregex_token_iterator end2;
 
    std::vector<std::string> row;
    while (ti != end2) {
      std::string token = ti->str();
      ++ti;
      row.push_back(token);
    }
    if (row.size() != 2) {
      logger(Logging::ERROR, BRIGHT_RED) << "Invalid checkpoint file format";
      return false;
    } else {
      uint32_t height = stoi(row[0]);
      bool r = add_checkpoint(height, row[1]);
      if (!r) {
        return false;
      }
      count += 1;
    }
  }

  logger(Logging::INFO) << "Loaded " << count << " checkpoint(s) from " << fileName;
  return true;
}
//---------------------------------------------------------------------------
bool Checkpoints::is_in_checkpoint_zone(uint32_t  height) const {
  return !m_points.empty() && (height <= (--m_points.end())->first);
}
//---------------------------------------------------------------------------
bool Checkpoints::check_block(uint32_t  height, const Crypto::Hash &h,
                              bool &is_a_checkpoint) const {
  auto it = m_points.find(height);
  is_a_checkpoint = it != m_points.end();
  if (!is_a_checkpoint)
    return true;

  if (it->second == h) {
    logger(Logging::INFO, Logging::GREEN) 
      << "CHECKPOINT PASSED FOR HEIGHT " << height << " " << h;
    return true;
  } else {
    logger(Logging::ERROR) << "CHECKPOINT FAILED FOR HEIGHT " << height
                           << ". EXPECTED HASH: " << it->second
                           << ", FETCHED HASH: " << h;
    return false;
  }
}
//---------------------------------------------------------------------------
bool Checkpoints::check_block(uint32_t  height, const Crypto::Hash &h) const {
  bool ignored;
  return check_block(height, h, ignored);
}
//---------------------------------------------------------------------------
bool Checkpoints::is_alternative_block_allowed(uint32_t  blockchain_height,
                                               uint32_t  block_height) const {
  if (0 == block_height)
    return false;

  auto it = m_points.upper_bound(blockchain_height);
  // Is blockchain_height before the first checkpoint?
  if (it == m_points.begin())
    return true;

  --it;
  uint32_t  checkpoint_height = it->first;
  return checkpoint_height < block_height;
}

std::vector<uint32_t> Checkpoints::getCheckpointHeights() const {
  std::vector<uint32_t> checkpointHeights;
  checkpointHeights.reserve(m_points.size());
  for (const auto& it : m_points) {
    checkpointHeights.push_back(it.first);
  }

  return checkpointHeights;
}
#ifndef __ANDROID__
//---------------------------------------------------------------------------
bool Checkpoints::load_checkpoints_from_dns()
{
  std::string domain("checkpoints.karbo.org");
  std::vector<std::string>records;

  if (!Common::fetch_dns_txt(domain, records)) {
    logger(Logging::INFO) << "Failed to lookup DNS checkpoint records from " << domain;
  }

  for (const auto& record : records) {
    uint32_t height;
    Crypto::Hash hash = NULL_HASH;
    std::stringstream ss;
    int del = record.find_first_of(':');
    std::string height_str = record.substr(0, del), hash_str = record.substr(del + 1, 64);
    ss.str(height_str);
    ss >> height;
    char c;
    if ((ss.fail() || ss.get(c)) || !Common::podFromHex(hash_str, hash)) {
      logger(Logging::INFO) << "Failed to parse DNS checkpoint record: " << record;
      continue;
    }

    if (!(0 == m_points.count(height))) {
      logger(DEBUGGING) << "Checkpoint already exists for height: " << height << ". Ignoring DNS checkpoint.";
    } else {
      add_checkpoint(height, hash_str);
    }
  }

  return true;
}
#endif
}
