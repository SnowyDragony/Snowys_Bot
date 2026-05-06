#pragma once

#ifndef BAN_HELPER_H
#define BAN_HELPER_H

#include "Common.h"
#include <dpp/dpp.h>

struct BanData {													// Struct to hold all relevant datas for the ban command

	bool target_member_is_in_guild		= true;						// Whether the target user is currently a member of the guild. Default value : true

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

CheckResult get_target_member(dpp::interaction command, BanData& data);									// Fetch the guild_member object of the target user and set it in the BanData struct. Return : See CheckResult struct for more details
CheckResult check_can_ban(const BanData& data, dpp::snowflake& bot_id);									// Check if the ban action can be performed based on permissions and other factors. Return : See CheckResult struct for more details
dpp::task<void> co_send_ban_interaction_reply(const dpp::slashcommand_t& event, const BanData& data);	// Edit the original command message with the ban success reply. Return : nothing
void send_ban_log_message(dpp::cluster& bot, const BanData& data);										// Send a log message to the mod-log channel of the server/guild. Return : nothing
void fetch_and_set_user_permissions(const dpp::interaction interaction, BanData& data);					// Fetch the permissions of the issuer, target and bot and set them in the BanData struct. Return : nothing

#endif // !BAN_HELPER_H