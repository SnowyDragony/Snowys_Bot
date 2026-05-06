// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

//Bot
#include <dpp/dpp.h>
#include "Snowys_Bot.h"
#include "GetBotToken.h"
#include <utility>
#include <fstream>
#include <string>

// Command Map
#include <functional>
#include <map>

// Utils
#include "include/utils/ConsoleLogs.h"
#include "include/utils/CommandRegistry.h"

// Database
#include "db/BotDatabase.h"

// Command Files
#include "include/BanCommand.h"
#include "include/DiceCommand.h"
#include "include/MuteCommand.h"
#include "include/PingCommand.h"
#include "include/SendCommand.h"
#include "include/SettingsCommand.h"
#include "include/UnbanCommand.h"
#include "include/UnmuteCommand.h"

#pragma region token

constexpr dpp::snowflake MY_GUILD_ID = 1412419747278098434;
constexpr dpp::snowflake GUILD_CHATON = 1464777599543676951;

#pragma endregion

int main() {
	load_env_file(PROJECT_ROOT "/.env");
	std::string token;
	try
	{
		token = get_bot_token("DISCORD_TOKEN");
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return 1;
	}
	LOG_DEBUG("Token correctly read.");

	dpp::cluster snow_bot(token, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content);	// i_default_intents : inclut la gestion des membres
																												// i_guild_members   : permet de recuperer les informations des membres du serveur
																												// i_message_intent  : inclut les raisons
	
	Database* db = Database::get_instance();
	if (!db) {
		LOG_ERROR("Impossible to initialize database");
		return 1;
	}
	else {
		LOG_SUCCESS("Database initialized successfully");
	}



	//SetConsoleOutputCP(CP_UTF8);
	//SetConsoleCP(CP_UTF8);
	// Logs du bot dans la console
	snow_bot.on_log([](const dpp::log_t& event) {
		LOG_UTILITY(event.message);
	});



	// Map pour associer les commandes à une fonction
	std::map <std::string, std::function<dpp::task<void>(const dpp::slashcommand_t&, dpp::cluster&)>> command_map = {
		{"ban",		handle_ban_command},
		{"dice",	handle_dice_command},
		{"mute",	handle_mute_command},
		{"ping",	handle_ping_command},
		{"send",	handle_send_command},
		{"settings",handle_settings_command},
		{"unban",	handle_unban_command},
		{"unmute",	handle_unmute_command}
	};
	LOG_DEBUG("Command map created successfully");

	// Map pour associer les custom_id des boutons à une fonction
	//register_button_handlers(snow_bot);

	// Map pour synchroniser les commandes du bot
	CommandRegistry global_command_registry(snow_bot.me.id);
	CommandRegistry guild_command_registry(snow_bot.me.id);
	CommandRegistry chaton_command_registry(snow_bot.me.id);

	// bot.on_slashcommand == quand une commande slash est utilisee
	snow_bot.on_slashcommand([&command_map, &snow_bot](const dpp::slashcommand_t& event) -> dpp::task<void> {
		auto it = command_map.find(event.command.get_command_name());
		if (it != command_map.end()) {
			co_await it->second(event, snow_bot);
			LOG_COMMAND("Commande " << event.command.get_command_name() << " executee a " << dpp::utility::current_date_time());
		}
		else {
			LOG_WARNING( "Commande inconnue " << event.command.get_command_name() << " a " << dpp::utility::current_date_time());
			event.reply("Commande non reconnue");
		}
	});



	snow_bot.on_ready([&snow_bot, &global_command_registry, &guild_command_registry, &chaton_command_registry](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {

	
			#pragma region register_commands

			// dpp::slashcommand command_name("command_name", "command description", dpp::cluster& bot.me.id);
			// command_name.add_option(dpp::command_option(dpp::co_type, "option_name", "option description", <mandatory>: true/false));


			//Ban
			dpp::slashcommand ban_cmd("ban", "Ban a member from the server", snow_bot.me.id);
			ban_cmd.add_option(dpp::command_option(dpp::co_user, "user", "User to ban", true));
			ban_cmd.add_option(dpp::command_option(dpp::co_string, "reason", "Reason of the ban", false)
				.set_min_length(1)
				.set_max_length(512));
			global_command_registry.add_command(ban_cmd);


			//Unban
			dpp::slashcommand unban_cmd("unban", "Unban a member from the server", snow_bot.me.id);
			unban_cmd.add_option(dpp::command_option(dpp::co_user, "user", "User to unban", true));
			unban_cmd.add_option(dpp::command_option(dpp::co_string, "reason", "Reason of the unban", false)
				.set_min_length(1)
				.set_max_length(512));
			global_command_registry.add_command(unban_cmd);


			//Ping
			dpp::slashcommand ping_cmd("ping", "Pong!", snow_bot.me.id);
			global_command_registry.add_command(ping_cmd);


			//Send
			dpp::slashcommand send_cmd("send", "Send a message with the bot!", snow_bot.me.id);
			send_cmd.add_option(dpp::command_option(dpp::co_string, "message", "Message to send", true));
			global_command_registry.add_command(send_cmd);


			//Settings
			dpp::slashcommand settings_cmd("settings", "Show and manage the bot settings", snow_bot.me.id);
			settings_cmd.default_member_permissions = dpp::p_administrator; // Only admins can use this command by default
			global_command_registry.add_command(settings_cmd);


			//Mute
			dpp::slashcommand mute_cmd("mute", "Mute a member from the server", snow_bot.me.id);
			mute_cmd.add_option(dpp::command_option(dpp::co_user, "user", "User to mute", true));
			mute_cmd.add_option(dpp::command_option(dpp::co_integer, "duration", "Duration in minutes (10'080 mins or 7 days max)", false)
				.set_min_value(1)
				.set_max_value(10080)); // 7 days
			mute_cmd.add_option(dpp::command_option(dpp::co_string, "reason", "Reason of the mute", false)
				.set_min_length(1)
				.set_max_length(512));
			guild_command_registry.add_command(mute_cmd);


			//Unmute
			dpp::slashcommand unmute_cmd("unmute", "Unmute a member from the server", snow_bot.me.id);
			unmute_cmd.add_option(dpp::command_option(dpp::co_user, "user", "User to unmute", true));
			guild_command_registry.add_command(unmute_cmd);


			//Dés
			dpp::slashcommand dice_cmd("dice", "Roll 2 dices and tell me the result !", snow_bot.me.id);
			dice_cmd.add_option(dpp::command_option(dpp::co_integer, "number_of_rolls", "Number of time you want to roll the dices", true)
				.set_min_value(1)
				.set_max_value(20));
			dice_cmd.add_option(dpp::command_option(dpp::co_integer, "number_of_dices", "Number of dices you want to roll", false)
				.set_min_value(2)
				.set_max_value(10));
			chaton_command_registry.add_command(dice_cmd);

			//verbose
			dpp::slashcommand verbose_cmd("verbose", "Toggle verbose mode for the bot (more detailed logs)", snow_bot.me.id);
			verbose_cmd.add_option(dpp::command_option(dpp::co_boolean, "enable", "Enable or disable verbose mode", true));
			guild_command_registry.add_command(verbose_cmd);
			// /!\ Ajouter variable globale au bot et a transmettre à toutes les commandes lors de leur exécution pour toggle le mode verbose
			// en temps réel

			#pragma endregion register_commands

			#pragma region Delete commands

			// Supprimer une commande globale
			/*
			snow_bot.global_commands_get([&snow_bot](const dpp::confirmation_callback_t& callback) {
				if (callback.is_error()) {
					LOG_ERROR("Erreur lors de la recuperation des commandes globales : " << callback.get_error().message);
				}
				else {
					auto commands = std::get<dpp::slashcommand_map>(callback.value);
					for (const auto& [id, cmd] : commands) {
						if (cmd.name == "command_name") {
							snow_bot.global_command_delete(id);
							LOG_WARNING("Deleted global command: " << cmd.name);
						}
					}
				}
			});
			*/

			//Suppression de toutes les commandes de guilde
			/*
			snow_bot.guild_commands_get(MY_GUILD_ID, [&snow_bot](const dpp::confirmation_callback_t& callback) {
				if (!callback.is_error()) {
					auto commands = std::get<dpp::slashcommand_map>(callback.value);
					for (const auto& [id, cmd] : commands) {
						snow_bot.guild_command_delete(id, MY_GUILD_ID);
						LOG_WARNING("Commande guilde " << cmd.name << " supprimee");
					}
				}
			});
			*/

			#pragma endregion


			// Enregistrer la slash command globalement (prends jusqu'a 1h pour apparaitre)
			// Pour tester plus vite, remplacer par :
			//bot.guild_command_create(dpp::slashcommand("command_name", "command_behaviour", 1234567890123456789ULL), MY_GUILD_ID);
			// Permet de tester plus vite, mais restreint l'utilisation des commande à un seul serveur via MY_GUILD_ID


			global_command_registry.sync_global_cmd(snow_bot);
			guild_command_registry.sync_guild_cmd(snow_bot, MY_GUILD_ID);
			chaton_command_registry.sync_guild_cmd(snow_bot, GUILD_CHATON);

		}
	});

	// Demarrer le bot (st_wait bloque jusqu'a shutdown)
	snow_bot.start(dpp::st_wait);
	return 0;
}
