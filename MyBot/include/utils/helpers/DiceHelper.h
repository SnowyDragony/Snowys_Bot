#pragma once
#ifndef DICE_HELPER_H
#define	DICE_HELPER_H

#include <dpp/dpp.h>

dpp::message create_initial_response(int64_t num_rolls, int64_t num_dices);
dpp::message create_dice_message(int* dice_results, int num_dice, int i, int num_rolls);

dpp::message create_results_message();
dpp::message edit_results_message_while_playing(dpp::message& message, int* dice_results, int num_dice, int total_result, int user_guess);
dpp::message final_edit_results_message(dpp::message& message, int correct_answers, int num_rolls);

dpp::message edit_dice_message_success(dpp::message& message, int total_result);
dpp::message edit_dice_message_fail(dpp::message& message, int total_result, int user_guess);

#endif // !DICE_HELPER_H
