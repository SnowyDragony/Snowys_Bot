#pragma once

#ifndef SEND_COMMAND_H
#define SEND_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_send_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif // !SEND_COMMAND_H