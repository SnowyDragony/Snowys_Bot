#pragma once

#ifndef SETTINGS_COMMAND_H
#define SETTINGS_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_settings_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

void	register_button_handlers(dpp::cluster& bot);

#endif // !SETTINGS_COMMAND_H