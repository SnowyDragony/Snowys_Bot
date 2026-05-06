#include "../../../db/BotDatabase.h"
#include "../../../include/utils/helpers/Common.h"
#include "../../../include/utils/ConsoleLogs.h"
#include <dpp/dpp.h>



int	find_highest_role_pos(const dpp::guild_member& member) {
	int pos = -1;
	for (dpp::snowflake role_id : member.get_roles()) {
		auto role = dpp::find_role(role_id);
		if (role == nullptr) continue;
		pos = std::max(pos, static_cast<int>(role->position));
	}
	return pos;
}



int find_role_pos(const dpp::snowflake role_id) {
	auto role = dpp::find_role(role_id);
	if (role == nullptr) return -1;
	return static_cast<int>(role->position);
}



int find_mod_role_pos(dpp::snowflake guild_id) {
	Database* db = Database::get_instance();
	if (db) {
		dpp::snowflake mod_role_id = db->get_mod_role(guild_id);
		if (mod_role_id != 0) {
			return find_role_pos(mod_role_id);
		}
		else {
			LOG_INFO("Moderator role not found in database. Skipping step");
			return -1;
		}
	}
	else {
		LOG_WARNING("Database instance is null. Failed to retrieve mod_role");
		return -1;
	}
}


dpp::snowflake find_log_channel_id(dpp::snowflake guild_id, dpp::snowflake command_channel_id) {
	Database* db = Database::get_instance();
	if (db) {
		dpp::snowflake log_channel_id = db->get_log_channel(guild_id);
		if (log_channel_id != 0) {
			return log_channel_id;
		}
		else {
			LOG_INFO("Log channel not found in database. Using command channel for log message");
			return command_channel_id;
		}
	}
	else {
		LOG_WARNING("Database instance is null. Using command channel for log message");
		return command_channel_id;
	}
}



dpp::task<std::string> co_fetch_username(dpp::cluster& bot, const dpp::snowflake user_id, const bool user_is_in_guild) {
	if (user_is_in_guild) {
		auto cached = dpp::find_user(user_id);
		if (cached) {
			LOG_DEBUG("Username cached");
			if (!cached->global_name.empty()) co_return cached->global_name;
			co_return cached->username;
		}

		LOG_INFO("Username not cached, fetching from API");
		auto user_cb = co_await bot.co_user_get(user_id);

		if (user_cb.is_error()) {
			LOG_WARNING("Cannot fetch user: " << user_cb.get_error().code);
			co_return "<@" + std::to_string(user_id) + ">";
		}


		if (auto* u = std::get_if<dpp::user>(&user_cb.value)) {
			LOG_INFO("User successfully found");
			if (!u->global_name.empty()) co_return u->global_name;
			else co_return u->username;
		}

		LOG_ERROR("co_user_get returned unexpected type");
	}
	LOG_INFO("User not in guild, using id to ping");
	co_return "<@" + std::to_string(user_id) + ">";
}



dpp::task<dpp::guild> co_get_guild(dpp::cluster& bot, const dpp::snowflake guild_id) {
	auto guild_cb = co_await bot.co_guild_get(guild_id);
	if (guild_cb.is_error()) {
		LOG_ERROR("Cannot fetch guild: " << guild_cb.get_error().code);
		co_return dpp::guild();
	}
	LOG_CHECK_SUCCESS("Guild information retrieved");
	co_return std::get<dpp::guild>(guild_cb.value);
}



dpp::task<CheckResult> co_check_if_user_banned(dpp::cluster& bot, dpp::snowflake guild_id, dpp::snowflake user_id) {
	auto ban_info_callback = co_await bot.co_guild_get_ban(guild_id, user_id);
	if (ban_info_callback.is_error()) {
		if (ban_info_callback.get_error().code == 10026) { // Discord -> 10026 Unknown Ban
			LOG_INFO("User is not banned");
			co_return{ false, 404, "The specified user is not currently banned." };
		}
		LOG_ERROR("Failed to retrieve ban information: " + ban_info_callback.get_error().message + " | err_code: " << ban_info_callback.get_error().code);
		co_return{ false, 500, "An internal error occured while attempting to get the ban information. Please try again later." };
	}
	co_return{ true, 200, "The specified user is currently banned." };
}