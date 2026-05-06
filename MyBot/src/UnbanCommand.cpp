#include "../include/UnbanCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include "../include/utils/helpers/Common.h"
#include "../include/utils/helpers/UnbanHelper.h"

#include <dpp/dpp.h>

dpp::task<void>	handle_unban_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
	LOG_INFO("Handling /unban command");
	bool verbose = true;	// Set to true to enable detailed logs for this command, false to disable them.


	//--------- Getting variables out of event -----------\\


	UnbanData data;
	data.issuer_user_id		= event.command.usr.id;		// Used to get various info (checking permissions, hierarchy, etc.)

	if (verbose) LOG_INFO("Retrieving 'user' parameter");
	data.target_user_id = std::get<dpp::snowflake>(event.get_parameter("user"));

	if (verbose) LOG_INFO("Retrieving 'reason' parameter");
	auto reason_param = event.get_parameter("reason");
	if (std::holds_alternative<std::string>(reason_param)) {
		data.reason = std::get<std::string>(reason_param);
		LOG_INFO("Reason specified : " + data.reason);
	}
	else { if (verbose) LOG_INFO("No reason specified, using default reason"); }

	if (verbose) LOG_INFO("Retrieving guild_member object of the issuer user");
	data.issuer_member = event.command.member;

	if (verbose) LOG_INFO("Retrieving target and bot's permissions");
	fetch_and_set_user_permissions(event.command, data);

	data.issuer_display_name = co_await co_fetch_username(bot, data.issuer_user_id, true);

	// Checking the context of the command (guild only)
	if (verbose) LOG_CHECK("Guild Command Check");
	if (!event.command.guild_id) {
		LOG_INFO("Command not in guild");
		co_await event.co_reply(dpp::message("This command can only be used in a server.").set_flags(dpp::m_ephemeral));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("Command used in guild");



	dpp::async thinking = event.co_thinking(true);
	co_await thinking;



	//--------------- Conditions Checks ------------------\\


	if (verbose) LOG_INFO("Getting guild info");
	data.guild = co_await co_get_guild(bot, event.command.guild_id);
	if (data.guild.id == 0) {
		LOG_WARNING("Failed to retrieve guild info");
		event.edit_response(
			dpp::message("An intern error occured while attempting to get the server informations. Please try again later"));
		co_return;
	}

	if (verbose) LOG_INFO("Is target banned check");
	auto ban_check = co_await co_check_if_user_banned(bot, data.guild.id, data.target_user_id);
	if (!ban_check.success) {
		LOG_CHECK_FAIL("Is target banned check failed: " + ban_check.message);
		event.edit_response(dpp::message(ban_check.message));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("The user is currently banned");

	if (verbose) LOG_INFO("Retrieving issuer role positions");
	data.issuer_highest_role_position = find_highest_role_pos(data.issuer_member);

	if (verbose) LOG_INFO("Retrieving moderator role position from database");
	data.mod_role_position = find_mod_role_pos(data.guild.id);

	if (verbose) LOG_CHECK("Can unban check");
	auto check = check_can_unban(data, bot.me.id);
	if (!check.success) {
		LOG_CHECK_FAIL("Can ban check failed: " + check.message);
		event.edit_response(dpp::message(check.message));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("Issuer can unban");

	
	//---------------- Proceed to unban ------------------\\


	auto unban_callback = co_await bot.set_audit_reason(data.reason).co_guild_ban_delete(data.guild.id, data.target_user_id);
	if (unban_callback.is_error()) {
		LOG_WARNING("Failed to unban user: " + unban_callback.get_error().message);
		event.edit_response(dpp::message("An intern error occured while attempting to unban the user. Please try again later"));
		co_return;
	}

	LOG_SUCCESS("User unbanned successfully");

	LOG_INFO("Sending reply");
	co_await co_send_unban_interaction_reply(event, data);

	LOG_INFO("Sending unban log message");
	data.command_channel_id = find_log_channel_id(data.guild.id, event.command.channel_id);
	send_unban_log_message(bot, data);

	LOG_COMMAND("End of /unban command");
	co_return;
}