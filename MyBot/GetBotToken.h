#pragma once
#ifndef GET_BOT_TOKEN_H
#define GET_BOT_TOKEN_H
#include <string>

void load_env_file(const std::string& filepath = ".env");
std::string get_bot_token(const std::string& var_name);

#endif