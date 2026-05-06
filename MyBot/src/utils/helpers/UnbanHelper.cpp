#include "../../../include/utils/helpers/UnbanHelper.h"
#include "../../../include/utils/ConsoleLogs.h"
#include <dpp/dpp.h>



enum class PermLevel {
	Everyone = 0,
	ModRole = 1,
	Admin = 2,
	Owner = 3
};



CheckResult check_role_hierarchy(const UnbanData& data) {
	if (data.issuer_highest_role_position < data.mod_role_position)
		return { false, 403, "You cannot unban anyone as your role is lower than the defined moderator role." };
	
	return { true, 200, "Issuer can unban the target" };
}



static PermLevel get_permission_level(const UnbanData& data) {
	if (data.issuer_user_id == data.guild.owner_id)
		return PermLevel::Owner;

	if (data.issuer_permissions.has(dpp::p_administrator))
		return PermLevel::Admin;

	if (data.mod_role_position >= 0 &&
		data.issuer_highest_role_position >= data.mod_role_position)
		return PermLevel::ModRole;

	return PermLevel::Everyone;
}


CheckResult check_can_unban(const UnbanData& data, dpp::snowflake& bot_id) {

	// Common checks
	if (data.issuer_user_id != data.guild.owner_id && data.issuer_member.get_roles().empty())		// Block interaction if issuer only have @everyone role
		return { false, 403, "You don't have the required permissions to use this command (no role)." };
	if (!data.issuer_permissions.has(dpp::p_ban_members))											// Verify if the issuer has the required permissions to unban members
		return { false, 403, "You don't have the required permissions to unban someone." };
	if (!data.app_permissions.has(dpp::p_ban_members))												// Verify if the bot has the required permissions to unban members
		return { false, 403, "The bot doesn't have the required permissions to unban anyone." };
	
	// Determination of the permission level of the issuer
	PermLevel level = get_permission_level(data);

	switch (level) {
	case PermLevel::Owner:
		return { true, 200, "Owner performing command, bypassing permission checks" };
	case PermLevel::Admin:
		return { true, 200, "Admin performing command, bypassing permission checks" };
	case PermLevel::ModRole:
		return check_role_hierarchy(data);
	case PermLevel::Everyone:
		return { false, 403, "You don't have the required permissions to use this command." };
	}
	return { false, 500, "An internal error occured, please try again later" };
}



dpp::task<void> co_send_unban_interaction_reply(const dpp::slashcommand_t& event, const UnbanData& data) {
	if (!event.from())	co_return;

	std::string success =
		"User <@" + std::to_string(data.target_user_id) + ">, (id: " + std::to_string(data.target_user_id) + ") has been unbanned successfully."
		+ "\nReason : " + data.reason;

	co_await event.co_edit_response(dpp::message(success));
	LOG_PROCESS_SUCCESS("Interaction reply sent");
	co_return;
}



void send_unban_log_message(dpp::cluster& bot, const UnbanData& data) {
	dpp::embed embed = dpp::embed()
		.set_title("User Unbanned")
		.set_description("A user has been unbanned from the server.")
		.add_field("Unbanned User",
			"<@" + std::to_string(data.target_user_id) + "> "
			"\n(" + std::to_string(data.target_user_id) + ")",
			true)
		.add_field("Moderator",
			data.issuer_member.get_mention() +
			"\n(" + std::to_string(data.issuer_user_id) + ")",
			true)
		.add_field("Reason", data.reason, false)
		.set_color(dpp::colors::green)
		.set_timestamp(std::time(nullptr));

	dpp::message log_msg;
	log_msg.add_embed(embed);
	log_msg.channel_id = data.command_channel_id;

	bot.message_create(log_msg, [](const dpp::confirmation_callback_t& cb) {
		if (cb.is_error()) {
			LOG_ERROR("Failed to send unban log message: " << cb.get_error().code);
		}
	});
	LOG_PROCESS_SUCCESS("Unban log message sent successfully");
	return;
}



void fetch_and_set_user_permissions(const dpp::interaction interaction, UnbanData& data) {
	try {
		data.issuer_permissions = interaction.get_resolved_permission(data.issuer_user_id);
	}
	catch (const dpp::exception& e) {
		LOG_ERROR("Failed to get issuer permissions: " << e.what());
	}
	data.app_permissions = interaction.app_permissions;
	return;
}