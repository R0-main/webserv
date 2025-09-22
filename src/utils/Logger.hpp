/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 12:15:48 by rguigneb          #+#    #+#             */
/*   Updated: 2025/08/26 12:16:16 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "Color.hpp"

// COULD BE IMPROVED LIKE A PRINTF in CPP17

class Logger : public std::ostringstream
{
	private:
		const std::string	_name;
		const Color			_color;
		std::ostream		&_oss;

		static void _print_header(std::string header, Color color, std::ostream &oss) throw()
		{
			oss << TextFormat::BOLD << Colors::GRAY << "[" << color << header << Colors::GRAY << "] ";
		}

		template<typename T>
		std::ostream &_print(std::string header, Color color, std::ostream &oss, T msg, bool new_line = true) const throw()
		{
			if (this->_name.size() > 0)
				Logger::_print_header(this->_name, this->_color, oss);
			Logger::_print_header(header, color, oss);
			oss << TextFormat::RESET << msg << (new_line ? "\n" : "");
			return oss;
		}

	public:
		Logger() : _name(""), _color(Colors::WHITE), _oss(std::cout) {}

		template <typename T>
		Logger(T name, Color color = Colors::WHITE, std::ostream& oss = std::cout) : _name(name), _color(color), _oss(oss) {}

		template <typename T>
		Logger(T name, std::ostream& oss) : _name(name), _color(Colors::WHITE), _oss(oss) {}

		Logger(std::ostream& oss) : _name(""), _color(Colors::WHITE), _oss(oss) {}

		template <typename T>
		std::ostream &operator<<(T value)
		{
			this->_oss << value;
			return this->_oss;
		}

		~Logger() {}

		std::ostream &log(void) const throw()
		{
			return Logger::_print("Log", Color(0, 255, 110), this->_oss, "", false);
		}

		template <typename T>
		std::ostream &log(T msg) const throw()
		{
			return Logger::_print("Log", Color(0, 255, 110), this->_oss, msg);
		}

		template <typename T>
		std::ostream &log(T msg, std::ostream &oss) const throw()
		{
			return Logger::_print("Log", Color(0, 255, 110), oss, msg);
		}

		std::ostream &info(void) const throw()
		{
			return Logger::_print("Info", Color(3, 136, 252), this->_oss, "", false);
		}

		template <typename T>
		std::ostream &info(T msg) const throw()
		{
			return Logger::_print("Info", Color(3, 136, 252), this->_oss, msg);
		}

		template <typename T>
		std::ostream &info(T msg, std::ostream &oss) const throw()
		{
			return Logger::_print("Info", Color(3, 136, 252), oss, msg);
		}

		std::ostream &debug(void) const throw()
		{
			return Logger::_print("Debug", Color(149, 0, 255), this->_oss, "", false);
		}


		template <typename T>
		std::ostream &debug(T msg) const throw()
		{
			return Logger::_print("Debug", Color(149, 0, 255), this->_oss, msg);
		}

		template <typename T>
		std::ostream &debug(T msg, std::ostream &oss) const throw()
		{
			return Logger::_print("Debug", Color(149, 0, 255), oss, msg);
		}

		std::ostream &warning(void) const throw()
		{
			return Logger::_print("Warning", Color(255, 140, 0), this->_oss, "", false);
		}

		template <typename T>
		std::ostream &warning(T msg) const throw()
		{
			return Logger::_print("Warning", Color(255, 140, 0), this->_oss, msg);
		}

		template <typename T>
		std::ostream &warning(T msg, std::ostream &oss) const throw()
		{
			return Logger::_print("Warning", Color(255, 140, 0), oss, msg);
		}

		std::ostream &error(std::ostream &oss = std::cerr) const throw()
		{
			return Logger::_print("Error", Color(255, 0, 0), oss, "", false);
		}

		template <typename T>
		std::ostream &error(T msg, std::ostream &oss = std::cerr) const throw()
		{
			return Logger::_print("Error", Color(255, 0, 0), oss, msg);
		}
};

static const Logger _Logger("Static", Color(100, 100, 240));
