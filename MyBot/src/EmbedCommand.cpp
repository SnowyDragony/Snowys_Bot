#include "../include/EmbedCommand.h"
#include "../include/utils/ConsoleLogs.h"   
#include <dpp/dpp.h>
#include <thread>
#include <chrono>

dpp::task<void> handle_embed_command(const dpp::slashcommand_t& event, dpp::cluster& bot) {
    // copie pour réponse asynchrone
    LOG_INFO("Handling /embed command");

    dpp::async thinking = event.co_thinking(true);
    co_await thinking;

    LOG_INFO("Création du thread différé");
    // travail long dans un thread séparé (ici 15 minutes)
    std::thread([event, &bot]() mutable {
        LOG_INFO("Sleep for 20s");
        std::this_thread::sleep_for(std::chrono::seconds(10));

        LOG_INFO("Sleep ended");
        dpp::embed e;
        e.set_title("Réponse retardée");
        e.set_description("Ceci est un embed envoyé après 10s de traitement.");
        dpp::message m;
        m.add_embed(e);

        try {
            LOG_INFO("Envoi du message");
            event.edit_response(m);
        }
        catch (const std::exception& ex) {
            LOG_DEBUG("Fallback reply failed: " << ex.what());
        }
    }).detach();
    co_return;
}