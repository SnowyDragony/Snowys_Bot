#include "../include/MuteCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include "../include/utils/helpers/Common.h"
#include "../include/utils/helpers/MuteHelper.h"

#include <dpp/dpp.h>

dpp::task<void> handle_mute_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
	LOG_COMMAND("Handling /mute command");
	bool verbose = true;	// Set to true to enable detailed logs for this command, false to disable them.


	//--------- Getting variables out of event -----------\\


	MuteData data;
	data.issuer_user_id = event.command.usr.id;

	if (verbose) LOG_INFO("Retrieving 'user' parameter");
	data.target_user_id = std::get<dpp::snowflake>(event.get_parameter("user"));

	if (verbose) LOG_INFO("Retrieving 'reason' parameter");
	auto reason_param = event.get_parameter("reason");
	if (std::holds_alternative<std::string>(reason_param)) {
		data.reason = std::get<std::string>(reason_param);
		if (verbose) LOG_INFO("Reason specified: " + data.reason);
	}
	else { if (verbose)LOG_INFO("No reason specified, using default reason"); }

	if (verbose) LOG_INFO("Retrieving 'duration' parameter");
	auto duration_param = event.get_parameter("duration");
	if (std::holds_alternative<int64_t>(duration_param)) {
		data.duration = static_cast<int>(std::get<int64_t>(duration_param));
		if (verbose) LOG_INFO("Time specified (int64_t), muting for " + std::to_string(data.duration) + " minutes");
	}
	else if (std::holds_alternative<dpp::snowflake>(duration_param)) {
		data.duration = static_cast<int>(std::get<dpp::snowflake>(duration_param));
		if (verbose) LOG_INFO("Time specified (snowflake), muting for " + std::to_string(data.duration) + " minutes");
	}
	else {
		if (verbose) LOG_INFO("No time specified, muting for 10 minutes");
	}
	data.until = time(nullptr) + (data.duration * 60);

	if (verbose) LOG_INFO("Retrieving guild_member objects");
	data.issuer_member = event.command.member;
	data.app_member = event.command.get_resolved_member(bot.me.id);
	auto target_member_check = get_target_member(event.command, data);
	if (!target_member_check.success) {
		event.reply(dpp::message(target_member_check.message).set_flags(dpp::m_ephemeral));
		co_return;
	}

	if (verbose) LOG_INFO("Retrieving target, issuer and bot's permissions");
	fetch_and_set_user_permissions(event.command, data);

	data.target_display_name = co_await co_fetch_username(bot, data.target_user_id, true);
	data.issuer_display_name = co_await co_fetch_username(bot, data.issuer_user_id, true);

	// Checking the context of the command (guild only)
	if (verbose) LOG_CHECK("Guild Command Check");
	if (!event.command.guild_id) {
		LOG_INFO("Command not used in a server");
		event.reply(
			dpp::message("This command can only be used in a server.").set_flags(dpp::m_ephemeral));
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
			dpp::message("An internal error occured while attempting to get server info. Please try again later."));
		co_return;
	}

	if (verbose) LOG_CHECK("Can mute check");
	auto mute_check = check_can_mute(data, bot.me.id);
	if (!mute_check.success) {
		LOG_CHECK_FAIL("Can mute check failed: " + mute_check.message);
		event.edit_response(dpp::message(mute_check.message));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("Issuer can mute");

	if (verbose) LOG_INFO("Retrieving role hierarchy");
	data.issuer_highest_role_position = find_highest_role_pos(data.issuer_member);
	data.target_highest_role_position = find_highest_role_pos(data.target_member);
	data.app_highest_role_position = find_highest_role_pos(data.app_member);

	if (verbose) LOG_INFO("Retrieving moderator role position from database");
	data.mod_role_position = find_mod_role_pos(data.guild.id);

	if (verbose) LOG_CHECK("Role hierarchy check");
	auto role_hierarchy_check = check_role_hierarchy(bot.me.id, data);
	if (!role_hierarchy_check.success) {
		LOG_CHECK_FAIL("Role hierarchy check failed: " + role_hierarchy_check.message);
		event.edit_response(dpp::message(role_hierarchy_check.message));
		co_return;
	}	if (verbose) LOG_CHECK_SUCCESS("Role hierarchy check passed: " + role_hierarchy_check.message);


	//----------------- Proceed to mute ------------------\\


	auto mute_callback = co_await bot.set_audit_reason(data.reason).co_guild_member_timeout(data.guild.id, data.target_user_id, data.until);
	if (mute_callback.is_error()) {
		LOG_WARNING("Failed to mute the user: " + mute_callback.get_error().message);
		event.edit_response(
			dpp::message("Failed to mute the user: " + mute_callback.get_error().message));
		co_return;
	}

	LOG_SUCCESS("User muted successfully");

	LOG_INFO("Sending Mute Reply");
	co_await co_send_mute_interaction_reply(event, data);

	LOG_INFO("Sending mute log message");
	send_mute_log_message(bot, data);

	LOG_COMMAND("End of /mute command");
	co_return;
}