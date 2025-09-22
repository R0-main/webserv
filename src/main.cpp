/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 11:44:59 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 14:27:21 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <csignal>

#include "Server.hpp"

int main(void)
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

	Server server(8081);

	return 0;
}
