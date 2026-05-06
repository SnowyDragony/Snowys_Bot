#include "../../../include/utils/helpers/DiceHelper.h"
#include "../../../include/utils/ConsoleLogs.h"
#include <dpp/dpp.h>

static std::string convert_dice_result_to_emote(int dice_result) {
	switch (dice_result) {
	case 1:
		return "<:DiceOne:1493314537322053762>";
	case 2:
		return "<:DiceTwo:1493314543806582784>";
	case 3:
		return "<:DiceThree:1493314541474283651>";
	case 4:
		return "<:DiceFour:1493314535828750536>";
	case 5:
		return "<:DiceFive:1493314533752701019>";
	case 6:
		return "<:DiceSix:1493314539616206950>";
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
		.set_title("Lancé de dés")
		.set_description("Résultat du lancé de dés numéro " + std::to_string(i) + "/" + std::to_string(num_rolls))
		.add_field(dice_emote_str, "", false)
		.add_field("Quel est le total ?", "Répondez avec la somme des " + std::to_string(num_dice) + " dés", false)
		.set_color(dpp::colors::cyan)
		.set_timestamp(std::time(nullptr));


	dpp::message response;
	response.add_embed(embed);

	return response;
}


dpp::message create_result_message(int correct_answers, int num_rolls) {
	float percentage = static_cast<float>(correct_answers) / num_rolls * 100;
	percentage = std::truncf(percentage);
	LOG_DEBUG("correct_answers: " << std::to_string(correct_answers) + ", num_rolls: " << std::to_string(num_rolls) + ", percentage: " << std::to_string(percentage));
	dpp::embed embed = dpp::embed()
		.set_title("Résultats")
		.set_description("Votre score est de " + std::to_string(correct_answers) + "/" + std::to_string(num_rolls) + " (" + std::to_string(static_cast<int>(percentage)) + "%) !")
		.set_color(dpp::colors::cyan)
		.set_timestamp(std::time(nullptr));

	dpp::message response;
	response.add_embed(embed);

	return response;
}



dpp::message edit_dice_message_success(dpp::message& message, int total_result) {
	message.embeds[0].fields[1]
		.name = "Bien joué !";
	message.embeds[0].fields[1]
		.value = "Le résultat est " + std::to_string(total_result) + " !";
	message.embeds[0].set_color(dpp::colors::green).set_timestamp(std::time(nullptr));

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
	message.embeds[0].set_color(dpp::colors::red).set_timestamp(std::time(nullptr));;
	
	return message;
}
