// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
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

#include "PathTools.h"
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <libgen.h>
#endif

namespace {

const char GENERIC_PATH_SEPARATOR = '/';

#ifdef _WIN32
const char NATIVE_PATH_SEPARATOR = '\\';
#else
const char NATIVE_PATH_SEPARATOR = '/';
#endif


std::string::size_type findExtensionPosition(const std::string& filename) {
  auto pos = filename.rfind('.');
  
  if (pos != std::string::npos) {
    auto slashPos = filename.rfind(GENERIC_PATH_SEPARATOR);
    if (slashPos != std::string::npos && slashPos > pos) {
      return std::string::npos;
    }
  }

  return pos;
}

} // anonymous namespace

namespace Common {

std::string NativePathToGeneric(const std::string& nativePath) {
  if (GENERIC_PATH_SEPARATOR == NATIVE_PATH_SEPARATOR) {
    return nativePath;
  }
  std::string genericPath(nativePath);
  std::replace(genericPath.begin(), genericPath.end(), NATIVE_PATH_SEPARATOR, GENERIC_PATH_SEPARATOR);
  return genericPath;
}

std::string GetPathDirectory(const std::string& path) {
  auto slashPos = path.rfind(GENERIC_PATH_SEPARATOR);
  if (slashPos == std::string::npos) {
    return std::string();
  }
  return path.substr(0, slashPos);
}

std::string GetPathFilename(const std::string& path) {
  auto slashPos = path.rfind(GENERIC_PATH_SEPARATOR);
  if (slashPos == std::string::npos) {
    return path;
  }
  return path.substr(slashPos + 1);
}

void SplitPath(const std::string& path, std::string& directory, std::string& filename) {
  directory = GetPathDirectory(path);
  filename = GetPathFilename(path);
}

std::string CombinePath(const std::string& path1, const std::string& path2) {
  return path1 + GENERIC_PATH_SEPARATOR + path2;
}

std::string ReplaceExtenstion(const std::string& path, const std::string& extension) {
  return RemoveExtension(path) + extension;
}

std::string GetExtension(const std::string& path) {
  auto pos = findExtensionPosition(path);
  if (pos != std::string::npos) {
    return path.substr(pos);
  }
  return std::string();
}

std::string RemoveExtension(const std::string& filename) { 
  auto pos = findExtensionPosition(filename);

  if (pos == std::string::npos) {
    return filename;
  }

  return filename.substr(0, pos);
}


bool HasParentPath(const std::string& path) {
  return path.find(GENERIC_PATH_SEPARATOR) != std::string::npos;
}

bool IsSysDir(const std::string &path) {
  bool res = false;
  const char *sys_dir[] = {
    "/usr/sbin",
    "/usr/local/sbin",
    "Program Files"
  };
  size_t sys_dir_len = sizeof(sys_dir) / sizeof(sys_dir[0]);
  for (size_t i = 0; i < sys_dir_len; i++) {
    if (path.find(sys_dir[i]) != std::string::npos) {
      res = true;
      break;
    }
  }
  return res;
}

bool GetExePath(std::string &path) {
  const size_t PATH_LEN = 1024;
  bool res = false;
  path.clear();
  char native_path[PATH_LEN + 1];
#ifdef _WIN32
  DWORD result = GetModuleFileNameA(nullptr, native_path, PATH_LEN);
  if (result > 0 && result != PATH_LEN) {
    path = std::string(native_path);
    res = true;
  }
#else
#ifdef __FreeBSD__
  // Must be enable procfs
  ssize_t count = readlink("/proc/curproc/file", native_path, PATH_LEN);
#else
  ssize_t count = readlink("/proc/self/exe", native_path, PATH_LEN);
#endif
  const char *path_base;
  if (count != -1) {
    path_base = dirname(native_path);
    path = std::string(path_base);
    res = true;
  }
#endif
  return res;
}

bool GetFileName(const std::string &path, std::string &fileName) {
  const char path_separator[] = {
    '/',
    '\\'
  };
  const char end_sub = '.';
  const size_t path_len = path.size();
  bool res = false;
  size_t fileNameStart = 0;
  size_t fileNameEnd = path_len - 1;
  fileName.clear();
  size_t i = path_len;
  if (path_len > 0) {
    while (i > 0) {
      i--;
      if (path[i] == end_sub && fileNameEnd == path_len - 1) fileNameEnd = i - 1;
      if (path[i] == path_separator[0] || path[i] == path_separator[1]) {
        fileNameStart = i + 1;
        break;
      }
    }
    fileName = path.substr(fileNameStart, fileNameEnd - fileNameStart + 1);
    if (fileName.size() > 0) res = true;
  }
  return res;
}

}
