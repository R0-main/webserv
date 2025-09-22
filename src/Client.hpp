/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 15:01:32 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 14:26:06 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "utils/String.hpp"
#include "http/HTTPRequestParser.hpp"
#include "http/HTTPResponse.hpp"
#include "Server.hpp"
#include <map>

class Client
{
	private:

		bool			_disconnected;

		String			_read_socket(void) throw()
		{
			char buffer[1024];

			int len =  recv(socket, &buffer, sizeof(buffer), 0);
			if (len <= 0)
			{
				this->_logger.error("Fail to read from socket !");
				this->disconnect();
				return String("");
			}

			return String(buffer);
		}

	public:
		bool			_proccessed_headers;

		Logger			_logger;

		HTTPRequestParser _request_parser;
		fd_t			 socket;

		Client(fd_t socket) : socket(socket), _logger(String("Client ") + String::fromInt(socket), Color(0xfc0ae0)), _disconnected(false), _proccessed_headers(false)
		{
			_logger.log("created !");
		}

		~Client()
		{
			this->disconnect();
			_logger.log("destroyed !");
		}

		void parse_request()
		{
			String buffer = this->_read_socket();

			if (buffer.length() == 0)
				return ;

			this->_request_parser.parse(buffer);

			if (this->_request_parser.fail())
			{
				this->_logger.error("Fail to parse Request !");
				this->disconnect();
				return ;
			}
			// this->_request_parser.clear();
		}

		void send(HTTPResponse &response)
		{
			if (this->isDisconnected())
				return ;
			response.send(this->socket);
			this->disconnect();
		}

		void disconnect(void) throw()
		{
			if (!_disconnected)
			{
				close(socket);
				_disconnected = true;
				_logger.log("disconnected !");
			}
		}

		bool isDisconnected(void) const throw()
		{
			return _disconnected;
		}
};

