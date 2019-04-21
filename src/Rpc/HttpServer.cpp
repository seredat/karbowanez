// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2016 XDN developers
// Copyright (c) 2016-2018 Karbowanec developers
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

#include "HttpServer.h"
#include <thread>
#include <string.h>
#include <streambuf>
#include <array>
#include <boost/scope_exit.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <HTTP/HttpParser.h>
#include <System/InterruptedException.h>
#include <System/TcpStream.h>
#include <System/SocketStream.h>
#include <System/Ipv4Address.h>

using boost::asio::ip::tcp;
using namespace Logging;


namespace {
	std::string base64Encode(const std::string& data) {
		static const char* encodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		const size_t resultSize = 4 * ((data.size() + 2) / 3);
		std::string result;
		result.reserve(resultSize);

		for (size_t i = 0; i < data.size(); i += 3) {
			size_t a = static_cast<size_t>(data[i]);
			size_t b = i + 1 < data.size() ? static_cast<size_t>(data[i + 1]) : 0;
			size_t c = i + 2 < data.size() ? static_cast<size_t>(data[i + 2]) : 0;

			result.push_back(encodingTable[a >> 2]);
			result.push_back(encodingTable[((a & 0x3) << 4) | (b >> 4)]);
			if (i + 1 < data.size()) {
				result.push_back(encodingTable[((b & 0xF) << 2) | (c >> 6)]);
				if (i + 2 < data.size()) {
					result.push_back(encodingTable[c & 0x3F]);
				}
			}
		}

		while (result.size() != resultSize) {
			result.push_back('=');
		}

		return result;
	}

	void fillUnauthorizedResponse(CryptoNote::HttpResponse& response) {
		response.setStatus(CryptoNote::HttpResponse::STATUS_401);
		response.addHeader("WWW-Authenticate", "Basic realm=\"RPC\"");
		response.addHeader("Content-Type", "text/plain");
		response.setBody("Authorization required");
	}

}

namespace CryptoNote {

HttpServer::HttpServer(System::Dispatcher& dispatcher, Logging::ILogger& log)
  : m_dispatcher(dispatcher), workingContextGroup(dispatcher), logger(log, "HttpServer") {
  this->server_ssl_start = false;
  this->chain_file = "";
  this->key_file = "";
  this->dh_file = "";
  this->server_ssl_port = 0;
  this->server_ssl_clients = 0;
}

void HttpServer::setCerts(const std::string& chain_file, const std::string& key_file, const std::string& dh_file){
  this->chain_file = chain_file;
  this->key_file = key_file;
  this->dh_file = dh_file;
}

void HttpServer::start(const std::string& address, uint16_t port, uint16_t port_ssl,
                       bool server_ssl_enable, const std::string& user, const std::string& password) {
  m_listener = System::TcpListener(m_dispatcher, System::Ipv4Address(address), port);
  workingContextGroup.spawn(std::bind(&HttpServer::acceptLoop, this));

  this->server_ssl_port = port_ssl;
  this->server_ssl_start = server_ssl_enable;
  
  		if (!user.empty() || !password.empty()) {
			m_credentials = base64Encode(user + ":" + password);
		}

  if (!this->chain_file.empty() && !this->key_file.empty() && !this->dh_file.empty() &&
      this->server_ssl_port != 0 && this->server_ssl_start){
    std::thread t(&HttpServer::server_ssl, this);
    t.detach();
    //this->server_ssl();
  }
}

void HttpServer::stop() {
  workingContextGroup.interrupt();
  workingContextGroup.wait();
}

void HttpServer::server(){
  bool srv_restart = false;
  try {
    tcp::acceptor accept(this->io_service, tcp::endpoint(tcp::v4(), 32448));
    srv_restart = true;
    for (;;){
      tcp::iostream stream;
      boost::system::error_code ec;
      accept.accept(*stream.rdbuf(), ec);
      if (!ec){
        HttpParser parser;
        HttpRequest req;
        HttpResponse resp;
        resp.addHeader("Access-Control-Allow-Origin", "*");
        resp.addHeader("content-type", "application/json");

        parser.receiveRequest(stream, req);

        if (authenticate(req)) {
          processRequest(req, resp);
        } else {
          logger(WARNING) << "Authorization required" << std::endl;
        }
        stream << resp;
        stream.flush();
      }
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  if (srv_restart){
    std::thread t(&HttpServer::server, this);
    t.detach();
  }
}

void HttpServer::do_session_ssl(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ctx){
  const size_t i_buff_size = 2048;
  boost::system::error_code ec;
  boost::asio::ssl::stream<tcp::socket&> stream(socket, ctx);
  stream.handshake(boost::asio::ssl::stream_base::server, ec);
  this->server_ssl_clients++;

  try {
    if (!ec){
      char i_buff[i_buff_size];
      size_t length = stream.read_some(boost::asio::buffer(i_buff, i_buff_size), ec);
      if (length > 0 && length < i_buff_size){
        System::SocketStreambuf streambuf((char *) i_buff, length);
        std::iostream io_stream(&streambuf);

        HttpParser parser;
        HttpRequest req;
        HttpResponse resp;
        resp.addHeader("Access-Control-Allow-Origin", "*");
        resp.addHeader("content-type", "application/json");

        parser.receiveRequest(io_stream, req);

        if (authenticate(req)) {
          processRequest(req, resp);
        } else {
          logger(WARNING) << "Authorization required" << std::endl;
        }
        io_stream << resp;
        io_stream.flush();

        stream.write_some(boost::asio::buffer(streambuf.o_buff), ec);
      } else {
        logger(WARNING) << "Unable to process request (SSL server)" << std::endl;
      }
    }
  } catch (std::exception& e) {
    logger(ERROR, BRIGHT_RED) << "SSL server error: " << e.what() << std::endl;
  }
  this->server_ssl_clients--;
}

void HttpServer::server_ssl(){
  bool srv_loop = false;
  try {
    tcp::acceptor accept(this->io_service, tcp::endpoint(tcp::v4(), this->server_ssl_port));

    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    ctx.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2);
    ctx.use_certificate_chain_file(this->chain_file);
    ctx.use_private_key_file(this->key_file, boost::asio::ssl::context::pem);
    ctx.use_tmp_dh_file(this->dh_file);

    srv_loop = true;
    while(srv_loop){
      tcp::socket sock(this->io_service);
      accept.accept(sock);
      std::thread t(std::bind(&HttpServer::do_session_ssl, this, std::move(sock), std::ref(ctx)));
      t.detach();
    }
  } catch (std::exception& e) {
    logger(ERROR, BRIGHT_RED) << "SSL server error: " << e.what() << std::endl;
  }
  if (srv_loop && this->server_ssl_start){
    std::thread t(&HttpServer::server_ssl, this);
    t.detach();
  }
}

void HttpServer::acceptLoop() {
  try {
    System::TcpConnection connection; 
    bool accepted = false;

    while (!accepted) {
      try {
        connection = m_listener.accept();
        accepted = true;
      } catch (System::InterruptedException&) {
        throw;
      } catch (std::exception&) {
        // try again
      }
    }

    m_connections.insert(&connection);
    BOOST_SCOPE_EXIT_ALL(this, &connection) { 
      m_connections.erase(&connection); };

	workingContextGroup.spawn(std::bind(&HttpServer::acceptLoop, this));

	//auto addr = connection.getPeerAddressAndPort();
	auto addr = std::pair<System::Ipv4Address, uint16_t>(static_cast<System::Ipv4Address>(0), 0);
	try {
		addr = connection.getPeerAddressAndPort();
	} catch (std::runtime_error&) {
		logger(WARNING) << "Could not get IP of connection";
	}

    logger(DEBUGGING) << "Incoming connection from " << addr.first.toDottedDecimal() << ":" << addr.second;

    System::TcpStreambuf streambuf(connection);
    std::iostream stream(&streambuf);
    HttpParser parser;

    for (;;) {
      HttpRequest req;
      HttpResponse resp;
	  resp.addHeader("Access-Control-Allow-Origin", "*");
	  resp.addHeader("content-type", "application/json");
	
      parser.receiveRequest(stream, req);
				if (authenticate(req)) {
					processRequest(req, resp);
				}
				else {
					logger(WARNING) << "Authorization required " << addr.first.toDottedDecimal() << ":" << addr.second;
					fillUnauthorizedResponse(resp);
				}

      stream << resp;
      stream.flush();

      if (stream.peek() == std::iostream::traits_type::eof()) {
        break;
      }
    }

    logger(DEBUGGING) << "Closing connection from " << addr.first.toDottedDecimal() << ":" << addr.second << " total=" << m_connections.size();

  } catch (System::InterruptedException&) {
  } catch (std::exception& e) {
    logger(DEBUGGING) << "Connection error: " << e.what();
  }
}

bool HttpServer::authenticate(const HttpRequest& request) const {
	if (!m_credentials.empty()) {
		auto headerIt = request.getHeaders().find("authorization");
		if (headerIt == request.getHeaders().end()) {
			return false;
		}

		if (headerIt->second.substr(0, 6) != "Basic ") {
			return false;
		}

		if (headerIt->second.substr(6) != m_credentials) {
			return false;
		}
	}

	return true;
}

size_t HttpServer::get_connections_count() const {
	return m_connections.size() + this->server_ssl_clients;
}

}
