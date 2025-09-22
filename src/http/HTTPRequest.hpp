/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:14:12 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 14:25:14 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "../utils/String.hpp"
#include "HTTPProtocol.hpp"
#include <map>

class HTTPRequest : public HTTPProtocol
{
	private:
		/* data */
	public:

		String method;
		String request_target;

		HTTPRequest() : HTTPProtocol(), method("T"), request_target("T")
		{

		}

		~HTTPRequest()
		{

		}

};

