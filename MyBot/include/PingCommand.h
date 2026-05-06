#pragma once

#ifndef PING_COMMAND_H
#define PING_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_ping_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif // !PING_COMMAND_H