#pragma once

#include <dpp/utility.h>
#include <iostream>
#include <string>

namespace console {
	enum class ConsoleColor {
		BLACK = 30,
		DARK_RED = 31,
		DARK_GREEN = 32,
		DARK_YELLOW = 33,
		DARK_BLUE = 34,
		DARK_MAGENTA = 35,
		DARK_CYAN = 36,
		GRAY = 37,

		DARK_GRAY = 90,
		RED = 91,
		GREEN = 92,
		YELLOW = 93,
		BLUE = 94,
		MAGENTA = 95,
		CYAN = 96,
		WHITE = 97
	};
	
	inline void	set_console_color(ConsoleColor color) {
		std::cout << "\033[" << static_cast<int>(color) << "m";
		std::cerr << "\033[" << static_cast<int>(color) << "m";
	}

	inline void	reset_console_color() {
		std::cout << "\033[0m";
		std::cerr << "\033[0m";
	}
}


#define LOG_COMMAND(msg)			{ console::set_console_color(console::ConsoleColor::MAGENTA);			std::cout << "\n[CMD-] "	<< msg;		console::reset_console_color(); }
#define LOG_ERROR(msg)				{ console::set_console_color(console::ConsoleColor::RED);				std::cerr << "\n[ERR-] "	<< msg;		console::reset_console_color(); }
#define LOG_WARNING(msg)			{ console::set_console_color(console::ConsoleColor::YELLOW);			std::cout << "\n[WARN] "	<< msg;		console::reset_console_color(); }
#define LOG_SUCCESS(msg)			{ console::set_console_color(console::ConsoleColor::GREEN);				std::cout << "\n[SUCC] "	<< msg;		console::reset_console_color(); }
#define LOG_PROCESS_SUCCESS(msg)	{ console::set_console_color(console::ConsoleColor::GREEN);				std::cout << "\r[SUCC] "	<< msg;		console::reset_console_color(); }
#define LOG_DEBUG(msg)				{ console::set_console_color(console::ConsoleColor::DARK_YELLOW);		std::cout << "\n[DBUG] "	<< msg;		console::reset_console_color(); }
#define LOG_INFO(msg)				{ console::set_console_color(console::ConsoleColor::BLUE);				std::cout << "\n[INFO] "	<< msg;		console::reset_console_color(); }
#define LOG_CHECK(msg)				{ console::set_console_color(console::ConsoleColor::CYAN);				std::cout << "\n[CHCK] "	<< msg;		console::reset_console_color(); }
#define LOG_CHECK_SUCCESS(msg)		{ console::set_console_color(console::ConsoleColor::GREEN);				std::cout << "\r[SUCC] "	<< msg;		console::reset_console_color(); }
#define LOG_CHECK_FAIL(msg)			{ console::set_console_color(console::ConsoleColor::RED);				std::cout << "\r[FAIL] "	<< msg;		console::reset_console_color(); }
#define LOG_UTILITY(msg)			{ console::set_console_color(console::ConsoleColor::WHITE);				std::cout << "\n[" << dpp::utility::current_date_time() << "] " << msg; console::reset_console_color(); }
