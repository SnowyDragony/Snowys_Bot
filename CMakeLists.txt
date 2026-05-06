cmake_minimum_required(VERSION 3.22)

project(Snowys_Bot LANGUAGES CXX)

# ====================== Configuration C++ ======================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================== VCPKG =============================
if(DEFINED ENV{VCPKG_ROOT})
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(DEFINED VCPKG_ROOT) 
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

set(VCPKG_BUILD_TYPE release)

# =========================== CMAKE ============================
set(CMAKE_PREFIX_PATH "/usr/local" ${CMAKE_PREFIX_PATH})

# ======================== Dependencies =========================
find_package(dpp CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# =========================== Sources ===========================

add_executable(Snowys_Bot
	MyBot/Snowys_Bot.cpp 
	MyBot/GetBotToken.cpp

	MyBot/src/BanCommand.cpp
	MyBot/src/DiceCommand.cpp
	MyBot/src/MuteCommand.cpp
	MyBot/src/PingCommand.cpp
	MyBot/src/SendCommand.cpp
	MyBot/src/SettingsCommand.cpp
	MyBot/src/UnbanCommand.cpp
	MyBot/src/UnmuteCommand.cpp

	MyBot/src/utils/helpers/BanHelper.cpp
	MyBot/src/utils/helpers/Common.cpp
	MyBot/src/utils/helpers/DiceHelper.cpp
	MyBot/src/utils/helpers/MuteHelper.cpp
	MyBot/src/utils/helpers/UnbanHelper.cpp
	MyBot/src/utils/helpers/UnmuteHelper.cpp
		
	MyBot/db/BotDatabase.cpp
)

# =========================== Includes ==========================

target_include_directories(Snowys_Bot PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/MyBot/include
	${CMAKE_CURRENT_SOURCE_DIR}/MyBot/include/utils
	${CMAKE_CURRENT_SOURCE_DIR}/MyBot/include/utils/helpers
	${CMAKE_CURRENT_SOURCE_DIR}/MyBot/db
)

# ========================== Libraries ==========================

target_link_libraries(Snowys_Bot PRIVATE
	dpp::dpp
	unofficial::sqlite3::sqlite3
	SQLiteCpp
	OpenSSL::SSL
	OpenSSL::Crypto
)

# ========================= Compilation =========================
target_compile_definitions(Snowys_Bot PRIVATE
	PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

if(MSVC)
	target_compile_options(Snowys_Bot PRIVATE /W4 /MP)
else()
	target_compile_options(Snowys_Bot PRIVATE -Wall -Wextra -Wpedantic)
endif()

