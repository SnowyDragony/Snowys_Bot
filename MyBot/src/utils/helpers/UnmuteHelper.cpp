#include "../../../include/utils/helpers/UnmuteHelper.h"
#include "../../../include/utils/ConsoleLogs.h"
#include <dpp/dpp.h>



enum class PermLevel {
	Everyone = 0,
	ModRole = 1,
	Admin = 2,
	Owner = 3
};



CheckResult get_target_member(dpp::interaction command, UnmuteData& data) {
	try {
		data.target_member = command.get_resolved_member(data.target_user_id);
	}
	catch (const dpp::exception& e) {
		LOG_INFO("Target user not resolved as a member: " << e.what());
		return { false, 404, "The specified user is not in the server." };
	}
	return { true, 200, "" };
}



static PermLevel get_permission_level(const UnmuteData& data) {
	if (data.issuer_user_id == data.guild.owner_id)
		return PermLevel::Owner;

	if (data.issuer_permissions.has(dpp::p_administrator))
		return PermLevel::Admin;

	if (data.mod_role_position >= 0 &&
		data.issuer_highest_role_position >= data.mod_role_position)
		return PermLevel::ModRole;

	return PermLevel::Everyone;
}



static CheckResult check_role_hierarchy(dpp::snowflake bot_id, const UnmuteData& data) {
	if (data.app_highest_role_position <= data.target_highest_role_position && bot_id != data.guild.owner_id)
		return { false, 403, "I cannot unmute someone whose role is equal or higher than mine." };

	if (data.target_highest_role_position >= data.issuer_highest_role_position)
		return { false, 403, "You cannot unmute someone whose role is equal or higher than yours." };
	
	if (data.issuer_highest_role_position < data.mod_role_position)
		return { false, 403, "You cannot unmute anyone as your role is lower than the defined moderator role." };

	return { true, 200, "Issuer can mute the target" };
}



CheckResult check_can_unmute(const UnmuteData& data, dpp::snowflake& bot_id) {
	
	// Common checks
	if (data.issuer_user_id != data.guild.owner_id && data.issuer_member.get_roles().empty())	// Block interaction if issuer only have @everyone role
		return { false, 403, "You don't have the required permissions to use this command (no role)." };
	if (!data.issuer_permissions.has(dpp::p_moderate_members))									// Verify if the issuer has the required permissions to mute members
		return { false, 403, "You don't have the required permissions to unmute someone." };
	if (!data.app_permissions.has(dpp::p_moderate_members))										// Verify if the bot has the required permissions to mute members
		return { false, 403, "The bot doesn't have the required permissions to unmute anyone." };
	
	// Target checks
	if (data.target_user_id == data.guild.owner_id)												// Verify if the target is not the server owner
		return { false, 403, "You don't have the required permissions to use this command (command performed on the owner)." };
	if (data.target_permissions.has(dpp::p_administrator))										// Verify if the target is not an admin
		return { false, 403, "You don't have the required permissions to use this Command (command performed on an administrator)." };
	if (data.target_user_id == data.issuer_user_id)												// Verify if the target is the issuer
		return { false, 403, "You don't have the required permissions to use this command (command performed on yourself)." };
	if (data.target_user_id == bot_id)															// Verify if the target is the bot itself
		return { false, 403, "You don't have the required permissions to use this command (command performed on the bot)." };

	// Determination of the permission level of the issuer
	PermLevel level = get_permission_level(data);

	switch (level) {
		case PermLevel::Owner:
			return { true, 200, "Owner performing command, bypassing permission checks" };
		case PermLevel::Admin:
			return { true, 200, "Admin performing command, bypassing permission checks" };
		case PermLevel::ModRole:
			return check_role_hierarchy(bot_id, data);
		case PermLevel::Everyone:
			return { false, 403, "You don't have the required permissions to use this command." };
	}
	return { false, 500, "An internal error occured, please try again later" };
}



dpp::task<void> co_send_unmute_interaction_reply(const dpp::slashcommand_t& event, const UnmuteData& data) {
	if (!event.from())	co_return;

	std::string success =
		"User " + data.target_display_name + " (" + data.target_member.get_mention() + ", id: " + std::to_string(data.target_user_id) + ") has been unmuted successfully"
		+ "\nReason : " + data.reason;

	co_await event.co_edit_response(dpp::message(success));
	LOG_PROCESS_SUCCESS("Interaction reply sent");
	co_return;
}



void send_mute_log_message(dpp::cluster& bot, const UnmuteData& data) {
	dpp::embed embed = dpp::embed()
		.set_title("User Unmuted")
		.set_description("A user has been muted in the server.")
		.add_field("Unmuted user",
			data.target_member.get_mention() +
			"\n(" + std::to_string(data.target_user_id) + ")",
			true)
		.add_field("Moderator",
			data.issuer_member.get_mention() +
			"\n" + std::to_string(data.issuer_user_id),
			true)
		.add_field("Reason", data.reason, false)
		.set_color(dpp::colors::green)
		.set_timestamp(std::time(nullptr));

	dpp::message log_msg;
	log_msg.add_embed(embed);
	log_msg.channel_id = data.command_channel_id;

	bot.message_create(log_msg, [](const dpp::confirmation_callback_t& cb) {
		if (cb.is_error()) {
			LOG_ERROR("Failed to send unmute log message: " << cb.get_error().code);
			return;
		}
		});
	LOG_PROCESS_SUCCESS("Unmute log message sent successfully");
	return;
}



void fetch_and_set_user_permissions(const dpp::interaction interaction, UnmuteData& data) {
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
