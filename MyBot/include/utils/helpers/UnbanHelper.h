#pragma once

#ifndef UNBAN_HELPER_H
#define UNBAN_HELPER_H

#include <dpp/dpp.h>
#include "Common.h"

struct UnbanData {													// Struct to hold all relevant datas for the unban command

	std::string reason					= "No specified reasons";	// Reason for the mute, default value is "No specified reasons"
	std::string issuer_display_name		= "";						// Display name of the issuer, default value is empty string

	dpp::snowflake issuer_user_id		= 0;						// Issuer user ID. Default value : 0 = empty/invalid
	dpp::snowflake target_user_id		= 0;						// Target user ID. Default value : 0 = empty/invalid
	dpp::snowflake command_channel_id	= 0;						// Channel ID of the channel the command was executed in. Default value : 0 = empty/invalid

	dpp::guild guild					= dpp::guild();				// Guild object of the server the command was executed in. Default value : empty/invalid

	dpp::guild_member issuer_member		= dpp::guild_member();		// Guild member object of the issuer. Default value : empty/invalid

	dpp::permission issuer_permissions	= 0;						// Permission ID of the issuer. This ID is a map of all of the user's permissions. Default value : 0 = no permissions
	dpp::permission app_permissions		= 0;						// Permission ID of the bot/application. This ID is a map of all of the user's permissions. Default value : 0 = no permissions

	int issuer_highest_role_position	= -1;						// Target's highest role position in the role hierarchy of the server/guild. 0 is for @everyone. Default value : -1 = empty/invalid
	int mod_role_position				= -1;						// Moderator role's position in the role hierarchy of the server/guild. 0 is for @everyone. Default value : -1 = empty/invalid
};

CheckResult check_can_unban(const UnbanData& data, dpp::snowflake& bot_id);									// Check if the unban action can be performed based on permissions and other factors. Return : See CheckResult struct for more details
CheckResult check_role_hierarchy(const UnbanData& data);													// Check the role hierarchy between the bot and the issuer to see if the unban can be performed. Return : See CheckResult struct for more details
dpp::task<void> co_send_unban_interaction_reply(const dpp::slashcommand_t& event, const UnbanData& data);	// Edit the original command message with the unban success reply. Return : nothing
void send_unban_log_message(dpp::cluster& bot, const UnbanData& data);										// Send a log message to the mod-log channel of the server/guild. Return : nothing
void fetch_and_set_user_permissions(const dpp::interaction interaction, UnbanData& data);					// Fetch the permissions of the user who issued the command and set it in the UnbanData struct. Return : nothing

#endif