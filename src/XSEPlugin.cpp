#include "managers/animation/Utils/CooldownManager.hpp"
#include "hooks/Experiments.hpp"

#include "Config.hpp"
#include "hooks/hooks.hpp"
#include "papyrus/papyrus.hpp"
#include "data/plugin.hpp"

#include "events.hpp"
#include "managers/register.hpp"
#include "managers/InputManager.hpp"
#include "UI/DebugAPI.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "spring.hpp"

#include <stddef.h>
#include <thread>
#include "git.h"

#include "skselog.hpp"
#include "api/APIManager.hpp"

using namespace RE::BSScript;
using namespace Gts;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {

	void PrintStartupBanner() {
		Cprint("[GTSPlugin.dll]: [ Giantess Mod v2.00 Beta was succesfully initialized. Waiting for New Game/Save Load. ]");
		Cprint("[GTSPlugin.dll]: Dll Build Date: {} {}", __DATE__, __TIME__);
		Cprint("[GTSPlugin.dll]: Git Info:");
		Cprint("\t -- Commit: {}", git_CommitSubject());
		Cprint("\t -- SHA1: {}", git_CommitSHA1());
		Cprint("\t -- Date: {}", git_CommitDate());
		Cprint("\t -- LocalChanges: {}", git_AnyUncommittedChanges() ? "Yes" : "No");
	}

	void InitializeLogging() {
		auto path = Gts::log_directory();

		if (!path) {
			report_and_fail("Unable to lookup SKSE logs directory.");
		}
		*path /= PluginDeclaration::GetSingleton()->GetName();
		*path += L".log";

		std::shared_ptr <spdlog::logger> log;

		if (IsDebuggerPresent()) {
			log = std::make_shared <spdlog::logger>(
				"Global", std::make_shared <spdlog::sinks::msvc_sink_mt>());
		} else {
			log = std::make_shared <spdlog::logger>(
				"Global", std::make_shared <spdlog::sinks::basic_file_sink_mt>(path->string(), true));
		}

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%l] [%s:%#] %v");
		spdlog::set_level(spdlog::level::level_enum::trace);
		spdlog::flush_on(spdlog::level::level_enum::trace);

	}

	void InitializeMessaging() {

		if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message *message) {
			switch (message->type) {

				// Called after all plugins have finished running SKSEPlugin_Load.
				case MessagingInterface::kPostLoad:     
				{ 
					//RegisterAPIs();
					break;
				}

				// Called after all kPostLoad message handlers have run.
				case MessagingInterface::kPostPostLoad: 
				{ 
					break;
				}

				// Called when all game data has been found.
				case MessagingInterface::kInputLoaded:   
				{
					break;
				}

				// All ESM/ESL/ESP plugins have loaded, main menu is now active.
				case MessagingInterface::kDataLoaded:  
				{    
					//Hooks::Hook_Experiments::PatchShaking();
					EventDispatcher::DoDataReady();
					InputManager::GetSingleton().DataReady();
					RegisterAPIs();
					break;
				}

				// Skyrim game events.
				// Player's selected save game has finished loading.
				case MessagingInterface::kPostLoadGame:  
				{
					Plugin::SetInGame(true);
					Cprint(" [GTSPlugin.dll]: [ Giantess Mod was succesfully initialized and loaded. ]");
					break;
				}

				// Player starts a new game from main menu.
				case MessagingInterface::kNewGame:
				{
					Plugin::SetInGame(true);
					EventDispatcher::DoReset();
					Cprint(" [GTSPlugin.dll]: [ Giantess Mod was succesfully initialized and loaded. ]");
					break;
				}

				// Player selected a game to load, but it hasn't loaded yet.
				// Data will be the name of the loaded save.
				case MessagingInterface::kPreLoadGame: 
				{
					Plugin::SetInGame(false);
					EventDispatcher::DoReset();
					break;
				}

				// The player has saved a game.
				case MessagingInterface::kSaveGame:
				{
					break;
				}

				// The player deleted a saved game from within the load menu.
				// Data will be the save name.
				case MessagingInterface::kDeleteGame:
				{
					break;
				} 

			}
		})) {
			stl::report_and_fail("Unable to register message listener.");
		}
	}
}

static void InitializeSerialization() {
	log::trace("Initializing cosave serialization...");
	auto* serde = GetSerializationInterface();
	serde->SetUniqueID(_byteswap_ulong('GTSP'));
	serde->SetSaveCallback(Persistent::OnGameSaved);
	serde->SetRevertCallback(Persistent::OnRevert);
	serde->SetLoadCallback(Persistent::OnGameLoaded);
	log::info("Cosave serialization initialized.");
}

static void InitializePapyrus() {
	log::trace("Initializing Papyrus binding...");
	if (GetPapyrusInterface()->Register(Gts::register_papyrus)) {
		log::info("Papyrus functions bound.");
	} else {
		stl::report_and_fail("Failure to register Papyrus bindings.");
	}
}

static void InitializeEventSystem() {
	EventDispatcher::AddListener(&DebugOverlayMenu::GetSingleton());
	EventDispatcher::AddListener(&Runtime::GetSingleton()); // Stores spells, globals and other important data
	EventDispatcher::AddListener(&Persistent::GetSingleton());
	EventDispatcher::AddListener(&Transient::GetSingleton());
	EventDispatcher::AddListener(&CooldownManager::GetSingleton());

	EventDispatcher::AddListener(&TaskManager::GetSingleton());
	EventDispatcher::AddListener(&SpringManager::GetSingleton());
	log::info("Adding Default Listeners");
	RegisterManagers();
}

static void PrintPluginInfo() {

	logger::info("SKSEPluginLoad... ");
	logger::info("Dll Build Date: {} {}", __DATE__, __TIME__);

	std::string git_commit = fmt::format("\t -- Commit: {}", git_CommitSubject());
	std::string git_sha1 = fmt::format("\t -- SHA1: {}", git_CommitSHA1());
	std::string git_date = fmt::format("\t -- Date: {}", git_CommitDate());
	std::string git_ditry = fmt::format("\t -- LocalChanges: {}", git_AnyUncommittedChanges() ? "Yes" : "No");

	logger::info("Git Info:\n{}\n{}\n{}\n{}", git_commit, git_sha1, git_date, git_ditry);

}

static void SetLogLevel() {
	try {
		log::info("Getting Logger Config...");
		const auto& debugConfig = Gts::Config::GetSingleton().GetDebug();
		log::info("Config Loaded");

		spdlog::set_level(debugConfig.GetLogLevel());
		spdlog::flush_on(debugConfig.GetFlushLevel());
	}
	catch (exception e){
		log::critical("Could not load config file", e.what());
		stl::report_and_fail("Could not load config file");
	}
}

SKSEPluginLoad(const LoadInterface * a_skse){


	const auto *plugin  = PluginDeclaration::GetSingleton();
	const auto version = plugin->GetVersion().string();
	const auto name = plugin->GetName();

	InitializeLogging();
	PrintPluginInfo();
	SetLogLevel();

	Init(a_skse);

	InitializeMessaging();
	Hooks::Install();
	InitializePapyrus();
	InitializeSerialization();
	InitializeEventSystem();

	logger::info("SKSEPluginLoad OK");

	return(true);
}



