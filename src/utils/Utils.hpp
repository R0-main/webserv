#pragma once
#include "Logger.hpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>



class Utils
{
	public:
		virtual ~Utils() = 0;

		static void check(int r, Logger &logger, const char *message) throw()
		{
			if (r == -1)
			{
				logger.error(message);
				exit(EXIT_FAILURE); // FIX THAT FOR LEAKS
			}
		}
};


