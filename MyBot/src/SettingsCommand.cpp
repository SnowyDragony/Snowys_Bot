#include "../include/SettingsCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include "../db/BotDatabase.h"

#include <dpp/appcommand.h>
#include <unordered_map>
#include <functional>
#include <cstdint>


// Gestion des interactions avec les boutons des embeds
static std::unordered_map<std::string, std::function<void(const dpp::button_click_t&, dpp::cluster&)>> button_handlers;
static std::unordered_map<std::string, std::function<void(const dpp::select_click_t&, dpp::cluster&)>> select_handlers;

#pragma region buttons_commands
void settings_button_moderator_role(const dpp::button_click_t& click, dpp::cluster& bot);
void settings_button_log_channel(const dpp::button_click_t& click, dpp::cluster& bot);
void settings_button_back(const dpp::button_click_t& click, dpp::cluster& bot);

void settings_button_select_channel(const dpp::select_click_t& select, dpp::cluster& bot);
void settings_button_select_role(const dpp::select_click_t& select, dpp::cluster& bot);
#pragma endregion

static void init_handlers() {
	button_handlers.emplace("settings:back",			settings_button_back);
	button_handlers.emplace("settings:moderator_role",	settings_button_moderator_role);
	button_handlers.emplace("settings:log_channel",		settings_button_log_channel);

	select_handlers.emplace("settings:channel_select",	settings_button_select_channel);
	select_handlers.emplace("settings:role_select",		settings_button_select_role);
}

void	register_button_handlers(dpp::cluster& bot) {
	init_handlers();

	bot.on_button_click([&bot](const dpp::button_click_t& click) {
		auto it = button_handlers.find(click.custom_id);
		LOG_INFO("Handling click action. Click name: " << click.custom_id);
		if (it != button_handlers.end()) {
			it->second(click, bot);			// Appelle la fonction
		}
		else {
			click.reply("Action inconnue", [](const dpp::confirmation_callback_t& cb) {
				if (cb.is_error()) {
					LOG_ERROR("register_button_handlers: Impossible de trouver l'id de l'action: Impossible de répondre a l'interaction: " << cb.get_error().message);
				}
				else {
					LOG_SUCCESS("register_button_handlers: Impossible de trouver l'id de l'action: reponse a l'interaction envoyee avec succes");
				}
			});
		}
	});

	bot.on_select_click([&bot](const dpp::select_click_t& select) {
		auto it = select_handlers.find(select.custom_id);
		LOG_INFO("Handling select action. Select name: " << select.custom_id);
		if (it != select_handlers.end()) {
			it->second(select, bot);
		}
		else {
			select.reply("Action inconnue", [](const dpp::confirmation_callback_t& cb) {
				if (cb.is_error()) {
					LOG_ERROR("register_button_handlers: Impossible de trouver l'id de l'action: Impossible de répondre a l'interaction: " << cb.get_error().message);
				}
				else {
					LOG_SUCCESS("register_button_handlers: Impossible de trouver l'id de l'action: reponse a l'interaction envoyee avec succes");
				}
			});
		}
	});


}


// Gestion de la commande slash
dpp::task<void>	handle_settings_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {

	// Construire l'embed des paramètres
	dpp::embed settings_embed = dpp::embed()
		.set_title("Paramètres du bot")
		.set_description("Choisissez une option ci-dessous pour configurer le bot")
		.set_color(dpp::colors::yellow_orange);



	// Construire le container pour les boutons
	dpp::component action_row = dpp::component()
		.set_type(dpp::cot_action_row);

	// Bouton pour choisir le log_channel
	dpp::component btn_log = dpp::component()
		.set_label("Log Channel")
		.set_style(dpp::cos_primary)
		// custom id envoyé lors du clic sur le bouton
		.set_id("settings:log_channel");

	// Bouton pour choisir le moderator_role
	dpp::component btn_role = dpp::component()
		.set_label("Moderator Role")
		.set_style(dpp::cos_primary)
		// custom id envoyé lors du clic sur le bouton
		.set_id("settings:moderator_role");

	// Ajouter les boutons à l'action row (container)
	action_row.add_component(btn_log);
	action_row.add_component(btn_role);



	// Construire et envoyer le message
	dpp::message settings_msg = dpp::message()
		.add_embed(settings_embed)
		.add_component(action_row);

	event.co_reply(settings_msg);
	co_return;
}


//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################


// Fonctions de comportement des boutons clickables
void settings_button_moderator_role(const dpp::button_click_t& click, dpp::cluster& bot) {
	dpp::embed selection_embed = dpp::embed()
		.set_title("Moderator Role")
		.set_color(dpp::colors::yellow_orange);


	LOG_INFO("Attempt to get mod_role_id from database")
	Database* db = Database::get_instance();
	if (db) {
		dpp::snowflake mod_role_id = db->get_mod_role(click.command.guild_id);
		if (mod_role_id != 0) {
			selection_embed.set_description("Role modérateur défini: <@&" + std::to_string(mod_role_id) + ">");
			LOG_INFO("mod_role_id found: " << std::to_string(mod_role_id));
		}
		else {
			selection_embed.set_description("Choisissez un role comme modérateur pour utiliser les commandes de modération");
			LOG_INFO("mod_role_id not found. Asking to select a role")
		}
	}
	else {
		LOG_INFO("Database instance is null. Asking to select a role");
		selection_embed.set_description("Choisissez un role comme modérateur pour utiliser les commandes de modération");

	}

	// Container pour le selecteur de salon
	dpp::component select_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Selecteur de salon
	dpp::component role_select = dpp::component()
		.set_type(dpp::cot_role_selectmenu)
		.set_id("settings:role_select")
		.set_placeholder("Selectionnez un role");

	select_row.add_component(role_select);

	// Container pour le bouton de retour
	dpp::component back_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Bouton de retour
	dpp::component back_btn = dpp::component()
		.set_label("Retour")
		.set_style(dpp::cos_secondary)
		.set_id("settings:back");

	back_row.add_component(back_btn);



	dpp::message edit = dpp::message()
		.add_embed(selection_embed)
		.add_component(select_row)
		.add_component(back_row);

	// Acknowledge puis éditer le message
	try {
		click.reply(dpp::ir_update_message, edit, [](const dpp::confirmation_callback_t& cb) {
			if (cb.is_error()) {
				LOG_ERROR("handle_settings_command: btn_role: click.reply() failed: " << cb.get_error().message);
			}
			else {
				LOG_SUCCESS("handle_settings_command: btn_role: Edit du message vers role_selection reussi");
			}
			});
	}
	catch (const dpp::exception& e) {
		LOG_ERROR("handle_settings_command: btn_role: error: " << e.what());
	}
	return;
}



void settings_button_log_channel(const dpp::button_click_t& click, dpp::cluster& bot) {
	dpp::embed selection_embed = dpp::embed()
		.set_title("Log Channel")
		.set_color(dpp::colors::yellow_orange);


	LOG_INFO("Attempt to get log_channel from database")
	Database* db = Database::get_instance();
	if (db) {
		dpp::snowflake log_channel = db->get_log_channel(click.command.guild_id);
		if (log_channel != 0) {
			selection_embed.set_description("Salon de log défini: <#" + std::to_string(log_channel) + ">");
			LOG_INFO("log_channel found: " << std::to_string(log_channel));
		}
		else {
			selection_embed.set_description("Choisissez un salon pour les logs du bot");
			LOG_INFO("log_channel not found. Asking to select a channel")
		}
	}
	else {
		selection_embed.set_description("Choisissez un salon pour les logs du bot");
		LOG_INFO("Database instance is null. Asking to select a channel")
	}


	// Container pour le selecteur de salon
	dpp::component select_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Selecteur de salon
	dpp::component channel_select = dpp::component()
		.set_type(dpp::cot_channel_selectmenu)
		.set_id("settings:channel_select")
		.set_placeholder("Selectionnez un salon");

	select_row.add_component(channel_select);

	// Container pour le bouton de retour
	dpp::component back_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Bouton de retour
	dpp::component back_btn = dpp::component()
		.set_label("Retour")
		.set_style(dpp::cos_secondary)
		.set_id("settings:back");

	back_row.add_component(back_btn);



	dpp::message edit = dpp::message()
		.add_embed(selection_embed)
		.add_component(select_row)
		.add_component(back_row);

	// Acknowledge puis éditer le message
	try {
		click.reply(dpp::ir_update_message, edit, [](const dpp::confirmation_callback_t& cb) {
			if (cb.is_error()) {
				LOG_ERROR("handle_settings_command: btn_log: click.reply() failed: " << cb.get_error().message);
			}
			else {
				LOG_SUCCESS("handle_settings_command: btn_log: Edit du message vers channel_selection reussi");
			}
			});
	}
	catch (const dpp::exception& e) {
		LOG_ERROR("handle_settings_command: btn_log: error: " << e.what());
	}
	return;
}



void settings_button_back(const dpp::button_click_t& click, dpp::cluster& bot) {
	// reconstruire l'embed principal
	dpp::embed settings_embed = dpp::embed()
		.set_title("Paramètres du bot")
		.set_description("Choisissez une option ci-dessous pour configurer le bot")
		.set_color(dpp::colors::yellow_orange);

	// Reconstruire les boutons principaux
	dpp::component action_row = dpp::component()
		.set_type(dpp::cot_action_row);

	dpp::component btn_log = dpp::component()
		.set_label("Log Channel")
		.set_style(dpp::cos_primary)
		.set_id("settings:log_channel");
	dpp::component btn_role = dpp::component()
		.set_label("Moderator Role")
		.set_style(dpp::cos_primary)
		.set_id("settings:moderator_role");

	action_row.add_component(btn_log);
	action_row.add_component(btn_role);

	dpp::message edit = dpp::message()
		.add_embed(settings_embed)
		.add_component(action_row);

	// Acknowledge puis éditer le message
	try {
		click.reply(dpp::ir_update_message , edit, [](const dpp::confirmation_callback_t& cb) {
			if (cb.is_error()) {
				LOG_ERROR("handle_settings_command: back_button: click.reply() failed: " << cb.get_error().message);
			}
			else {
				LOG_SUCCESS("handle_settings_command: back_button: Retour en arriere effectue");
			}
			});
	}
	catch (const dpp::exception& e) {
		LOG_ERROR("handle_settings_command: back_button: error: " << e.what());
	}
	return;
}


//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################


// Fonctions de comportement des boutons de selection
void settings_button_select_channel(const dpp::select_click_t& select, dpp::cluster& bot) {
	if (select.values.empty()) {
		select.reply(dpp::ir_channel_message_with_source, "Aucun salon selectionne.");
		return;
	}
	// La valeur renvoyée est un string contenant l'id du channel
	std::string channel_id_string = select.values.front();
	dpp::snowflake channel_id = select.values.front();


	LOG_INFO("Attempt to set log_channel into database");
	Database* db = Database::get_instance();
	if (db) {
		try {
			db->set_log_channel(select.command.guild_id, channel_id);
		}
		catch (const dpp::exception& e) {
			LOG_ERROR("set_log_channel: Error: " << e.what());
		}

		// Vérification de l'enregistrement
		dpp::snowflake log_channel = db->get_log_channel(select.command.guild_id);
		if (log_channel != 0) {
			LOG_SUCCESS("Enregistrement du log_channel réussi");
		}
		else {
			LOG_WARNING("Enregistrement du log_channel échoué");
		}
	}


	// Mettre à jour l'embed
	dpp::embed updated = dpp::embed()
		.set_title("Log Channel")
		.set_description("Salon de logs défini: <#" + channel_id_string + ">")
		.set_color(dpp::colors::green);

	// Container pour le selecteur de salon
	dpp::component select_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Selecteur de salon
	dpp::component channel_select = dpp::component()
		.set_type(dpp::cot_channel_selectmenu)
		.set_id("settings:channel_select")
		.set_placeholder("Selectionnez un salon");

	select_row.add_component(channel_select);

	// Container pour le bouton de retour
	dpp::component back_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Bouton de retour
	dpp::component back_btn = dpp::component()
		.set_label("Retour")
		.set_style(dpp::cos_secondary)
		.set_id("settings:back");

	back_row.add_component(back_btn);

	dpp::message edit = dpp::message()
		.add_embed(updated)
		.add_component(select_row)
		.add_component(back_row);

	// Acknowledge puis éditer le message
	try {
		select.reply(dpp::ir_update_message, edit, [](const dpp::confirmation_callback_t& cb) {
			if (cb.is_error()) {
				LOG_ERROR("handle_settings_command: channel_select: select.reply() failed: " << cb.get_error().message);
			}
			else {
				LOG_SUCCESS("handle_settings_command: channel_select: Channel correctement selectionne");
			}
			});
	}
	catch (const dpp::exception& e) {
		LOG_ERROR("handle_settings_command: channel_select: error: " << e.what());
	}
	return;
}



void settings_button_select_role(const dpp::select_click_t& select, dpp::cluster& bot) {
	if (select.values.empty()) {
		select.reply(dpp::ir_channel_message_with_source, "Aucun role selectionne.");
		return;
	}
	// La valeur renvoyée est un string contenant l'id du channel
	std::string role_id_string = select.values.front();
	dpp::snowflake role_id = select.values.front();

	LOG_INFO("Attempt to set mod_role_id into database");
	Database* db = Database::get_instance();
	if (db) {
		try {
			db->set_mod_role(select.command.guild_id, role_id);
		}
		catch (const dpp::exception& e) {
			LOG_ERROR("set_mod_role: Error: " << e.what());
		}

		// Vérification de l'enregistrement
		dpp::snowflake mod_role = db->get_mod_role(select.command.guild_id);
		if (mod_role != 0) {
			LOG_SUCCESS("Enregistrement du mod_role_id réussi");
		}
		else {
			LOG_WARNING("Enregistrement du mod_role_id échoué");
		}
	}


	// Mettre à jour l'embed
	dpp::embed updated = dpp::embed()
		.set_title("Moderator Role")
		.set_description("Role modérateur défini: <@&" + role_id_string + ">")
		.set_color(dpp::colors::green);

	// Container pour le selecteur de salon
	dpp::component select_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Selecteur de salon
	dpp::component role_select = dpp::component()
		.set_type(dpp::cot_role_selectmenu)
		.set_id("settings:role_select")
		.set_placeholder("Selectionnez un role");

	select_row.add_component(role_select);

	// Container pour le bouton de retour
	dpp::component back_row = dpp::component()
		.set_type(dpp::cot_action_row);
	// Bouton de retour
	dpp::component back_btn = dpp::component()
		.set_label("Retour")
		.set_style(dpp::cos_secondary)
		.set_id("settings:back");

	back_row.add_component(back_btn);

	dpp::message edit = dpp::message()
		.add_embed(updated)
		.add_component(select_row)
		.add_component(back_row);

	// Acknowledge puis éditer le message
	try {
		select.reply(dpp::ir_update_message, edit, [](const dpp::confirmation_callback_t& cb) {
			if (cb.is_error()) {
				LOG_ERROR("handle_settings_command: role_select: select.reply() failed: " << cb.get_error().message);
			}
			else {
				LOG_SUCCESS("handle_settings_command: role_select: Role correctement selectionne");
			}
			});
	}
	catch (const dpp::exception& e) {
		LOG_ERROR("handle_settings_command: role_select: error: " << e.what());
	}
	return;
}
