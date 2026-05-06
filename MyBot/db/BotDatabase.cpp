// Contournement du bug vcpkg pour sqlitecpp :
namespace SQLite {
	constexpr int OPEN_READONLY		= 0x00000001;
	constexpr int OPEN_READWRITE	= 0x00000002;
	constexpr int OPEN_CREATE		= 0x00000004;
	constexpr int OPEN_URI			= 0x00000040;
	constexpr int OPEN_MEMORY		= 0x00000080;
	constexpr int OPEN_NOMUTEX		= 0x00008000;
	constexpr int OPEN_FULLMUTEX	= 0x00010000;
	constexpr int OPEN_SHAREDCACHE	= 0x00020000;
	constexpr int OPEN_PRIVATECACHE	= 0x00040000;
} // constexpr = valeur calculee � la compilation, const = valeur constante � l'ex�cution

#include "BotDatabase.h"
#include "../include/utils/ConsoleLogs.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <dpp/snowflake.h>

#include <string>
#include <memory>
#include <mutex>
#include <cstdint>



std::unique_ptr<Database> Database::instance = nullptr;
std::once_flag Database::init_flag;


Database::Database(const std::string& db_path) : db(nullptr) {

	// Initialisation de la base de donn�es
	try {
		// Ouvre ou cr�e le fichier db (� l'emplacement db_path), et cr�e l'objet Database accessible dans le code
		db = std::make_unique<SQLite::Database>(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		
		// PRAGMA : cl�s de la db pour une meilleure int�grit� et gestion
		// - WAL : permet des lecteurs concurrents avec des �critures
		// - busy_timeout : diminue les erreurs "database is locked" en attendant avant d'abandonner une op�ration
		// - synchronous = NORMAL : �quilibre entre performance et s�curit� des donn�es
		db->exec("PRAGMA journal_mode = WAL;");
		db->exec("PRAGMA busy_timeout = 5000;"); // 5000 ms
		db->exec("PRAGMA synchronous = NORMAL;");
		db->exec("PRAGMA foreign_keys = ON;");
		
		// Cr�e la table si elle n'existe pas
		db->exec("CREATE TABLE IF NOT EXISTS bot_guild_info ("
			"guild_id TEXT PRIMARY KEY, "
			"channel_id INTEGER DEFAULT 0, "
			"mod_role_id INTEGER DEFAULT 0);");
	}
	catch (const SQLite::Exception& e) {
		LOG_ERROR("Erreur lors de la cr�ation des tables de la base de donn�es : " << e.what());
		// db reste nullptr si l'ouverture ou la cr�ation a �chou�
	}
}

/*
	Mise � jour de la base de donn�es
	� ex�cuter une seule fois pour migrer si la base de donn�es existante contient des donn�es
	db->exec("ALTER TABLE bot_guild_info RENAME TO bot_guild_info_old;");
	db->exec("CREATE TABLE bot_guild_info (guild_id TEXT PRIMARY KEY, channel_id INTEGER DEFAULT 0, mod_role_id INTEGER DEFAULT 0);");
	db->exec("INSERT INTO bot_guild_info (guild_id, channel_id, mod_role_id) SELECT guild_id, channel_id, mod_role_id FROM bot_guild_info_old;");
	db->exec("DROP TABLE bot_guild_info_old;");
*/




Database::~Database() {
	// Le pointeur unique g�re automatiquement la lib�ration des ressources
	db.reset();
}



Database* Database::get_instance() {
	// std::call_once garantit que l'initialisation n'est faite qu'une seule fois, m�me depuis plusieurs threads
	std::call_once(init_flag, [&]() {
		instance.reset(new Database(PROJECT_ROOT "/MyBot/db/bot_guild_info.db"));
	});

	return instance.get();
}



void Database::set_log_channel(dpp::snowflake guild_id, dpp::snowflake channel_id) {
	if (!db) {
		LOG_ERROR("set_log_channel: Base de donn�es non initialisee.");
		return;
	}
	
	std::lock_guard<std::mutex> lock(db_mutex);

	try {
		// ON CONFLICT DO UPDATE �vite de DELETE + INSERT une nouvelle ligne
		SQLite::Statement query(*db,
			"INSERT INTO bot_guild_info (guild_id, channel_id) "
			"VALUES (?, ?) "
			"ON CONFLICT(guild_id) DO UPDATE SET channel_id = excluded.channel_id;");
		// Si guild_id d�j� pr�sent dans db alors ne pas cr�er de ligne et update channel_id
		query.bind(1, std::to_string(static_cast<uint64_t>(guild_id)));
		query.bind(2, static_cast<int64_t>(channel_id));
		query.exec();
	}
	catch (const SQLite::Exception& e) {
		LOG_ERROR("Erreur lors de la sauvegarde du channel de logs : " << e.what());
	}
}



dpp::snowflake Database::get_log_channel(dpp::snowflake guild_id) {
	if (!db) {
		LOG_ERROR("get_log_channel: Base de donn�es non initialisee.");
		return 0;
	}

	std::lock_guard<std::mutex> lock(db_mutex);

	try {
		SQLite::Statement query(*db,
			"SELECT channel_id "
			"FROM bot_guild_info "
			"WHERE guild_id = ?;");
		query.bind(1, std::to_string(guild_id));
		if (query.executeStep()) {
			return static_cast<dpp::snowflake>(query.getColumn(0).getInt64());
		}
	}
	catch (const SQLite::Exception& e) {
		LOG_ERROR("Erreur lors de la r�cup�ration du channel de logs : " << e.what());
	}
	return 0; // Retourne 0 si aucun channel n'est d�fini
}



void Database::set_mod_role(dpp::snowflake guild_id, dpp::snowflake role_id) {
	if (!db) {
		LOG_ERROR("set_log_channel: Base de donn�es non initialisee.");
		return;
	}

	std::lock_guard<std::mutex> lock(db_mutex);

	try {
		SQLite::Statement query(*db,
			"INSERT INTO bot_guild_info (guild_id, mod_role_id) "
			"VALUES (?, ?) "
			"ON CONFLICT(guild_id) DO UPDATE SET mod_role_id = excluded.mod_role_id;");
		query.bind(1, std::to_string(static_cast<uint64_t>(guild_id)));
		query.bind(2, static_cast<int64_t>(role_id));
		query.exec();
	}
	catch (const SQLite::Exception& e) {
		LOG_ERROR("Erreur lors de la sauvegarde du role moderateur : " << e.what());
	}
}

dpp::snowflake Database::get_mod_role(dpp::snowflake guild_id) {
	if (!db) {
		LOG_ERROR("get_log_channel: Base de donn�es non initialisee.");
		return 0;
	}

	std::lock_guard<std::mutex> lock(db_mutex);

	try {
		SQLite::Statement query(*db,
			"SELECT mod_role_id "
			"FROM bot_guild_info "
			"WHERE guild_id = ?;");
		query.bind(1, std::to_string(guild_id));
		if (query.executeStep()) {
			return static_cast<dpp::snowflake>(query.getColumn(0).getInt64());
		}
	}
	catch (const SQLite::Exception& e) {
		LOG_ERROR("Erreur lors de la r�cup�ration du role mod�rateur : " << e.what());
	}
	return 0; // Retourne 0 si aucun r�le n'est d�fini
}