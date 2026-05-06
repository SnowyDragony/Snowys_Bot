#include "GetBotToken.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

void load_env_file(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue;
        
        auto pos = line.find("=");
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        setenv(key.c_str(), value.c_str(), 0);
    }
}

std::string get_bot_token(const std::string& var_name)
{
    const char* value = getenv(var_name.c_str());
    if (!value)
    {
        throw std::runtime_error("Environment variable \"" + var_name + "\" not found.");
    }
    return std::string(value);
}