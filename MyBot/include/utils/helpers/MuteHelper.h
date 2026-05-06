#pragma once

#ifndef MUTE_HELPER_H
#define MUTE_HELPER_H

#include "Common.h"
#include <dpp/dpp.h>

struct MuteData {													// Struct to hold all relevant datas for the mute command

	int duration						= 10;						// Duration of the mute in minutes. Default value : 10min
	time_t until						= 0;						// Unix timestamp of when the mute will expire. Default value : 0 = empty/invalid

	std::string reason					= "No specified reasons";	// Reason for the mute, default value is "No specified reasons"
	std::string issuer_display_name		= "";						// Display name of the issuer, default value is empty string
	std::string target_display_name		= "";						// Display name of the target, default value is empty string

	dpp::snowflake issuer_user_id		= 0;						// Issuer user ID. Default value : 0 = empty/invalid
	dpp::snowflake target_user_id		= 0;						// Target user ID. Default value : 0 = empty/invalid
	dpp::snowflake command_channel_id	= 0;						// Channel ID of the channel the command was executed in. Default value : 0 = empty/invalid

	dpp::guild guild					= dpp::guild();				// Guild object of the server the command was executed in. Default value : empty/invalid

	dpp::guild_member issuer_member		= dpp::guild_member();		// Guild member object of the issuer. Default value : empty/invalid
	dpp::guild_member target_member		= dpp::guild_member();		// Guild member object of the target. Default value : empty/invalid
	dpp::guild_member app_member		= dpp::guild_member();		// Guild member object of the bot/application. Default value : empty/invalid

	dpp::permission issuer_permissions	= 0;						// Permission ID of the issuer. This ID is a map of all of the user's permissions. Default value : 0 = no permissions
	dpp::permission target_permissions	= 0;						// Permission ID of the target. This ID is a map of all of the user's permissions. Default value : 0 = no permissions
	dpp::permission app_permissions		= 0;						// Permission ID of the bot/application. This ID is a map of all of the user's permissions. Default value : 0 = no permissions

	int target_highest_role_position	= -1;						// Issuer's highest role position in the role hierarchy of the server/guild. 0 is for @everyone. Default value : -1 = empty/invalid
	int issuer_highest_role_position	= -1;						// Target's highest role position in the role hierarchy of the server/guild. 0 is for @everyone. Default value : -1 = empty/invalid
	int app_highest_role_position		= -1;						// Bot/Application's highest role position in the role hierarchy of the server/guild. 0 is for @everyone. Default value : -1 = empty/invalid
	int mod_role_position				= -1;						// Moderator role's position in the role hierarchy of the server/guild. 0 is for @everyone. Default value : -1 = empty/invalid
};

CheckResult get_target_member(dpp::interaction command, MuteData& data);								// Fetch the guild_member object of the target user and set it in the MuteData struct. Return : See CheckResult struct for more details
CheckResult check_can_mute(const MuteData& data, dpp::snowflake& bot_id);								// Check if the mute action can be performed based on permissions and other factors. Return : See CheckResult struct for more details
CheckResult check_role_hierarchy(dpp::snowflake bot_id, const MuteData& data);							// Check the role hierarchy between the bot, the issuer and the target to see if the mute can be performed. Return : See CheckResult struct for more details
dpp::task<void> co_send_mute_interaction_reply(const dpp::slashcommand_t& event, const MuteData& data);	// Edit the original command message with the mute success reply. Return : nothing
void send_mute_log_message(dpp::cluster& bot, const MuteData& data);									// Send a log message in the defined log channel with the details of the mute. Return : nothing
void fetch_and_set_user_permissions(const dpp::interaction interaction, MuteData& data);				// Fetch the permissions of the issuer, target and bot and set them in the MuteData struct. Return : nothing

#endif //MUTE_HELPER_H