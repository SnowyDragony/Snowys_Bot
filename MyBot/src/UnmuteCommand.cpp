#include "../include/UnmuteCommand.h"
#include "../include/utils/ConsoleLogs.h"
#include "../include/utils/helpers/Common.h"
#include "../include/utils/helpers/UnmuteHelper.h"



dpp::task<void>	handle_unmute_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
	LOG_COMMAND("Handling /unmute command");
	
	event.reply(dpp::message("Unmute interaction received successfully. This interaction is currently not available, please try again later."));
	
	co_return;
}