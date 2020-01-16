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
    const uint16_t DEFAULT_RPC_SSL_PORT = RPC_DEFAULT_SSL_PORT;
    const std::string DEFAULT_RPC_CHAIN_FILE = std::string(RPC_DEFAULT_CHAIN_FILE);
    const std::string DEFAULT_RPC_KEY_FILE = std::string(RPC_DEFAULT_KEY_FILE);
    const std::string DEFAULT_RPC_DH_FILE = std::string(RPC_DEFAULT_DH_FILE);

    const command_line::arg_descriptor<std::string> arg_rpc_bind_ip = { "rpc-bind-ip", "Interface for RPC service", DEFAULT_RPC_IP };
    const command_line::arg_descriptor<uint16_t> arg_rpc_bind_port = { "rpc-bind-port", "Port for RPC service", DEFAULT_RPC_PORT };
    const command_line::arg_descriptor<bool> arg_rpc_bind_ssl_enable = { "rpc-bind-ssl-enable", "Enable SSL for RPC service", false };
    const command_line::arg_descriptor<uint16_t> arg_rpc_bind_ssl_port = { "rpc-bind-ssl-port", "SSL port for RPC service", DEFAULT_RPC_SSL_PORT };
    const command_line::arg_descriptor<std::string> arg_chain_file = { "rpc-chain-file", "SSL chain file", DEFAULT_RPC_CHAIN_FILE };
    const command_line::arg_descriptor<std::string> arg_key_file = { "rpc-key-file", "SSL key file", DEFAULT_RPC_KEY_FILE };
    const command_line::arg_descriptor<std::string> arg_dh_file = { "rpc-dh-file", "SSL DH file", DEFAULT_RPC_DH_FILE };
  }

  RpcServerConfig::RpcServerConfig() : m_bind_ip(DEFAULT_RPC_IP),
                                       m_bind_port(DEFAULT_RPC_PORT),
                                       m_bind_port_ssl(RPC_DEFAULT_SSL_PORT) {
  }

  bool RpcServerConfig::isEnableSSL() const { return m_enable_ssl; }
  uint16_t RpcServerConfig::getBindPort() const { return m_bind_port; }
  uint16_t RpcServerConfig::getBindPortSSL() const { return m_bind_port_ssl; }
  std::string RpcServerConfig::getBindIP() const { return m_bind_ip; }
  std::string RpcServerConfig::getDhFile() const { return m_dh_file; }
  std::string RpcServerConfig::getChainFile() const { return m_chain_file; }
  std::string RpcServerConfig::getKeyFile() const { return m_key_file; }
  std::string RpcServerConfig::getBindAddress() const { return m_bind_ip + ":" + std::to_string(m_bind_port); }
  std::string RpcServerConfig::getBindAddressSSL() const { return m_bind_ip + ":" + std::to_string(m_bind_port_ssl); }

  void RpcServerConfig::initOptions(boost::program_options::options_description& desc) {
    command_line::add_arg(desc, arg_rpc_bind_ip);
    command_line::add_arg(desc, arg_rpc_bind_port);
    command_line::add_arg(desc, arg_rpc_bind_ssl_enable);
    command_line::add_arg(desc, arg_rpc_bind_ssl_port);
    command_line::add_arg(desc, arg_chain_file);
    command_line::add_arg(desc, arg_key_file);
    command_line::add_arg(desc, arg_dh_file);
  }

  void RpcServerConfig::init(const boost::program_options::variables_map& vm)  {
    m_bind_ip = command_line::get_arg(vm, arg_rpc_bind_ip);
    m_bind_port = command_line::get_arg(vm, arg_rpc_bind_port);
    m_enable_ssl = command_line::get_arg(vm, arg_rpc_bind_ssl_enable);
    m_bind_port_ssl = command_line::get_arg(vm, arg_rpc_bind_ssl_port);
    m_chain_file = command_line::get_arg(vm, arg_chain_file);
    m_key_file = command_line::get_arg(vm, arg_key_file);
    m_dh_file = command_line::get_arg(vm, arg_dh_file);
  }

}
