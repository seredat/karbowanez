// Copyright (c) 2017-2019 The Karbowanec developers
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

#include <string.h>
#include <streambuf>
#include <array>
#include <iostream>
#include <System/SocketStream.h>


namespace System {

SocketStreambuf::SocketStreambuf(char *data, size_t lenght){
  this->lenght = lenght;
  memcpy(this->readBuf.data(), data, lenght);
  setg(&readBuf.front(), &readBuf.front(), &readBuf.front());
  setp(reinterpret_cast<char*>(&writeBuf.front()), reinterpret_cast<char *>(&writeBuf.front() + writeBuf.max_size()));
  this->read_t = true;
}

SocketStreambuf::~SocketStreambuf(){
}

std::streambuf::int_type SocketStreambuf::underflow(){
  if (gptr() < egptr()){
    return traits_type::to_int_type(*gptr());
  }
  if (read_t){
    read_t = false;
  } else {
    lenght = 0;
  }
  if (this->lenght == 0) {
    return traits_type::eof();
  }
  setg(&readBuf.front(), &readBuf.front(), &readBuf.front() + this->lenght);
  return traits_type::to_int_type(*gptr());
}

int SocketStreambuf::sync(){
  const size_t o_buff_size = 65536;
  size_t count = pptr() - pbase();
  memset(this->o_buff, 0, o_buff_size);
  if (count > 0 && count < o_buff_size){
    strncpy (this->o_buff, (char*) this->writeBuf.data(), count);
  }
  return 0;
}

}

