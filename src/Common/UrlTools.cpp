
// Copyright (c) 2017-2019, Karbo developers
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
// along with Karbo. If not, see <http://www.gnu.org/licenses/>.


#include <stdint.h>
#include <stdio.h>
#include <string>
#include "CryptoNoteConfig.h"

namespace Common {

const uint16_t HTTP_PORT = 80;
const uint16_t HTTPS_PORT = 443;
const std::string RPC_PATH = "/";

bool parseUrlAddress(const std::string& url, std::string& host, uint16_t& port, std::string& path, bool& ssl) {
  size_t pos = url.find("://");
  size_t hostStart = 0;

  host.clear();
  port = 0;
  path.clear();
  ssl = false;

  bool res = true;

  if (pos != std::string::npos) {
    hostStart = pos + 3;
  }

  size_t hostEnd = url.find(':', hostStart);

  if (hostEnd != std::string::npos) {
    size_t portEnd = url.find('/', hostEnd);
    if (portEnd != std::string::npos) {
      portEnd = portEnd - hostEnd - 1;
    } else {
      portEnd = url.length() - 1;
    }
    //port = Common::fromString<uint16_t>(url.substr(hostEnd + 1, portEnd));
    int port_src = 0;
    if (sscanf(url.substr(hostEnd + 1, portEnd).c_str(), "%d", &port_src) == 1) {
      if (port_src > 0 && port_src <= 0xFFFF) port = (uint16_t) port_src;
    }
  } else {
    hostEnd = url.find('/', hostStart);
    if (hostEnd == std::string::npos) hostEnd = url.length();
    if (url.compare(0, 7, "http://") == 0) {
      port = HTTP_PORT;
    } else if (url.compare(0, 8, "https://") == 0) {
      port = HTTPS_PORT;
    } else {
      port = CryptoNote::RPC_DEFAULT_PORT;
    }
  }

  if (url.compare(0, 8, "https://") == 0) ssl = true;

  host = url.substr(hostStart, hostEnd - hostStart);

  size_t PathStart = url.find('/', hostEnd);
  size_t PathEnd = url.find_last_of('/');
  if (PathStart != std::string::npos && PathEnd != std::string::npos && PathStart != PathEnd) {
    path = url.substr(PathStart, PathEnd - PathStart + 1);
  } else {
    path = RPC_PATH;
  }

  if (host.empty() || port == 0 || path.empty()){
    host.clear();
    port = 0;
    path.clear();
    ssl = false;
    res = false;
  }

  return res;
}

}
