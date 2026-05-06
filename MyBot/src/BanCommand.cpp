#include "../include/BanCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include "../include/utils/helpers/Common.h"
#include "../include/utils/helpers/BanHelper.h"
#pragma warning(disable : 26811)

#include <dpp/dpp.h>

dpp::task<void> handle_ban_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
	LOG_INFO("Handling /ban command");
	bool verbose = true;	// Set to true to enable detailed logs for this command, false to disable them.


	//--------- Getting variables out of event -----------\\


	BanData data;
	data.issuer_user_id		= event.command.usr.id;		// Used to get various info (checking permissions, hierarchy, etc.)

	if (verbose) LOG_INFO("Retrieving 'user' parameter");
	data.target_user_id = std::get<dpp::snowflake>(event.get_parameter("user"));

	if (verbose) LOG_INFO("Retrieving 'reason' parameter");
	auto reason_param = event.get_parameter("reason");
	if (std::holds_alternative<std::string>(reason_param)) {	// If reason_param is of string type, get reason, else use default reason
		data.reason = std::get<std::string>(reason_param);
		LOG_INFO("Reason specified : " + data.reason);
	}
	else { if (verbose) LOG_INFO("No reason specified, using default reason"); }

	if (verbose) LOG_INFO("Retrieving guild_member objects");
	data.issuer_member = event.command.member;
	auto target_member_check = get_target_member(event.command, data);
	if (!target_member_check.success) {
		event.reply(dpp::message(target_member_check.message).set_flags(dpp::m_ephemeral));
		data.target_member_is_in_guild = false;
	}

	if (verbose) LOG_INFO("Retrieving target, issuer and bot's permissions");
	fetch_and_set_user_permissions(event.command, data); 

	data.target_display_name = co_await co_fetch_username(bot, data.target_user_id, data.target_member_is_in_guild);
	data.issuer_display_name = co_await co_fetch_username(bot, data.issuer_user_id, true); // Issuer must be in the server to perform the command, so true

	// Checking the context of the command (guild only)
	if (verbose) LOG_CHECK("Guild Command Check");
	if (!event.command.guild_id) {
		LOG_INFO("Command not in guild");
		event.reply(
			dpp::message("This command can only be used in a server.").set_flags(dpp::m_ephemeral));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("Command used in guild");



	dpp::async thinking = event.co_thinking(true); // Entering thinking state allows us 15mins to reply
	co_await thinking; // Wait discord's confirmation of the thinking state before continuing



	//--------------- Conditions Checks ------------------\\


	if (verbose) LOG_INFO("Getting guild info")
	data.guild = co_await co_get_guild(bot, event.command.guild_id);
	if (data.guild.id == 0) {
		event.edit_response(
			dpp::message("An intern error occured while attempting to get server informations. Please try again later."));
		co_return;
	}

	if (verbose) LOG_CHECK("Is target banned check");
	auto ban_check = co_await co_check_if_user_banned(bot, data.guild.id, data.target_user_id);
	if (ban_check.err_code != 404) {
		LOG_CHECK_FAIL("Is target banned check failed: " + ban_check.message);
		event.edit_response(dpp::message(ban_check.message));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("The user is not currently banned");

	if (verbose) LOG_INFO("Retrieving role hierarchy");
	data.issuer_highest_role_position = find_highest_role_pos(data.issuer_member);
	if (data.target_member_is_in_guild)	data.target_highest_role_position = find_highest_role_pos(data.target_member);
	data.app_highest_role_position = find_highest_role_pos(data.app_member);

	if (verbose) LOG_INFO("Retrieving moderator role position from database");
	data.mod_role_position = find_mod_role_pos(data.guild.id);

	if (verbose) LOG_CHECK("Can ban check");
	auto check = check_can_ban(data, bot.me.id);
	if (!check.success) {
		LOG_CHECK_FAIL("Can ban check failed: " + check.message);
		event.edit_response(dpp::message(check.message));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("Issuer can ban");
	

	//----------------- Proceed to ban -------------------\\


	auto ban_callback = co_await bot.set_audit_reason(data.reason).co_guild_ban_add(data.guild.id, data.target_user_id, 0);
	if (ban_callback.is_error()) {
		LOG_ERROR("Failed to ban the user: " << ban_callback.get_error().message << "(code: " << ban_callback.get_error().code << ")");
		event.edit_response(
			dpp::message("Failed to ban the user: " + ban_callback.get_error().message));
		co_return;
	}

	LOG_SUCCESS("User banned successfully");

	LOG_INFO("Sending reply");
	co_await co_send_ban_interaction_reply(event, data);

	LOG_INFO("Sending ban log message");
	data.command_channel_id = find_log_channel_id(data.guild.id, event.command.channel_id);
	send_ban_log_message(bot, data);

	LOG_COMMAND("End of /ban command");
	co_return;
}