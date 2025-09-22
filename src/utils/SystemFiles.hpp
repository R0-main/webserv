/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SystemFiles.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 14:39:34 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 14:39:45 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Logger.hpp"
#include "String.hpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cstring>



class SystemFiles
{
	public:
		virtual ~SystemFiles() = 0;

		static bool isDirectory(String &path) {
			struct stat info;
			if (stat(path.c_str(), &info) != 0) {
				return false;
			}
			return S_ISDIR(info.st_mode);
		}

		static bool isFile(String &path) {
			struct stat info;
			if (stat(path.c_str(), &info) != 0) {
				return false;
			}
			return S_ISREG(info.st_mode);
		}
};


