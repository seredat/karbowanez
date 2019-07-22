// Copyright (c) 2016-2019, The Karbo developers
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

#include "Base64.h"

namespace Tools
{
  namespace Base64
  {
    std::string encode(const std::string& data) {
      static const char* encodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
      const size_t resultSize = 4 * ((data.size() + 2) / 3);
      std::string result;
      result.reserve(resultSize);

      for (size_t i = 0; i < data.size(); i += 3) {
        size_t a = static_cast<size_t>(data[i]);
        size_t b = i + 1 < data.size() ? static_cast<size_t>(data[i + 1]) : 0;
        size_t c = i + 2 < data.size() ? static_cast<size_t>(data[i + 2]) : 0;

        result.push_back(encodingTable[a >> 2]);
        result.push_back(encodingTable[((a & 0x3) << 4) | (b >> 4)]);
        if (i + 1 < data.size()) {
          result.push_back(encodingTable[((b & 0xF) << 2) | (c >> 6)]);
          if (i + 2 < data.size()) {
            result.push_back(encodingTable[c & 0x3F]);
          }
        }
      }

      while (result.size() != resultSize) {
        result.push_back('=');
      }

      return result;
    }
  }
}
