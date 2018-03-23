// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <signal.h>
#include <string>
#include <iostream>

#include "service/posix_fork.h"

namespace posix {

void fork(const std::string &pidfile){
  std::ofstream pidofs;
  if (!pidfile.empty()){
    int oldpid;
    std::ifstream pidrifs;
    pidrifs.open(pidfile, std::fstream::in);
    if (!pidrifs.fail()){
      if (pidrifs >> oldpid && oldpid > 1 && kill(oldpid, 0) == 0){
        std::cout << "PID file already exists and the PID therein is valid" << std::endl;
      }
      pidrifs.close();
    }
    pidofs.open(pidfile, std::fstream::out | std::fstream::trunc);
    if (pidofs.fail()){
      std::cout << "Failed to open specified PID file for writing" << std::endl;
    }
  }
  if (pid_t pid = ::fork()){
    if (pid > 0){
      pidofs.close();
      exit(0);
      } else {
      std::cout << "Fork failed" << std::endl;
    }
  }
  if (!pidofs.fail()){
    int pid = ::getpid();
    pidofs << pid << std::endl;
    pidofs.close();
  }
  close(0);
  close(1);
  if (open("/dev/null", O_RDONLY) < 0){
    std::cout << "Unable to open /dev/null" << std::endl;
  }
}

}