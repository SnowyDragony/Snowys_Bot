#pragma once

#ifndef UNBAN_COMMAND_H
#define UNBAN_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_unban_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif // !UNBAN_COMMAND_H