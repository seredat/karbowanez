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
// Copyright (c) 2014-2016 XDN developers
//
// You should have received a copy of the GNU Lesser General Public License
// along with Karbo.  If not, see <http://www.gnu.org/licenses/>.

#pragma once 

#include <unordered_set>
#include <string.h>

#include <HTTP/HttpRequest.h>
#include <HTTP/HttpResponse.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <System/ContextGroup.h>
#include <System/Dispatcher.h>
#include <System/TcpListener.h>
#include <System/TcpConnection.h>
#include <System/Event.h>

#include <Logging/LoggerRef.h>


namespace CryptoNote {

class HttpServer {

public:

  HttpServer(System::Dispatcher& dispatcher, Logging::ILogger& log);

  void setCerts(const std::string& chain_file, const std::string& key_file, const std::string& dh_file);
  void start(const std::string& address, uint16_t port, uint16_t port_ssl = 0,
             bool server_ssl_enable = false, const std::string& user = "", const std::string& password = "");
  void stop();

  virtual void processRequest(const HttpRequest& request, HttpResponse& response) = 0;
  virtual size_t get_connections_count() const;

  uint16_t server_ssl_port;
  bool server_ssl_start;

protected:

  System::Dispatcher& m_dispatcher;

private:
  boost::asio::io_service io_service;
  std::string chain_file;
  std::string key_file;
  std::string dh_file;
  void server();
  void server_ssl();
  void do_session_ssl(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ctx);
  unsigned int server_ssl_clients;
  void acceptLoop();
  void connectionHandler(System::TcpConnection&& conn);
  bool authenticate(const HttpRequest& request) const;

  System::ContextGroup workingContextGroup;
  Logging::LoggerRef logger;
  System::TcpListener m_listener;
  std::unordered_set<System::TcpConnection*> m_connections;
  std::string m_credentials;
};

}
