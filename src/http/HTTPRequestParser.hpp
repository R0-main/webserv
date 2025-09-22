/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:14:12 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/11 09:21:17 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "../utils/String.hpp"
#include "../utils/Logger.hpp"
#include "HTTPRequest.hpp"
#include <map>

class HTTPRequestParser
{
	private:
		/* data */

		Logger _logger;

		String _buffer;
		size_t _buffer_offset;
		size_t _header_len;

		bool _first_line_parsed;
		bool _header_parsed;
		bool _body_parsed;

		bool _failed;

		HTTPRequest _request;

		void _parse_headers(void)
		{
			size_t end = _buffer.find(CRLF CRLF, 0);

			while (_buffer.find(CRLF, this->_buffer_offset) != std::string::npos)
			{
				if (end == this->_buffer_offset - 2)
				{
					this->_buffer_offset += 2;
					_header_parsed = true;
					_header_len = this->_buffer_offset;
					break;
				}
				if (_buffer.find_first_of(" \t\f\v", this->_buffer_offset) < _buffer.find(":", this->_buffer_offset))
				{
					_failed = true;
					return ;
				}
				if (_buffer.find_first_of(HEADER_KEY_CHARS, this->_buffer_offset) < _buffer.find(":", this->_buffer_offset))
				{
					_failed = true;
					return ;
				}

				String key = _buffer.substr(this->_buffer_offset, _buffer.find(":", this->_buffer_offset) - this->_buffer_offset);
				String value = _buffer.substr(_buffer.find(":", this->_buffer_offset) + 1, _buffer.find(CRLF, this->_buffer_offset) - _buffer.find(":", this->_buffer_offset));

				value.trim();
				key.lower();

				_request.setHeader(key, value);

				this->_buffer_offset = _buffer.find(CRLF, this->_buffer_offset) + String(CRLF).length();
			}

			if (this->_buffer_offset >= end)
			{
				_header_parsed = true;
				_header_len = this->_buffer_offset;
			}
		}

		void _parse_first_line(void)
		{
			size_t space_count = _buffer.count(' ', 0, _buffer.find(CRLF, 0));
			if (space_count < 2 || space_count > 2)
			{
				this->_logger.error("Invalid Request Line Spaces Count");
				_failed = true;
				return ;
			}

			if (_buffer.find_first_of("	\v", 0) < _buffer.find(CRLF, 0))
			{
				this->_logger.error("Invalid Request Line White Spaces");
				_failed = true;
				return ;
			}

			size_t first_space = _buffer.find(" ");
			if (first_space == std::string::npos)
			{
				this->_logger.error("Invalid Request Line First Space");
				_failed = true;
				return ;
			}

			size_t second_space = _buffer.find(" ", first_space + 1);
			if (second_space == std::string::npos)
			{
				this->_logger.error("Invalid Request Line Second Space");
				_failed = true;
				return ;
			}

			String method = _buffer.substr(0, first_space);
			_request.method = method;

			String request_target = _buffer.substr(first_space + 1, second_space - first_space - 1);
			_request.request_target = request_target;

			String http_version = _buffer.substr(second_space + 1, _buffer.find(CRLF) - second_space - 1);
			_request.http_version = http_version;

			this->_buffer_offset = _buffer.find(CRLF) + String(CRLF).length();
			_first_line_parsed = true;
		}

	public:

		HTTPRequestParser() : _failed(false), _logger("HTTPRequestParser", Color(0xc6d40b)), _first_line_parsed(false) ,_buffer_offset(0) ,_header_parsed(false)
		{
			_logger.info("Start Parsing ...");
		}

		~HTTPRequestParser()
		{

		}

		void clear() throw()
		{
			_buffer.clear();
			_buffer_offset = 0;
			_failed = false;
			_first_line_parsed = false;
			_header_parsed = false;
			_request = HTTPRequest();
		}

		void parse(String &buffer) throw()
		{
			if (fail())
				return ;

			_buffer += buffer;

			if (!_header_parsed)
			{
				if (_buffer.find(CRLF, this->_buffer_offset) == std::string::npos)
					return ;
				if (!_first_line_parsed)
				{
					this->_parse_first_line();
					if (fail())
						return ;
				}
				this->_parse_headers();
			}

			if (_header_parsed)
			{
				if (this->_request.hasHeader("content-length"))
				{
					try
					{
						int body_len = this->_request.getHeader("content-length").toInt();
						if (body_len < 0 || body_len > 10000000)
						{
							this->_failed = true;
							return;
						}

						int bytes_needed = body_len - this->_request.body.length();
						if (bytes_needed > 0)
						{
							int available_in_buffer = _buffer.length() - this->_buffer_offset;

							if (available_in_buffer > 0)
							{
								int bytes_to_take = (bytes_needed < available_in_buffer) ? bytes_needed : available_in_buffer;
								this->_request.body += _buffer.substr(this->_buffer_offset, bytes_to_take);
								this->_buffer_offset += bytes_to_take;
							}
						}

						if (this->_request.body.length() >= body_len)
							_body_parsed = true;
					}
					catch(const std::exception& e)
					{
						std::cerr << e.what() << '\n';
						this->_failed = true;
					}
				}
			}


		}

		HTTPRequest &get() throw()
		{
			return _request;
		}

		bool isHeaderParsed(void) const throw()
		{
			return _header_parsed;
		}

		bool isBodyParsed(void) const throw()
		{
			return _body_parsed;
		}

		bool fail(void) const throw()
		{
			return _failed;
		}
};

