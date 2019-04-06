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

#include "MinerConfig.h"

#include "Common/CommandLine.h"

namespace CryptoNote {

namespace {
const command_line::arg_descriptor<std::string> arg_extra_messages  = {"extra-messages-file", "Specify file for extra messages to include into coinbase transactions", "", true};
const command_line::arg_descriptor<std::string> arg_start_mining    = {"start-mining", "Specify wallet address to mining for", "", true};
const command_line::arg_descriptor<uint32_t>    arg_mining_threads  = {"mining-threads", "Specify mining threads count", 0, true};
const command_line::arg_descriptor<std::string> arg_wallet_host     = {"wallet-host", "Specify wallet RPC host", "127.0.0.1", true};
const command_line::arg_descriptor<uint16_t>    arg_wallet_port     = {"wallet-port", "Specify wallet RPC port", 32349, true};
const command_line::arg_descriptor<size_t>      arg_stake_mixin     = {"stake-mixin", "Specify stake transaction mixin", 0, true };
}

MinerConfig::MinerConfig() {
  miningThreads = 0;
}

void MinerConfig::initOptions(boost::program_options::options_description& desc) {
  command_line::add_arg(desc, arg_extra_messages);
  command_line::add_arg(desc, arg_start_mining);
  command_line::add_arg(desc, arg_mining_threads);
  command_line::add_arg(desc, arg_wallet_host);
  command_line::add_arg(desc, arg_wallet_port);
  command_line::add_arg(desc, arg_stake_mixin);
}

void MinerConfig::init(const boost::program_options::variables_map& options) {
  if(command_line::has_arg(options, arg_extra_messages)) {
    extraMessages = command_line::get_arg(options, arg_extra_messages);
  }

  if (command_line::has_arg(options, arg_start_mining)) {
    startMining = command_line::get_arg(options, arg_start_mining);
  }

  if (command_line::has_arg(options, arg_mining_threads)) {
    miningThreads = command_line::get_arg(options, arg_mining_threads);
  }

  if (command_line::has_arg(options, arg_wallet_host)) {
    walletHost = command_line::get_arg(options, arg_wallet_host);
  }

  if (command_line::has_arg(options, arg_wallet_port)) {
    walletPort = command_line::get_arg(options, arg_wallet_port);
  }

  if (command_line::has_arg(options, arg_stake_mixin)) {
    stakeMixin = command_line::get_arg(options, arg_stake_mixin);
  }
}

} //namespace CryptoNote
