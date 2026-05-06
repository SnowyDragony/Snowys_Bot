#pragma once

#ifndef UNMUTE_COMMAND_H
#define UNMUTE_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_unmute_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif // !UNMUTE_COMMAND_H