// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
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

#include "CoreConfig.h"

#include <iostream>

#include "Common/Util.h"
#include "Common/CommandLine.h"
#include "Common/PasswordContainer.h"

namespace {
  Tools::PasswordContainer pwd_container;
}

namespace CryptoNote {

CoreConfig::CoreConfig() {
  configFolder = Tools::getDefaultDataDirectory();
}

namespace {
  const command_line::arg_descriptor<std::string> arg_wallet_host = { "wallet-host", "Specify wallet RPC host", "127.0.0.1", true };
  const command_line::arg_descriptor<uint16_t>    arg_wallet_port = { "wallet-port", "Specify wallet RPC port", 32349, true };
  const command_line::arg_descriptor<size_t>      arg_stake_mixin = { "stake-mixin", "Specify stake transaction mixin", 0, true };
  const command_line::arg_descriptor<std::string> arg_wallet_user = { "wallet-user", "Wallet RPC user. Default: none", "", true };
  const command_line::arg_descriptor<std::string> arg_wallet_pass = { "wallet-password", "Wallet RPC password. Default: none", "", true };
}

void CoreConfig::init(const boost::program_options::variables_map& options) {
  if (options.count(command_line::arg_data_dir.name) != 0 && (!options[command_line::arg_data_dir.name].defaulted() || configFolder == Tools::getDefaultDataDirectory())) {
    configFolder = command_line::get_arg(options, command_line::arg_data_dir);
    configFolderDefaulted = options[command_line::arg_data_dir.name].defaulted();
  }
  if (command_line::has_arg(options, arg_wallet_host)) {
    walletHost = command_line::get_arg(options, arg_wallet_host);
  }

  if (command_line::has_arg(options, arg_wallet_port)) {
    walletPort = command_line::get_arg(options, arg_wallet_port);
  }

  if (command_line::has_arg(options, arg_wallet_user)) {
    walletRpcUser = command_line::get_arg(options, arg_wallet_user);
  }

  if (command_line::has_arg(options, arg_wallet_pass)) {
    walletRpcPassword = command_line::get_arg(options, arg_wallet_pass);
  }

  if (command_line::has_arg(options, arg_wallet_user) && !command_line::has_arg(options, arg_wallet_pass)) {
    std::cout << "Wallet RPC password is not set." << std::endl;
    if (pwd_container.read_password()) {
      walletRpcPassword = pwd_container.password();
    }
  }

  if (command_line::has_arg(options, arg_stake_mixin)) {
    stakeMixin = command_line::get_arg(options, arg_stake_mixin);
  }
}

void CoreConfig::initOptions(boost::program_options::options_description& desc) {
  command_line::add_arg(desc, arg_wallet_host);
  command_line::add_arg(desc, arg_wallet_port);
  command_line::add_arg(desc, arg_stake_mixin);
  command_line::add_arg(desc, arg_wallet_user);
  command_line::add_arg(desc, arg_wallet_pass);
}
} //namespace CryptoNote
