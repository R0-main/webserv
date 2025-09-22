/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Color.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 12:15:48 by rguigneb          #+#    #+#             */
/*   Updated: 2025/08/26 12:16:16 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>

class Color
{
	private:
		std::string _color;

		std::string _int_to_string(long value) {
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

	public:
		Color(unsigned char r, unsigned char g, unsigned char b)
		{
			this->_color = "\033[38;2;" + this->_int_to_string(r) + ";" + this->_int_to_string(g) + ";" + this->_int_to_string(b) + "m";
		}

		Color(unsigned int hex)
		{
			unsigned char r = (hex >> 16) & 0xFF;  // Red: bits 16-23
			unsigned char g = (hex >> 8) & 0xFF;   // Green: bits 8-15
			unsigned char b = hex & 0xFF;          // Blue: bits 0-7

			this->_color = "\033[38;2;" + this->_int_to_string(r) + ";" + this->_int_to_string(g) + ";" + this->_int_to_string(b) + "m";
		}

		std::string get() const {
			return this->_color;
		}
};

namespace Colors {
    static const Color RED = Color(255, 0, 0);
    static const Color GREEN = Color(0, 255, 0);
    static const Color BLUE = Color(0, 0, 255);
    static const Color YELLOW = Color(255, 255, 0);
    static const Color MAGENTA = Color(255, 0, 255);
    static const Color CYAN = Color(0, 255, 255);
    static const Color WHITE = Color(255, 255, 255);
    static const Color BLACK = Color(0, 0, 0);
    static const Color ORANGE = Color(0xFF5733);
    static const Color PURPLE = Color(0x800080);
    static const Color GRAY = Color(128, 128, 128);
}

namespace TextFormat {
    static const std::string BOLD = "\033[1m";
    static const std::string ITALIC = "\033[3m";
    static const std::string UNDERLINE = "\033[4m";
    static const std::string DIM = "\033[2m";
    static const std::string STRIKETHROUGH = "\033[9m";
    static const std::string RESET = "\033[0m";
}

std::ostream &operator<<(std::ostream &oss, Color color)
{
	oss << color.get();
	return oss;
}

