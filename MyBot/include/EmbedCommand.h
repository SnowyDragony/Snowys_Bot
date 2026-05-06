#pragma once

#ifndef EMBED_COMMAND_H
#define EMBED_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void>	handle_embed_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif // !EMBED_COMMAND_H