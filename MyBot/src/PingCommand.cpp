#include "../include/PingCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include <iostream>

dpp::task<void>	handle_ping_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
	LOG_INFO("Handling /ping command");
	dpp::message msg("Pong !");
	event.reply(msg);
	co_return;
}