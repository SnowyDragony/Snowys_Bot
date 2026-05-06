#pragma once

#ifndef MUTE_COMMAND_H
#define MUTE_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_mute_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif // !MUTE_COMMAND_H