#include "../../../include/utils/helpers/DiceHelper.h"
#include "../../../include/utils/ConsoleLogs.h"
#include <dpp/dpp.h>

static std::string convert_dice_result_to_emote(int dice_result) {
	switch (dice_result) {
	case 1:
		return "<:DiceOne:1502832999790809238>";
	case 2:
		return "<:DiceTwo:1502833001434714252>";
	case 3:
		return "<:DiceThree:1502833003016097842>";
	case 4:
		return "<:DiceFour:1502833004190634074>";
	case 5:
		return "<:DiceFive:1502833005675151370>";
	case 6:
		return "<:DiceSix:1502833007357067274>";
	default:
		return "Invalid dice result";
	}
}


dpp::message create_initial_response(int64_t num_rolls, int64_t num_dices) {

	dpp::embed embed = dpp::embed()
		.set_title("Lancé de dés")
		.set_description(std::to_string(num_rolls) + " lancés de dés ont été choisis avec " + std::to_string(num_dices) + " dés !")
		.set_color(dpp::colors::cyan)
		.set_timestamp(std::time(nullptr));

	dpp::message response;
	response.add_embed(embed);
	return response;
}


dpp::message create_dice_message(int* dice_results, int num_dice, int i, int num_rolls) {

	std::string dice_emote_str;
	for (int j = 0; j < num_dice; ++j) {
		dice_emote_str += convert_dice_result_to_emote(dice_results[j]);
	}

	dpp::embed embed = dpp::embed()
		.set_title("Lancé de dés (" + std::to_string(i) + "/" + std::to_string(num_rolls) + ")")
		.add_field("", dice_emote_str, false)
		.add_field("Quel est le total ?", "Répondez avec la somme des " + std::to_string(num_dice) + " dés", false)
		.set_color(dpp::colors::cyan);


	dpp::message response;
	response.add_embed(embed);

	return response;
}


dpp::message create_results_message()
{
	dpp::message message;
	dpp::embed embed = dpp::embed()
	.set_title("Résultats")
	.set_timestamp(std::time(nullptr));
	
	message.add_embed(embed);
	return message;
}


dpp::message edit_results_message_while_playing(dpp::message& message, int* dice_results, int num_dice, int total_result, int user_guess)
{
	std::string dice_emote_str;
	for (int j = 0; j < num_dice; ++j) {
		dice_emote_str += convert_dice_result_to_emote(dice_results[j]);
	}
	
	if (user_guess == total_result)
		message.embeds[0].add_field(":white_check_mark:", dice_emote_str, true);
	else 
		message.embeds[0].add_field(":cross_mark:", dice_emote_str, true);
		
	message.embeds[0].add_field("Résultat", std::to_string(total_result), true);
	message.embeds[0].add_field("Ta réponse", std::to_string(user_guess), true);
	
	return message;
}



dpp::message final_edit_results_message(dpp::message& message, int correct_answers, int num_rolls) {
	float percentage = static_cast<float>(correct_answers) / num_rolls * 100;
	percentage = std::truncf(percentage);
	LOG_DEBUG("correct_answers: " << std::to_string(correct_answers) + ", num_rolls: " << std::to_string(num_rolls) + ", percentage: " << std::to_string(percentage));
	
	message.embeds[0].set_description("Votre score est de " + std::to_string(correct_answers) + "/" + std::to_string(num_rolls) + " (" + std::to_string(static_cast<int>(percentage)) + "%) !");
	message.embeds[0].set_timestamp(std::time(nullptr));
	
	if (percentage > 70)
		message.embeds[0].set_color(dpp::colors::cyan);
	else if (percentage > 50)
		message.embeds[0].set_color(dpp::colors::pink);
	else
		message.embeds[0].set_color(dpp::colors::purple);
	
	return message;
}



dpp::message edit_dice_message_success(dpp::message& message, int total_result) {
	message.embeds[0].fields[1]
		.name = "Bien joué !";
	message.embeds[0].fields[1]
		.value = "";
	message.embeds[0].set_color(dpp::colors::green);

	return message;
}


dpp::message edit_dice_message_fail(dpp::message& message, int total_result, int user_guess) {
	message.embeds[0].fields[1]
		.name = "Dommage";
	message.embeds[0].fields[1]
		.value = "Le résultat était " + std::to_string(total_result);
	message.embeds[0].fields[1]
		.is_inline = true;
	message.embeds[0].add_field("Votre réponse", std::to_string(user_guess), true);
	message.embeds[0].set_color(dpp::colors::red);
	
	return message;
}
