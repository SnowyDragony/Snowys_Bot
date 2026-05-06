#include "../../../include/utils/helpers/MuteHelper.h"
#include "../../../include/utils/ConsoleLogs.h"
#include <dpp/dpp.h>



CheckResult get_target_member(dpp::interaction command, MuteData& data) {
	try {
		data.target_member = command.get_resolved_member(data.target_user_id);
	}
	catch (const dpp::exception& e) {
		LOG_INFO("Target user not resolved as a member: " << e.what());
		return { false, 404, "The specified user is not in the server." };
	}
	return { true, 200, "" };
}



CheckResult check_can_mute(const MuteData& data, dpp::snowflake& bot_id) {
	if (data.issuer_user_id != data.guild.owner_id && data.issuer_member.get_roles().empty())	// Block interaction if issuer only have @everyone role
		return { false, 403, "You don't have the required permissions to use this command (no role)." };
	if (!data.issuer_permissions.has(dpp::p_moderate_members))									// Verify if the issuer has the required permissions to mute members
		return { false, 403, "You don't have the required permissions to mute someone." };
	if (!data.app_permissions.has(dpp::p_moderate_members))										// Verify if the bot has the required permissions to mute members
		return { false, 403, "The bot doesn't have the required permissions to mute anyone." };
	if (data.target_user_id == data.guild.owner_id)												// Verify if the target is not the server owner
		return { false, 403, "You cannot mute the server owner." };
	if (data.target_permissions.has(dpp::p_administrator))										// Verify if the target is not an admin
		return { false, 403, "You cannot mute an admin." };
	if (data.target_user_id == data.issuer_user_id)												// Verify if the target is the issuer
		return { false, 403, "You cannot mute yourself." };
	if (data.target_user_id == bot_id)															// Verify if the target is the bot itself
		return { false, 403,  "I cannot mute myself." };

	return { true, 200, "" };
}



CheckResult check_role_hierarchy(dpp::snowflake bot_id, const MuteData& data) {
	if (data.app_highest_role_position <= data.target_highest_role_position && bot_id != data.guild.owner_id)
		return { false, 403, "I cannot mute someone whose role is equal or higher than mine." };

	if (data.issuer_user_id == data.guild.owner_id) 
		return { true, 200, "Owner performing command, bypassing role hierarchy" };
	if (data.issuer_permissions.has(dpp::p_administrator))
		return { true, 200, "Administrator performing command, bypassing role hierarchy" };

	if (data.target_highest_role_position >= data.issuer_highest_role_position)
		return { false, 403, "You cannot mute someone whose role is equal or higher than yours." };
	if (data.issuer_highest_role_position < data.mod_role_position)
		return { false, 403, "You cannot mute anyone as your role is lower than the defined moderator role." };
	
	return { true, 200, "Issuer can mute the target" };
}



dpp::task<void> co_send_mute_interaction_reply(const dpp::slashcommand_t& event, const MuteData& data) {
	if (!event.from())	co_return;

	std::string success =
		"User " + data.target_display_name + " (" + data.target_member.get_mention() + ", id: " + std::to_string(data.target_user_id) + ") has been muted successfully for " + std::to_string(data.duration) + "min"
		+ "\nReason : " + data.reason;

	co_await event.co_edit_response(dpp::message(success));
	LOG_PROCESS_SUCCESS("Interaction reply sent");
	co_return;
}



void send_mute_log_message(dpp::cluster& bot, const MuteData& data) {
	dpp::embed embed = dpp::embed()
		.set_title("User Muted")
		.set_description("A user has been muted in the server.")
		.add_field("Muted user",
			data.target_member.get_mention() +
			"\n(" + std::to_string(data.target_user_id) + ")",
			true)
		.add_field("Moderator",
			data.issuer_member.get_mention() +
			"\n" + std::to_string(data.issuer_user_id),
			true)
		.add_field("Reason", data.reason, false)
		.set_color(dpp::colors::orange)
		.set_timestamp(std::time(nullptr));

	dpp::message log_msg;
	log_msg.add_embed(embed);
	log_msg.channel_id = data.command_channel_id;

	bot.message_create(log_msg, [](const dpp::confirmation_callback_t& cb) {
		if (cb.is_error()) {
			LOG_ERROR("Failed to send mute log message: " << cb.get_error().code);
			return;
		}
	});
	LOG_PROCESS_SUCCESS("Mute log message sent successfully");
	return;
}



void fetch_and_set_user_permissions(const dpp::interaction interaction, MuteData& data) {
	// Issuer
	try {
		data.issuer_permissions = interaction.get_resolved_permission(data.issuer_user_id);
	}
	catch (const dpp::exception& e) {
		LOG_WARNING("Failed to get issuer permissions: " << e.what());
	}

	// Target
	try {
		data.target_permissions = interaction.get_resolved_permission(data.target_user_id);
	}
	catch (const dpp::exception& e) {
		LOG_WARNING("Failed to get issuer permissions: " << e.what());
	}

	// App
	data.app_permissions = interaction.app_permissions;
	return;
}
