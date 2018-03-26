//
// Copyright (c) 2016-2018, Karbo developers
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <boost/filesystem.hpp>

#include "service/service.h"

namespace Service {

service::service(std::string srv_name){
  memset(this->srv_name, 0, SRV_SIZE);
  memset(this->pidfile, 0, PIDFILE_SIZE);
  boost::filesystem::path temp = boost::filesystem::temp_directory_path();
  std::string pidfile_default = temp.native() + "/" + srv_name + ".pid";
  this->status = false;
  strncpy(this->pidfile, pidfile_default.c_str(), sizeof(this->pidfile));
  strncpy(this->srv_name, srv_name.c_str(), sizeof(this->srv_name));
}

service::~service(){
}

bool service::createPID(int pid){
  bool result = false;
  char pidbuff[4];
  FILE *pFile;
  pFile = fopen(this->pidfile, "w");
  if (pFile != NULL){
    memset(pidbuff, 0, 4);
    pidbuff[0] = pid & 0x000000FF;
    pidbuff[1] = (pid & 0x0000FF00) >> 8;
    pidbuff[2] = (pid & 0x00FF0000) >> 16;
    pidbuff[3] = (pid & 0xFF000000) >> 24;
    if (fwrite (pidbuff, sizeof(char), 4, pFile) == 4){
      result = true;
    }
    fclose (pFile);
  }
  return result;
}

int service::readPID(){
  int result = -1;
  char pidbuff[5];
  FILE *pFile;
  pFile = fopen(this->pidfile, "r");
  if (pFile != NULL){
    memset(pidbuff, 0, 5);
    if (fgets(pidbuff, 5, pFile) != NULL){
      result = ((pidbuff[3] & 0x000000FF) << 24) | ((pidbuff[2] & 0x000000FF) << 16) | ((pidbuff[1] & 0x000000FF) << 8) | (pidbuff[0] & 0x000000FF);
    }
    fclose (pFile);
  }
  return result;
}

bool service::deletePID(){
  bool result = false;
  if (remove(this->pidfile) == 0){
    result = true;
  }
  return result;
}

void service::setPid(std::string pidfile){
  strncpy(this->pidfile, pidfile.c_str(), sizeof(this->pidfile));
}

void service::run(){
  pid_t pid;
  this->status = false;
  if (this->readPID() == -1){
    pid = fork();
    if (pid == -1){
      std::cout << "Error creating child process!" << std::endl;
      exit(1);
      } else if (pid == 0){
      this->status = false;
      setsid();
      close(0);
      close(1);
      close(2);
      } else {
      this->createPID((int) pid);
      this->status = true;
      std::cout << "Process forked successful: " << pid << std::endl;
    }
    } else {
    this->status = false;
    std::cout << "Process already started!" << std::endl;
    exit(1);
  }
}

void service::stop(){
  int pid;
  bool stop_t;
  pid = this->readPID();
  //std::cout << pid << std::endl;
  if (pid != -1){
    if (kill((pid_t) pid, 15) == 0){
      std::cout << "Send stop signal and wait..." << std::endl;
      stop_t = false;
      for (unsigned int n = 0; n < 300; n++){ 
        if (waitpid((pid_t) pid, NULL, 0) == -1){
          this->deletePID();
	  stop_t = true;
          break;
	}
        usleep(1000000);
      }
      if (stop_t){
        std::cout << "Service stoped" << std::endl;
        } else {
        std::cout << "Error stop service!" << std::endl;
      }
    }
    exit(0);
    } else {
    std::cout << "Service not started!" << std::endl;
    exit(0);
  }
}

bool service::getStatus(){
  return this->status;
}

}
