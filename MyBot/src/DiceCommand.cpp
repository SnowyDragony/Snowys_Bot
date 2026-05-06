#include "../include/DiceCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include "../include/utils/helpers/Common.h"
#include "../include/utils/helpers/DiceHelper.h"
#include <memory>
#include <random>
#pragma warning(disable : 26811)



dpp::task<void> handle_dice_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
	bool verbose = true;
	LOG_COMMAND("Handling /dice command");

	int64_t num_rolls = 1;
	int64_t num_dice = 2;

	auto rolls_param = event.get_parameter("number_of_rolls");
	if (std::holds_alternative<int64_t>(rolls_param))
		num_rolls = std::get<int64_t>(rolls_param);
	if (num_rolls < 1 || num_rolls > 20)
		num_rolls = 1;

	auto dices_param = event.get_parameter("number_of_dices");
	if (std::holds_alternative<int64_t>(dices_param))
		num_dice = std::get<int64_t>(dices_param);
	if (num_dice < 2 || num_dice > 10)
		num_dice = 2;
		
	int correct_answers = 0;

	if (verbose) LOG_INFO("Sending initial response with number of dice rolls selected");
	dpp::message initial_response = create_initial_response(num_rolls, num_dice);
	co_await event.co_reply(initial_response);

	if (verbose) LOG_INFO("Setting dice rolls parameters");
	std::random_device rd;						// Obtain a random number from hardware
	std::mt19937 gen(rd());						// Seed the generator
	std::uniform_int_distribution<> dis(1, 6);	// Define the range for the dice rolls (1-6)

	bool continue_rolls = true;
	int* dice_results = (int*)malloc(sizeof(int) * num_dice);

	if (verbose) LOG_INFO("Starting dice rolls loop");
	for (int i = 1; i <= num_rolls; ++i) {

		int total_result = 0;

		if (verbose) LOG_INFO("Rolling dice for roll number " + std::to_string(i));
		for (int j = 0; j < num_dice; ++j) {
			dice_results[j] = dis(gen);
			total_result += dice_results[j];
		}

		if (verbose) {
			std::string str = " (";
			for (int j = 0; j < num_dice; ++j) {
				str += std::to_string(dice_results[j]);
				if (j != (num_dice - 1)) str += " + ";
			}
			str += ").";
			LOG_INFO("Sending message for roll number " + std::to_string(i) + ". Result is " + std::to_string(total_result) + str);
		}
		dpp::message dice_message = create_dice_message(dice_results, num_dice, i, num_rolls);
		dice_message.set_channel_id(event.command.channel_id);

		auto dice_message_callback = co_await bot.co_message_create(dice_message);
		auto message = dice_message_callback.get<dpp::message>();

		if (verbose) LOG_INFO("Waiting for user response for roll number " + std::to_string(i));
		std::string user_reply = "";

		auto listener = bot.on_message_create([&](const dpp::message_create_t& ev) {
			if ((ev.msg.channel_id == event.command.channel_id &&
				 ev.msg.author.id == event.command.usr.id &&
				 ev.msg.author.id != bot.me.id)) {
				 user_reply = ev.msg.content;
			}
		});
	
		dpp::timer timeout_timer = bot.start_timer([&](dpp::timer) {}, 30);

		int k = 0;
		while (user_reply.empty()) {
			//if (verbose) LOG_DEBUG("Time waited: " + std::to_string(k));
			++k;
			co_await bot.co_sleep(1);
		}

		bot.stop_timer(timeout_timer);
		bot.on_message_create.detach(listener);

		if (user_reply.empty()) {
			if (verbose) LOG_INFO("No response received for roll number " + std::to_string(i) + " within the time limit");
			if (!continue_rolls) // Si l'utilisateur n'a pas répondu au lancé précédent, assumer qu'il ne joue plus et arrêter la commande
				break;
			continue_rolls = false;
			// Créer fonction pour envoyer un message propre
			co_await bot.co_message_create(dpp::message(event.command.channel_id, "Perdu ! Temps écoulé pour le lancer n°" + std::to_string(i) + ". La réponse était " + std::to_string(total_result)));
		}
		else continue_rolls = true; // Si l'utilisateur répond, continuer les lancés
		
		if (verbose) LOG_INFO("Received response for roll number " + std::to_string(i) + ": '" + user_reply + "'. Checking if correct");
		try {
			int user_guess = std::stoi(user_reply);
			if (user_guess == total_result) {
				if (verbose) LOG_INFO("User guess is correct for roll number " + std::to_string(i));
				correct_answers++;
				message = edit_dice_message_success(message, total_result);
				co_await bot.co_message_edit(message);
			} else {
				if (verbose) LOG_INFO("User guess is incorrect for roll number " + std::to_string(i) + ". Guess: " + std::to_string(user_guess) + ", actual: " + std::to_string(total_result));
				message = edit_dice_message_fail(message, total_result, user_guess);
				co_await bot.co_message_edit(message);
			}
		}
		catch (...) {
			if (verbose) LOG_INFO("User response for roll number " + std::to_string(i) + " is not a valid integer: '" + user_reply + "'.");
			continue_rolls = false;
		}
	}

	free(dice_results);

	if (verbose) LOG_INFO("Final score: " + std::to_string(correct_answers) + " out of " + std::to_string(num_rolls));
	dpp::message results_message = create_result_message(correct_answers, num_rolls);
	results_message.channel_id = event.command.channel_id;
	co_await bot.co_message_create(results_message);

	co_return;
}