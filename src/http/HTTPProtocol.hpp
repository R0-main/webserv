/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPProtocol.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:14:12 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 15:54:46 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "../utils/String.hpp"
#include <map>


#define CR "\r"
#define LF "\n"
#define CRLF CR LF

#define HEADER_KEY_CHARS "\v()<>@,;\\\"/[]?={}"

class HTTPProtocol
{
	private:
		/* data */
	public:
		typedef enum
		{
			GET,
			POST,
			UPDATE,
			DELETE
		} request_type_e;

		typedef std::map<String, String> header_t;

		header_t headers;
		String http_version;

		String body;
		size_t max_body_len;

		HTTPProtocol() : http_version("HTTP/1.1"), max_body_len(0), body(""), headers()
		{

		}

		~HTTPProtocol()
		{

		}

		void setHeader(const String &name, const String &value)
		{
			headers[name] = value;
		}

		bool hasHeader(String name)
		{
			return headers.find(name) != headers.end();
		}

		String getHeader(String name)
		{
			if (!this->hasHeader(name))
				return "";

			return headers[name];
		}

		String generate_headers()
		{
			String formatted_headers;


			for (header_t::iterator i = headers.begin(); i != headers.end(); i++)
			{
				String key = (*i).first;
				String &value = (*i).second;
				formatted_headers += key + ": " + value + CRLF;
			}

			formatted_headers += CRLF;
			return formatted_headers;
		}
};

