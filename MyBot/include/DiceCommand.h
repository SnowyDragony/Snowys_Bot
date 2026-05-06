#pragma once
#ifndef DICE_COMMAND_H
#define DICE_COMMAND_H

#include <dpp/dpp.h>

dpp::task<void> handle_dice_command(const dpp::slashcommand_t& event, dpp::cluster& bot);

#endif