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

#include "RpcServerConfig.h"
#include "Common/CommandLine.h"
#include "CryptoNoteConfig.h"
#include "android.h"

namespace CryptoNote {

  namespace {

    const std::string DEFAULT_RPC_IP = "127.0.0.1";
    const uint16_t DEFAULT_RPC_PORT = RPC_DEFAULT_PORT;

    const command_line::arg_descriptor<std::string> arg_rpc_bind_ip     = { "rpc-bind-ip", "", DEFAULT_RPC_IP };
    const command_line::arg_descriptor<uint16_t>    arg_rpc_bind_port   = { "rpc-bind-port", "", DEFAULT_RPC_PORT };
    const command_line::arg_descriptor<bool>        arg_restricted_rpc  = { "restricted-rpc", "Restrict RPC to view only commands to prevent abuse", false };
    const command_line::arg_descriptor<std::string> arg_enable_cors     = { "enable-cors", "Adds header 'Access-Control-Allow-Origin' to the daemon's RPC responses. Uses the value as domain. Use * for all", "" };
    const command_line::arg_descriptor<std::string> arg_set_contact     = { "contact", "Sets node admin contact", "" };
    const command_line::arg_descriptor<std::string> arg_set_fee_address = { "fee-address", "Sets fee address for light wallets.", "" };
    const command_line::arg_descriptor<std::string> arg_set_fee_amount  = { "fee-amount", "Sets flat rate fee for light wallets.", "" };
    const command_line::arg_descriptor<std::string> arg_set_view_key    = { "view-key", "Sets private view key to check for node's fee.", "" };
  }


  RpcServerConfig::RpcServerConfig() : 
    bindIp(DEFAULT_RPC_IP),
    bindPort(DEFAULT_RPC_PORT),
    enableCors(""),
    restrictedRpc(false),
    contactInfo(""),
    nodeFeeAddress(""),
    nodeFeeAmountStr(""),
    nodeFeeViewKey("") {
  }

  std::string RpcServerConfig::getBindAddress() const {
    return bindIp + ":" + std::to_string(bindPort);
  }
  
  void RpcServerConfig::initOptions(boost::program_options::options_description& desc) {
    command_line::add_arg(desc, arg_rpc_bind_ip);
    command_line::add_arg(desc, arg_rpc_bind_port);
    command_line::add_arg(desc, arg_restricted_rpc);
    command_line::add_arg(desc, arg_set_contact);
    command_line::add_arg(desc, arg_enable_cors);
    command_line::add_arg(desc, arg_set_fee_address);
    command_line::add_arg(desc, arg_set_fee_amount);
    command_line::add_arg(desc, arg_set_view_key);
  }

  void RpcServerConfig::init(const boost::program_options::variables_map& vm)  {
    bindIp = command_line::get_arg(vm, arg_rpc_bind_ip);
    bindPort = command_line::get_arg(vm, arg_rpc_bind_port);
    enableCors = command_line::get_arg(vm, arg_enable_cors);
    restrictedRpc = command_line::get_arg(vm, arg_restricted_rpc);
    contactInfo = command_line::get_arg(vm, arg_set_contact);
    nodeFeeAddress = command_line::get_arg(vm, arg_set_fee_address);
    nodeFeeAmountStr = command_line::get_arg(vm, arg_set_fee_amount);
    nodeFeeViewKey = command_line::get_arg(vm, arg_set_view_key);
  }

}
