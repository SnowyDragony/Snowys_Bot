#pragma once
#include "ConsoleLogs.h"
#include <dpp/dpp.h>
#include <vector>

class CommandRegistry {
private:
	std::vector<dpp::slashcommand> commands;
	dpp::snowflake app_id;

public:
	explicit CommandRegistry(dpp::snowflake application_id) : app_id(application_id) {}

	void add_command(const dpp::slashcommand& command) {
		commands.push_back(command);
	}

	void sync_global_cmd(dpp::cluster& bot) {
		if (commands.empty()) return;

		bot.global_bulk_command_create(commands, [](const dpp::confirmation_callback_t& cb) {
			if (cb.is_error()) {
				LOG_ERROR("CommandRegistry: sync_global: Error bulk create: " << cb.get_error().message);
			}
			else {
				LOG_SUCCESS("CommandRegistry: sync_global: Success: All (" + std::to_string(cb.get<dpp::slashcommand_map>().size()) + ") global command has been synchronized");
			}
		});
	}

	void sync_guild_cmd(dpp::cluster& bot, dpp::snowflake guild_id) {
		if (commands.empty()) return;

		bot.guild_bulk_command_create(commands, guild_id, [guild_id](const dpp::confirmation_callback_t& cb) {
			if (cb.is_error()) {
				LOG_ERROR("CommandRegistry: sync_guild: Error bulk create for guild " << guild_id << ": " << cb.get_error().message);
			}
			else {
				LOG_SUCCESS("CommandRegistry: sync_guild: Success: All (" + std::to_string(cb.get<dpp::slashcommand_map>().size()) + ") guild command has been synchronized for guild " + std::to_string(guild_id));
			}
		});
	}
};