/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 12:15:44 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 15:12:07 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

class Server;

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <unistd.h>
#include <sys/epoll.h>
#include <poll.h>
#include <map>
#include <map>

#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include "Epoll.hpp"
#include "parsing/YamlParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPRequestParser.hpp"
#include "Client.hpp"

#define END_OF_HTTP "\r\n\r\n"

namespace ServerConfig {
	static const size_t MAX_CONNECTIONS = 2;
}

static volatile sig_atomic_t g_stop = 1;

void on_signal(int sig)
{
    (void)sig;
    g_stop = 0;
}

class Server
{

	private:

		Logger _logger;
		fd_t	socket_fd;

		size_t	_current_connection;

		EpollManager	_epoll_manager;

		struct sockaddr_in						socket_address;
		socklen_t								addrs_len;

		std::map<fd_t, Client *>						_clients;

		YamlParser						config_parser;

		fd_t   _accept_connection()
		{

			fd_t client_socket = accept(socket_fd, (sockaddr*)&this->socket_address, &this->addrs_len);
			if (client_socket == -1)
			{
				_logger.error("accept() fails !");
			}
			return client_socket;
		}

		bool _exist_client(fd_t socket) const throw()
		{
			return this->_clients.find(socket) != this->_clients.end();
		}

		static void _onServerReceiveConnection(int socket_fd, Server *server)
		{
			if (server->_current_connection == ServerConfig::MAX_CONNECTIONS)
			{
				server->_logger.error("Max Connection Reached");
				fd_t client_socket = server->_accept_connection();
				Client *client = new Client(client_socket);

				client->disconnect();
				delete client;
			}
			else
			{
				fd_t client_socket = server->_accept_connection();
				Client *client = new Client(client_socket);
				server->addClient(client_socket, client);
			}
		}

		static void _onClientSendData(int client_fd, Server *server)
		{
			if (!server->_exist_client(client_fd))
				return ;
			Client *client = server->_clients.find(client_fd)->second;

			client->parse_request();

			if (client->isDisconnected() == 1)
			{
				server->removeClient(client);
				return ;
			}

			if (client->_request_parser.isHeaderParsed() && client->_proccessed_headers == false)
			{
				client->_proccessed_headers = true;


				HTTPRequest &request = client->_request_parser.get();

				if (request.method == "GET")
				{

					YamlDict *dict = server->config_parser.get();
					YamlDict *server_dict = static_cast<YamlDict *>(dict->get("server"));
					YamlDict *routing_dict = static_cast<YamlDict *>(server_dict->get("routing"));

					bool found = false;

					for (YamlDict::t_map::const_iterator i = routing_dict->begin(); i != routing_dict->end(); i++)
					{
						if (request.request_target.startsWith((*i).first) == false)
							continue;

						YamlDict *d = static_cast<YamlDict *>((*i).second);
						YamlValue<String> *folder = static_cast<YamlValue<String> *>(d->get("root"));

						String file_path = folder->_value + request.request_target;

						server->_logger.log("Route for " + file_path);
						if (access(file_path.c_str(), F_OK) == -1)
							continue;
						// if (request.request_target != (*i).first)
						// 	continue;


						HTTPResponse response;
						response.http_version = request.http_version;
						response.status = "OK";
						response.status_code = 200;




						client->_logger.log(file_path);
						response.setupFile(file_path, request.request_target);

						client->send(response);
						found = true;
					}

					if (found == false)
					{
						HTTPResponse response;
						response.http_version = request.http_version;
						response.status = "Not Found";
						response.status_code = 404;

						client->send(response);
					}
				}
				else
				{
					client->_logger.error("Unknown Method !");
					client->disconnect();
				}
			}

			if (client->isDisconnected() == 1)
			{
				server->removeClient(client);
				return ;
			}

		}

	public:

		Server(int port) : _logger("Server", Color(0, 244, 10)), _epoll_manager(_logger, 0, ServerConfig::MAX_CONNECTIONS * 2), _current_connection(0), config_parser("./config.yml")
		{
			// socket
			Utils::check(socket_fd = socket(AF_INET, SOCK_STREAM, 0), _logger, "socket() fails");
			int yes = 1;

			setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
			// bind
			bindPort(port);

			Utils::check(listen(socket_fd, ServerConfig::MAX_CONNECTIONS), _logger, "listen() fails");

			_epoll_manager.bindToFd(socket_fd, EPOLLIN, reinterpret_cast<EpollManager::callback_t>(_onServerReceiveConnection));

			loop();
		}

		void bindPort(int port) throw()
		{
			socket_address.sin_port = htons(port);
			socket_address.sin_family = AF_INET;
			socket_address.sin_addr.s_addr = INADDR_ANY;

			addrs_len = sizeof(socket_address);

			Utils::check(bind(socket_fd, (sockaddr*)&socket_address, addrs_len), _logger, "bind() fails");
		}

		void loop() throw()
		{
			while (g_stop)
			{
				_epoll_manager.watchForEvents(this);
			}
		}

		EpollManager &getEpollManager(void) throw()
		{
			return this->_epoll_manager;
		}

		void removeClient(Client *client, bool force_erase = true)
		{
			if (!this->_exist_client(client->socket))
				return ;
			this->_epoll_manager.unbindFd(client->socket, -1);
			if (force_erase)
				this->_clients.erase(client->socket);
			this->_current_connection --;
			client->disconnect();
			delete client;
		}

		void addClient(fd_t client_socket, Client *client)
		{
			if (!this->_exist_client(client_socket))
				this->_clients[client_socket] = client;
			else
			{
				this->_logger.error("Client already exists !");
				return;
			}

			this->_epoll_manager.bindToFd(client_socket, EPOLLIN, reinterpret_cast<EpollManager::callback_t>(_onClientSendData));
			this->_current_connection++;
		}

		~Server()
		{
			for (std::map<fd_t, Client *>::iterator i = _clients.begin(); i != _clients.end(); i++)
			{
				this->removeClient(i->second, false);
			}

			close(socket_fd);
		}
};
