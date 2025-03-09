#include "API/Racemenu.hpp"
#include "API/SmoothCam.hpp"
#include "Hooks/Hooks.hpp"
#include "Papyrus/Papyrus.hpp"
#include "UI/DebugAPI.hpp"
#include "Config/Config.hpp"
#include "Utils/InitUtils.hpp"

#include "Managers/Register.hpp"
#include "Managers/Input/InputManager.hpp"
#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Managers/Console/ConsoleManager.hpp"

using namespace SKSE;
using namespace RE;
using namespace RE::BSScript;
using namespace GTS;

namespace {

	void InitializeLogging() {
		auto path = GTS::log_directory_fixed();

		if (!path) {
			ReportAndExit("Unable to lookup SKSE logs directory.");
		}

		*path /= PluginDeclaration::GetSingleton()->GetName();
		*path += L".log";

		std::shared_ptr <spdlog::logger> log;

		if (IsDebuggerPresent()) {
			log = std::make_shared <spdlog::logger>("Global", std::make_shared <spdlog::sinks::msvc_sink_mt>());
			log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
		}
		else {

			#ifdef GTSCONSOLE

				auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
				console_sink->set_pattern("\033[37m[\033[33mGTS\033[37m]\033[0m\033[37m[\033[37m%H:%M:%S.%e\033[37m]\033[0m\033[37m[%^%l%$]\033[0m\033[37m[\033[33m%s:%#\033[37m]\033[0m\033[37m: %v\033[0m");
				log = std::make_shared <spdlog::logger>(spdlog::logger("Global", console_sink));

			#else

				log = std::make_shared <spdlog::logger>("Global", std::make_shared <spdlog::sinks::basic_file_sink_mt>(path->string(), true));
				log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

			#endif

		}

		spdlog::set_default_logger(std::move(log));
		spdlog::set_level(spdlog::level::level_enum::trace);
		spdlog::flush_on(spdlog::level::level_enum::trace);

	}

	void InitializeMessaging() {

		if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message *message) {
			switch (message->type) {

				// Called after all plugins have finished running SKSEPlugin_Load.
				case MessagingInterface::kPostLoad: { 
					break;
				}

				// Called after all kPostLoad message handlers have run.
				case MessagingInterface::kPostPostLoad: {
					//Racemenu::Register(); // <- Disabled For Now...
					break;
				}

				// Called when all game data has been found.
				case MessagingInterface::kInputLoaded: {
					break;
				}

				// All ESM/ESL/ESP plugins have loaded, main menu is now active.
				case MessagingInterface::kDataLoaded: {

					EventDispatcher::DoDataReady();
					InputManager::GetSingleton().Init();
					ConsoleManager::Init();
					SmoothCam::Register();

					CPrintPluginInfo();
					break;
				}

				// Skyrim game events.
				// Player's selected save game has finished loading.
				case MessagingInterface::kPostLoadGame: {
					Plugin::SetInGame(true);
					Cprint("[GTSPlugin.dll]: [ Succesfully initialized and loaded ]");
					break;
				}

				// Player starts a new game from main menu.
				case MessagingInterface::kNewGame: {
					Plugin::SetInGame(true);
					EventDispatcher::DoReset();
					Cprint("[GTSPlugin.dll]: [ Succesfully initialized and loaded ]");
					break;
				}

				// Player selected a game to load, but it hasn't loaded yet.
				// Data will be the name of the loaded save.
				case MessagingInterface::kPreLoadGame:{
					Plugin::SetInGame(false);
					EventDispatcher::DoReset();
					break;
				}

				// The player has saved a game.
				case MessagingInterface::kSaveGame:{
					break;
				}

				// The player deleted a saved game from within the load menu.
				// Data will be the save name.
				case MessagingInterface::kDeleteGame:{
					break;
				} 
			}
		})) {
			ReportAndExit("Unable to register message listener.");
		}
	}

	void InitializeSerialization() {
		log::trace("Initializing cosave serialization...");

		auto* serde = GetSerializationInterface();


		serde->SetUniqueID(_byteswap_ulong('GTSP'));

		serde->SetSaveCallback(Persistent::OnGameSaved);
		serde->SetRevertCallback(Persistent::OnRevert);
		serde->SetLoadCallback(Persistent::OnGameLoaded);

		log::info("Cosave serialization initialized.");
	}

	void InitializePapyrus() {

		log::trace("Initializing Papyrus bindings...");

		if (GetPapyrusInterface()->Register(GTS::register_papyrus)) {
			log::info("Papyrus functions bound.");
		}
		else {
			ReportAndExit("Failure to register Papyrus bindings.");
		}
	}

	void InitializeEventSystem() {

		EventDispatcher::AddListener(&DebugOverlayMenu::GetSingleton());
		EventDispatcher::AddListener(&Runtime::GetSingleton()); // Stores spells, globals and other important data
		EventDispatcher::AddListener(&Persistent::GetSingleton());
		EventDispatcher::AddListener(&Transient::GetSingleton());
		EventDispatcher::AddListener(&CooldownManager::GetSingleton());
		EventDispatcher::AddListener(&TaskManager::GetSingleton());
		EventDispatcher::AddListener(&SpringManager::GetSingleton());

		log::info("Added Default Listeners");

		RegisterManagers();
	}

	void SetLogLevel() {

		try {
			log::info("Getting Logger Config...");
			const auto& debugConfig = Config::GetAdvanced();
			log::info("Config Loaded from settings struct: Print: {} Flush: {}", debugConfig.sLogLevel, debugConfig.sFlushLevel);
			spdlog::set_level(spdlog::level::from_str(debugConfig.sLogLevel));
			spdlog::flush_on(spdlog::level::from_str(debugConfig.sFlushLevel));
		}
		catch (exception& e) {
			logger::critical("Could not load spdlog settings from config struct", e.what());
			ReportAndExit("Could not load spdlog settings from config struct");
		}
	}
}

SKSEPluginLoad(const LoadInterface * a_skse){

	//This hack is needed because debug builds of commonlib combletly break during trampoline hooks.
	//Destination pointers for write call and write branch suddenly forget to add offsets to skyrims base image address.
	//Why??? who tf knows why...
	//So we instead build with the relwithdebinfo preset when using the debug and debug-eha presets, but pass all debug flags to the compiler when doing so...
	//This results in this dll being built with full debug options but commonlib and other libraries being built as release...
	//I mean is this good? No. But does it finnaly allow us to have working breakpoints in the dll when using a debugger? Yes.

	#ifdef GTSCONSOLE
		AllocateConsole();
    #endif

	#ifdef GTSDEBUG
		ReportInfo("GTSplugin Debug Build.\nAttach the debugger and press OK.");
		std::cout << "GTSPlugin Debug Build" << '\n';
	#endif

	InitializeLogging();
	LogPrintPluginInfo();

	#ifndef GTSCONSOLE
		SetLogLevel();
	#endif

	Init(a_skse);
	VersionCheck(a_skse);


	InitializeMessaging();
	Hooks::Install();
	InitializePapyrus();
	InitializeSerialization();
	InitializeEventSystem();

	logger::info("SKSEPluginLoad OK");

	return true;
}