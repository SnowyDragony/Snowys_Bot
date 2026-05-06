#pragma once
#ifndef BAN_COMMAND_H
#define BAN_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_ban_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif // !BAN_COMMAND_H