#include "../include/SendCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include <string>
#include <variant>

dpp::task<void>	handle_send_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
	LOG_INFO("Handling /send command");

	// Récupérer le paramètre "message"
	auto param = event.get_parameter("message");
	std::string texte = "";

	// std::holds_alternative<T>(v) : Renvoie true si la variante v contient une valeur du type T.
	if (std::holds_alternative<std::string>(param)) {
		texte = std::get<std::string>(param);
	}
	dpp::message msg(texte.empty() ? "Message par defaut" : texte);
	msg.set_channel_id(event.command.channel_id);


	dpp::async thinking = event.co_thinking(true);
	co_await thinking;

	// Envoyer le message
	LOG_COMMAND("Commande /send utilisee avec comme texte '" << texte << "'");
	auto send_job = co_await bot.co_message_create(msg);
	if (send_job.is_error()) {
		LOG_ERROR("Erreur lors de l'envoi du message : " << send_job.get_error().message);
		event.edit_response(dpp::message("Erreur lors de l'envoi du message : " + send_job.get_error().message));
		co_return;
	}

	LOG_SUCCESS("Message successfuly sent!");
	event.edit_response(dpp::message("Message successfuly sent!"));
	co_return;
}