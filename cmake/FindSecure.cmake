
# Copyright (c) 2016-2019, The Karbo developers
#
# This file is part of Karbo.
#
# This is custom implementation of cmake module to OpenSSL library.
# The mechanism for obtaining a version is based on the original solution.
#
# Karbo is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Karbo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Karbo.  If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 2.8.12)

set(SECURE_VERSION "0.0.0")
set(SECURE_VERSION_MAJOR "0")
set(SECURE_VERSION_MINOR "0")
set(SECURE_VERSION_PATCH "0")

if (UNIX)
  set(_SECURE_LIB_SSL_NAME_STATIC "libssl.a")
  set(_SECURE_LIB_SSL_NAME_SHARED "libssl.so")
  set(_SECURE_LIB_CRYPTO_NAME_STATIC "libcrypto.a")
  set(_SECURE_LIB_CRYPTO_NAME_SHARED "libcrypto.so")
else()
  set(_SECURE_LIB_SSL_NAME "ssleay32.lib")
  set(_SECURE_LIB_CRYPTO_NAME "libeay32.lib")
endif()

if (UNIX AND STATIC)
  set(_SECURE_LIB_SSL_NAME ${_SECURE_LIB_SSL_NAME_STATIC})
  set(_SECURE_LIB_CRYPTO_NAME ${_SECURE_LIB_CRYPTO_NAME_STATIC})
elseif (UNIX AND NOT STATIC)
  set(_SECURE_LIB_SSL_NAME ${_SECURE_LIB_SSL_NAME_SHARED})
  set(_SECURE_LIB_CRYPTO_NAME ${_SECURE_LIB_CRYPTO_NAME_SHARED})
endif()

if (SECURE_INCLUDE_DIRS AND SECURE_LIBRARY_DIRS)
  find_library(SECURE_SSL_LIBRARY ${_SECURE_LIB_SSL_NAME} ${SECURE_LIBRARY_DIRS} NO_DEFAULT_PATH)
  find_library(SECURE_CRYPTO_LIBRARY ${_SECURE_LIB_CRYPTO_NAME} ${SECURE_LIBRARY_DIRS} NO_DEFAULT_PATH)
  find_path(SECURE_INCLUDE_DIR "openssl/ssl.h" ${SECURE_INCLUDE_DIRS} NO_DEFAULT_PATH)
else()
  find_library(SECURE_SSL_LIBRARY ${_SECURE_LIB_SSL_NAME})
  find_library(SECURE_CRYPTO_LIBRARY ${_SECURE_LIB_CRYPTO_NAME})
  find_path(SECURE_INCLUDE_DIR "openssl/ssl.h")
endif()

function(from_hex HEX DEC)
  string(TOUPPER "${HEX}" HEX)
  set(_res 0)
  string(LENGTH "${HEX}" _strlen)
  while (_strlen GREATER 0)
    math(EXPR _res "${_res} * 16")
    string(SUBSTRING "${HEX}" 0 1 NIBBLE)
    string(SUBSTRING "${HEX}" 1 -1 HEX)
    if (NIBBLE STREQUAL "A")
      math(EXPR _res "${_res} + 10")
    elseif (NIBBLE STREQUAL "B")
      math(EXPR _res "${_res} + 11")
    elseif (NIBBLE STREQUAL "C")
      math(EXPR _res "${_res} + 12")
    elseif (NIBBLE STREQUAL "D")
      math(EXPR _res "${_res} + 13")
    elseif (NIBBLE STREQUAL "E")
      math(EXPR _res "${_res} + 14")
    elseif (NIBBLE STREQUAL "F")
      math(EXPR _res "${_res} + 15")
    else()
      math(EXPR _res "${_res} + ${NIBBLE}")
    endif()
    string(LENGTH "${HEX}" _strlen)
  endwhile()
  set(${DEC} ${_res} PARENT_SCOPE)
endfunction()

if (SECURE_INCLUDE_DIR AND SECURE_SSL_LIBRARY AND SECURE_CRYPTO_LIBRARY)
  if (EXISTS "${SECURE_INCLUDE_DIR}/openssl/opensslv.h")
    file(READ "${SECURE_INCLUDE_DIR}/openssl/opensslv.h" _OPENSSLV_DATA)
    string(REGEX MATCH "#[\t| ]*define[\t| ]+OPENSSL_VERSION_NUMBER[\t| ]+0x[0-9a-fA-F]+"
           _SECURE_VERSION_DATA "${_OPENSSLV_DATA}")
    if ("${_SECURE_VERSION_DATA}" MATCHES
        "([0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F])")
      set(_SECURE_VERSION_MAJOR ${CMAKE_MATCH_1})
      set(_SECURE_VERSION_MINOR ${CMAKE_MATCH_2})
      set(_SECURE_VERSION_FIX ${CMAKE_MATCH_3})
      set(_SECURE_VERSION_PATCH ${CMAKE_MATCH_4})
      from_hex("${_SECURE_VERSION_MINOR}" _SECURE_VERSION_MINOR)
      from_hex("${_SECURE_VERSION_FIX}" _SECURE_VERSION_FIX)
      if (NOT _SECURE_VERSION_PATCH STREQUAL "00")
        from_hex("${_SECURE_VERSION_PATCH}" _tmp)
        math(EXPR _SECURE_VERSION_PATCH_ASCII "${_tmp} + 96")
        unset(_tmp)
        string(ASCII "${_SECURE_VERSION_PATCH_ASCII}" _SECURE_VERSION_PATCH_STRING)
      endif()
      set(SECURE_VERSION_MAJOR ${_SECURE_VERSION_MAJOR})
      set(SECURE_VERSION_MINOR ${_SECURE_VERSION_MINOR})
      set(SECURE_VERSION_PATCH "${_SECURE_VERSION_FIX}${_SECURE_VERSION_PATCH_STRING}")
      set(SECURE_VERSION "${SECURE_VERSION_MAJOR}.${SECURE_VERSION_MINOR}.${SECURE_VERSION_PATCH}")
      message(STATUS "OpenSSL version: ${SECURE_VERSION}")
      if (Secure_FIND_VERSION)
        if (${SECURE_VERSION} VERSION_EQUAL ${Secure_FIND_VERSION} OR
            ${SECURE_VERSION} VERSION_GREATER ${Secure_FIND_VERSION})
          set(SECURE_FOUND TRUE)
        else()
          message(WARNING "Installed version OpenSSL is too old and will not be used")
          set(SECURE_FOUND FALSE)
        endif()
      else()
        set(SECURE_FOUND TRUE)
      endif()
    endif()
  endif()
  set(SECURE_LIBRARIES ${SECURE_SSL_LIBRARY} ${SECURE_CRYPTO_LIBRARY})
else()
  message(STATUS "Can`t find OpenSSL installed")
endif()

