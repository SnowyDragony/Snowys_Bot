#pragma once

#ifndef COMMON_H
#define COMMON_H

#include <dpp/dpp.h>

/*
 *@brief Check something and return a success or failure result, along with an error code and message if applicable.
 *Codes : 
 *@see http codes
 */
struct CheckResult {
	bool	success = false;	// Whether the check was successful or not. Default value : false
	int		err_code = 0;		// Error code if the check failed. Default value : 0
	std::string message = "";	// Error message if the check failed. Default value : empty string
};


int find_role_pos(const dpp::snowflake role_id);
int	find_highest_role_pos(const dpp::guild_member& member);
int find_mod_role_pos(dpp::snowflake guild_id);
dpp::snowflake find_log_channel_id(dpp::snowflake guild_id, dpp::snowflake command_channel_id);
dpp::task<std::string> co_fetch_username(dpp::cluster& bot, const dpp::snowflake user_id, const bool user_is_in_guild);
dpp::task<dpp::guild> co_get_guild(dpp::cluster& bot, const dpp::snowflake guild_id);

/*
 * This function checks if the target user is currently banned in the guild by attempting to retrieve their ban information.
 * If there is an error while retrieving the ban information (other than a 404 Not Found), it logs the error and returns a CheckResult indicating that an internal error occurred.
 *
 * @return bool success : True if the user is banned, false otherwise.
 * @return string message : An error message if the user is not banned or if an internal error occurred, empty string otherwise.
 */
dpp::task<CheckResult> co_check_if_user_banned(dpp::cluster& bot, dpp::snowflake guild_id, dpp::snowflake target_id);

#endif // !COMMON_H