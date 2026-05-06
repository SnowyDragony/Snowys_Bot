#pragma once
#ifndef DATABASE_H
#define DATABASE_H

#include <SQLiteCpp/Database.h>

#include <dpp/snowflake.h>

#include <string>
#include <mutex>
#include <memory>

class Database {
private :
	static std::unique_ptr<Database>	instance;	// Instance unique de la classe, unique_ptr pour la destruction automatique
	static std::once_flag				init_flag;	// Assure l'initialisation thread-safe (initiation une seule fois)
	std::unique_ptr<SQLite::Database>	db;			// Pointeur vers la base de données SQLite
	std::mutex							db_mutex;	// Mutex pour la synchronisation des accès à la base de données (opérations internes)

	// Constructeur privé pour empêcher l'instanciation directe
	Database(const std::string& db_path);

	// Empecher la copie et l'assignation
	Database(const Database&) = delete;
	Database& operator=(const Database&) = delete;

public :
	~Database();

	// Méthode pour obtenir l'instance unique
	// db_path fixé à "MyBot/db/bot_guild_info.db"
	static Database* get_instance();

	// Méthode pour sauvegarder l'ID du channel de logs (par serveur/guild)
	void set_log_channel(dpp::snowflake guild_id, dpp::snowflake channel_id);

	// Méthode pour récupérer l'ID du channel de logs (par serveur/guild)
	// Renvoie 0 si aucune entrée n'est trouvée
	dpp::snowflake get_log_channel(dpp::snowflake guild_id);

	// Méthode pour sauvegarder l'ID du rôle de modérateur (par serveur/guild)
	void set_mod_role(dpp::snowflake guild_id, dpp::snowflake role_id);

	// Méthode pour récupérer l'ID du rôle de modérateur (par serveur/guild)
	// Renvoie 0 si aucune entrée n'est trouvée
	dpp::snowflake get_mod_role(dpp::snowflake guild_id);

};

#endif // !DATABASE_H