#pragma once
#include "SKSE/Logger.h"

#include "RE/V/VirtualMachine.h"
#include "SKSE/API.h"

#include <ShlObj.h>
using namespace SKSE;
using namespace RE;


namespace Gts {

	std::optional<std::filesystem::path> log_directory() {
		wchar_t* buffer{ nullptr };
		const auto result = ::SHGetKnownFolderPath(::FOLDERID_Documents, ::KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
		std::unique_ptr<wchar_t[], decltype(&::CoTaskMemFree)> knownPath(buffer, ::CoTaskMemFree);

		if (!knownPath || result != 0) {
			SKSE::stl::report_and_fail("failed to get known folder path"sv);
			return std::nullopt;
		}

		std::filesystem::path path = knownPath.get();
		path /= "My Games"sv;
	#ifndef SKYRIMVR
		path /= std::filesystem::exists("steam_api64.dll") ? "Skyrim Special Edition" : "Skyrim Special Edition GOG";
	#else
		//Broken on > 1130
		path /= *REL::Relocation<const char**>(RELOCATION_ID(508778, 502114)).get();
	#endif
		path /= "SKSE"sv;

		return path;
	}
}