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

#pragma once

#include <boost/program_options.hpp>

namespace CryptoNote {

class RpcServerConfig {

  public:
    RpcServerConfig();
    static void initOptions(boost::program_options::options_description& desc);
    void init(const boost::program_options::variables_map& options);
    bool isEnableSSL() const;
    uint16_t getBindPort() const;
    uint16_t getBindPortSSL() const;
    std::string getBindIP() const;
    std::string getBindAddress() const;
    std::string getBindAddressSSL() const;
    std::string getDhFile() const;
    std::string getChainFile() const;
    std::string getKeyFile() const;

  private:
    bool m_enable_ssl;
    uint16_t m_bind_port;
    uint16_t m_bind_port_ssl;
    std::string m_bind_ip;
    std::string m_dh_file;
    std::string m_chain_file;
    std::string m_key_file;
};

}
