#pragma once

namespace GTS {

	static inline void ReportAndExit(const std::string_view& a_message) {
		MessageBoxA(
			nullptr,
			a_message.data(),
			"Size Matters - GtsPlugin.dll",
			MB_OK | MB_ICONERROR | MB_TOPMOST
		);
		SKSE::WinAPI::TerminateProcess(SKSE::WinAPI::GetCurrentProcess(), EXIT_FAILURE);
	}


	static inline void ReportInfo(const std::string_view& a_message) {
		MessageBoxA(
			nullptr,
			a_message.data(),
			"Size Matters - GtsPlugin.dll",
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST
		);
	}

}