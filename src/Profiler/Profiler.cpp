
#include "Profiler/Profiler.hpp"

#include "Managers/GtsManager.hpp"

#include "UI/DearImGui/imgui.h"
#include "UI/ImGui/ImFontManager.hpp"

namespace GTS {

	Profiler::Profiler(std::string_view name) : name(std::string(name)) {}

	void Profiler::Start() {
		if (!this->running) {
			m_beg = Clock::now();
			this->running = true;
		}
	}

	void Profiler::Stop() {
		if (this->running) {
			this->elapsed += RunningTime();
			this->running = false;
		}
	}

	void Profiler::Reset() {
		this->elapsed = 0.0f;
	}

	double Profiler::Elapsed() const {
		if (this->IsRunning()) {
			return this->elapsed + this->RunningTime();
		}
		return this->elapsed;
	}

	bool Profiler::IsRunning() const {
		return this->running;
	}

	double Profiler::RunningTime() const {
		if (this->running) {
			return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
		}
		return 0;
	}

	std::string Profiler::GetName() {
		return this->name;
	}

	ProfilerHandle::ProfilerHandle(std::string_view name) : name(std::string(name)) {
		Profilers::Start(name);
	}

	ProfilerHandle::~ProfilerHandle() {
		Profilers::Stop(this->name);
	}

	void Profilers::Start(std::string_view name) {

		[[unlikely]] if (Profiler::ProfilerEnabled) {
			auto& me = Profilers::GetSingleton();
			auto key = std::string(name);
			me.profilers.try_emplace(key, name);
			me.profilers.at(key).Start();
			if (me.AnyRunning()) {
				me.totalTime.Start();
			}
		}
	}

	void Profilers::Stop(std::string_view name) {
		[[unlikely]] if (Profiler::ProfilerEnabled) {
			auto& me = Profilers::GetSingleton();
			auto key = std::string(name);
			me.profilers.try_emplace(key, name);
			me.profilers.at(key).Stop();
			if (!me.AnyRunning()) {
				me.totalTime.Stop();
			}
		}
	}

	bool Profilers::AnyRunning() {
		for (auto& profiler : this->profilers | views::values) {
			if (profiler.IsRunning()) {
				return true;
			}
		}
		return false;
	}


    void Profilers::DisplayReport() {

        auto& Instance = Profilers::GetSingleton();

		ImGui::PushFont(ImFontManager::GetFont("subscript"));

        if (ImGui::Begin("Profiler Report", nullptr, ImGuiWindowFlags_NavFlattened | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {

			double sTotalTime = Instance.totalTime.Elapsed();
			ImGui::Text("Total DLL Time: %.3fms", sTotalTime * 1000);
			ImGui::SameLine();
			ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

			ImGui::Text("Find_Actors: %d", GtsManager::LoadedActorCount);

            if (ImGui::BeginTable("ProfilerTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingFixedFit)) {

                ImGui::TableSetupColumn("Profiler", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("CPU Time", ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("DLL %", ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Game %", ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_WidthFixed);

                ImGui::TableHeadersRow();

                static double Time_LastReport = 0.0;
                std::uint64_t Frame_CurrentReport = Time::FramesElapsed();
                double Time_CurrentReport = Time::WorldTimeElapsed();
                double TotalTime = Time_CurrentReport - Time_LastReport;

                std::vector<std::tuple<std::string, double, double, double>> SortableRows;

                // Compute sorting data for each profiler
                for (auto& [Name, ProfilerInstance] : Instance.profilers) {

                    const double Elapsed = ProfilerInstance.Elapsed();
                    const double TimePercent = (TotalTime > 0.0) ? (Elapsed / TotalTime * 100.0) : 0.0;
                    const double DLLPercent = Elapsed * 100.0 / sTotalTime;

                    SortableRows.emplace_back(Name, Elapsed * 1000, DLLPercent, TimePercent);
					ProfilerInstance.Reset();
                }

                ImGuiTableSortSpecs* SortSpecs = ImGui::TableGetSortSpecs();

				ranges::stable_sort(SortableRows, [&SortSpecs](const auto& aFirst, const auto& aSecond) {

					if (SortSpecs && SortSpecs->SpecsCount > 0) {

						for (int i = 0; i < SortSpecs->SpecsCount; i++) {

							auto& Spec = SortSpecs->Specs[i];
							bool Ascending = Spec.SortDirection == ImGuiSortDirection_Ascending;

							switch (Spec.ColumnIndex) {

								// Profiler Name
								case 0: {
									if (std::get<0>(aFirst) != std::get<0>(aSecond)) {
										return Ascending ? (std::get<0>(aFirst) < std::get<0>(aSecond)) : (std::get<0>(aFirst) > std::get<0>(aSecond));
									}
									break;
								}
								// DLL CPU Time ms
								case 1: {
									if (std::get<1>(aFirst) != std::get<1>(aSecond)) {
										return Ascending ? (std::get<1>(aFirst) < std::get<1>(aSecond)) : (std::get<1>(aFirst) > std::get<1>(aSecond));
									}
									break;
								}
								// % Of DLL Time
								case 2: {
									if (std::get<2>(aFirst) != std::get<2>(aSecond)) {
										return Ascending ? (std::get<2>(aFirst) < std::get<2>(aSecond)) : (std::get<2>(aFirst) > std::get<2>(aSecond));
									}
									break;
								}
								// % Of Game Frame
								case 3: {
									if (std::get<3>(aFirst) != std::get<3>(aSecond)) {
										return Ascending ? (std::get<3>(aFirst) < std::get<3>(aSecond)) : (std::get<3>(aFirst) > std::get<3>(aSecond));
									}
									break;
								}
							}
						}
					}
					// If all specified columns are equal, maintain original order
					return false;
				});

				// Always mark specs as clean after sorting
				if (SortSpecs) {
					SortSpecs->SpecsDirty = false;
				}

				//Render
                for (const auto& Row : SortableRows) {
                    ImGui::TableNextRow();

                    //Name
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", std::get<0>(Row).c_str());

                    //ms time
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.3fms", std::get<1>(Row));

                    //% Of Dll
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.2f%%", std::get<2>(Row));

                    //ms Per Frame
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.2f%%", std::get<3>(Row));
                }

                ImGui::EndTable();

                // Reset total time tracking for the next report
                Instance.totalTime.Reset();
                Time_LastReport = Time_CurrentReport;
            }
        }
        ImGui::End();
		ImGui::PopFont();
    }


	Profilers& Profilers::GetSingleton() {
		static Profilers instance;
		return instance;
	}
}
