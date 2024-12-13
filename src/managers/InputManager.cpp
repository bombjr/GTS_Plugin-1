#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "data/plugin.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "utils/av.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;

namespace {
	std::vector<InputEventData> LoadInputEvents() {
		const auto data = toml::parse(R"(Data\SKSE\Plugins\GtsInput.toml)");
		// Toml Example
		// ```toml
		// [[InputEvent]]
		// name = “Stomp”
		// keys = [“E”, “LeftShift”]
		// duration = 0.0
		// blockinput = true
		// ```
		const auto aov = toml::find_or<std::vector<toml::value> >(data, "InputEvent", {});
		std::vector<InputEventData> results;
		for (const auto& table: aov) {
			std::string name = toml::find_or<std::string>(table, "name", "");
			const auto keys = toml::find_or<vector<std::string> >(table, "keys", {});
			if (name != "" && !keys.empty()) {
				InputEventData newData(table);
				if (newData.HasKeys()) {
					results.push_back(newData);
				} 
				else {
					log::error("No valid keys found for event {} at line {}", name, table.location().line());
					PrintMessageBox("GtsInput.toml error: No valid keys found for event {} at line {}. GTS Input won't work because of errors.", name, table.location().line());
				}
			} 
			else if (keys.empty()) {
				log::warn("Missing keys for {} at line {}", name, table.location().line());
				PrintMessageBox("GtsInput.toml error: Missing keys for {} at line {}.  GTS Input won't work because of errors.", name, table.location().line());
			} 
			else {
				log::warn("Missing name for [[InputEvent]] at line {}", table.location().line());
				PrintMessageBox("GtsInput.toml error: Missing name for [[InputEvent]] at line {}. GTS Input won't work because of errors.", table.location().line());
			}
		}

		// Sort longest duration first
		std::sort(results.begin(), results.end(),
		          [] (InputEventData const& a, InputEventData const& b) {
			return a.MinDuration() > b.MinDuration();
		});
		return results;
	}
}

namespace Gts {

	//-----------------
	// InputEventData
	//-----------------

	InputEventData::InputEventData(const toml::value& data) {
		this->name = toml::find_or<std::string>(data, "name", "");
		float duration = toml::find_or<float>(data, "duration", 0.0f);
		this->exclusive = toml::find_or<bool>(data, "exclusive", false);
		std::string blockInput = toml::find_or<std::string>(data, "blockinput", "default");
		std::string trigger = toml::find_or<std::string>(data, "trigger", "once");

		std::string lower_trigger = str_tolower(trigger);
		std::string lower_blockInput = str_tolower(blockInput);

		//Trigger Parse
		if (lower_trigger == "once") {
			this->trigger = TriggerMode::Once;
		} 
		else if (lower_trigger == "release") {
			this->trigger = TriggerMode::Release;
		} 
		else if (
			lower_trigger ==  "continuous"
			|| lower_trigger ==  "cont"
			|| lower_trigger ==  "continue") {
			this->trigger = TriggerMode::Continuous;
		} 
		else {
			log::warn("Unknown trigger value: {}", lower_trigger);
			this->trigger = TriggerMode::Once;
		}

		//blockInput Parse
		if (lower_blockInput == "default") {
			this->blockinput = BlockCondition::Default;
		}
		else if (lower_blockInput == "force") {
			this->blockinput = BlockCondition::Force;
		}
		else if (lower_blockInput == "never") {
			this->blockinput = BlockCondition::Never;
		}
		else {
			log::warn("Unknown trigger value: {}", lower_blockInput);
			this->blockinput = BlockCondition::Default;
		}



		this->minDuration = duration;
		this->startTime = 0.0;
		this->keys = {};
		const auto keys = toml::find_or<vector<std::string> >(data, "keys", {});
		for (const auto& key: keys) {
			std::string upper_key = str_toupper(remove_whitespace(key));
			if (upper_key != "LEFT" && upper_key != "DIK_LEFT") {
				// This changes LEFTALT to LALT
				// But NOT LEFT into L
				replace_first(upper_key, "LEFT", "L");
			}
			if (upper_key != "RIGHT" && upper_key != "DIK_RIGHT") {
				replace_first(upper_key, "RIGHT", "R");
			}
			try {
				std::uint32_t key_code = NAMED_KEYS.at(upper_key);
				this->keys.emplace(key_code);
			} 
			catch (std::out_of_range e) {
				log::warn("Key named {}=>{} in {} was unrecongized.", key, upper_key, this->name);
				this->keys.clear();
				return; // Remove all keys and return so that this becomes an INVALID key entry and won't fire
			}
		}
	}

	float InputEventData::Duration() const {
		return static_cast<float>(Time::WorldTimeElapsed() - this->startTime);
	}

	float InputEventData::MinDuration() const {
		return this->minDuration;
	}

	void InputEventData::Reset() {
		this->startTime = Time::WorldTimeElapsed();
		this->state = InputEventState::Idle;
		this->primed = false;
	}

	bool InputEventData::IsOnUp() const {
		return this->trigger == TriggerMode::Release;
	}

	bool InputEventData::SameGroup(const InputEventData& other) const {
		if (this->IsOnUp() && other.IsOnUp()) {
			return this->keys == other.keys;
		}
		return false;
	}

	bool InputEventData::AllKeysPressed(const std::unordered_set<std::uint32_t>& keys) {
		if (this->keys.empty()) {
			return false;
		}
		for (const auto& key : this->keys) {
			if (keys.find(key) == keys.end()) {
				// Key not found
				return false;
			}
		}
		return true;
	}

	bool InputEventData::OnlyKeysPressed(const std::unordered_set<std::uint32_t>& keys_in) {
		std::unordered_set<std::uint32_t> keys(keys_in); // Copy
		for (const auto& key : this->keys) {
			keys.erase(key);
		}
		return keys.size() == 0;
	}

	bool InputEventData::ShouldFire(const std::unordered_set<std::uint32_t>& a_gameInputKeys) {
		bool shouldFire = false;
		// Check based on keys and duration
		if (this->AllKeysPressed(a_gameInputKeys) && (!this->exclusive || this->OnlyKeysPressed(a_gameInputKeys))) {
			shouldFire = true;
		} 
		else {
			// Keys aren't held reset the start time of the button hold
			this->startTime = Time::WorldTimeElapsed();
			// and reset the state to idle
			this->state = InputEventState::Idle;
		}
		// Check based on duration
		if (shouldFire) {
			if (this->minDuration > 0.0) {
				// Turn it off if duration is not met
				shouldFire = this->Duration() > this->minDuration;
			}
		}

		// Check based on held and trigger state
		if (shouldFire) {
			this->primed = true;
			switch (this->state) {
				case InputEventState::Idle: {
					this->state = InputEventState::Held;
					switch (this->trigger) {
						// If once or continius start firing now
						case TriggerMode::Once: {
							return true;
						}
						case TriggerMode::Continuous: {
							return true;
						}
						case TriggerMode::Release: {
							return false;
						}
						default: {
							log::error("Unexpected TriggerMode.");
							return false; // Catch if something goes weird
						}
					}
				}
				case InputEventState::Held: {
					switch (this->trigger) {
						// If once stop firing
						case TriggerMode::Once: {
							return false;
						}
						case TriggerMode::Continuous: {
							// if continous keep firing
							return true;
						}
						case TriggerMode::Release: {
							// For release still do nothing
							return false;
						}
						default: {
							log::error("Unexpected TriggerMode.");
							return false; // Catch if something goes weird
						}
					}
				}
				default: {
					log::error("Unexpected InputEventState.");
					return false; // Catch if something goes weird
				}
			}
		} 
		else {
			if (this->primed) {
				this->primed = false;
				switch (this->trigger) {
					case TriggerMode::Release: {
						// For release fire now that we have stopped pressing
						return true;
					}
					default: {
						return false;
					}
				}
			} 
			else {
				return false;
			}
		}
	}

	bool InputEventData::HasKeys() const {
		return !this->keys.empty();
	}

	std::string InputEventData::GetName() const {
		return this->name;
	}

	unordered_set<std::uint32_t> InputEventData::GetKeys() {
		return keys;
	}

	BlockCondition InputEventData::ShouldBlock() {
		return this->blockinput;
	}

	//-----------------
	// InputManager
	//-----------------

	InputManager& InputManager::GetSingleton() noexcept {
		static InputManager instance;
		return instance;
	}

	void InputManager::RegisterInputEvent(std::string_view namesv, std::function<void(const InputEventData&)> callback, std::function<bool(void)> condition) {
		auto& me = InputManager::GetSingleton();
		std::string name(namesv);
		me.registedInputEvents.try_emplace(name, callback, condition);
		log::debug("Registered input event: {}", namesv);
	}

	void InputManager::DataReady() {
		try {
			InputManager::GetSingleton().keyTriggers = LoadInputEvents();
		} 
		catch (toml::exception e) {
			log::error("Error in parsing GtsInput.toml: {}", e.what());
			PrintMessageBox("Error in parsing GtsInput.toml: {}. GTS Input won't work, double-check GtsInput.toml for errors", e.what());
			return;
		} 
		catch (std::runtime_error e) {
			log::error("Error in opening GtsInput.toml: {}", e.what());
			PrintMessageBox("Error in opening GtsInput.toml: {}. GTS Input won't work, double-check GtsInput.toml for errors", e.what());
			return;
		} 
		catch (std::exception e) {
			log::error("Error in GtsInput.toml: {}", e.what());
			PrintMessageBox("Error in GtsInput.toml: {}. GTS Input won't work, double-check GtsInput.toml for errors", e.what());
			return;
		}

		log::info("Loaded {} key bindings", InputManager::GetSingleton().keyTriggers.size());
		
		Ready = true;
	}

	void InputManager::ProcessEvents(InputEvent** a_event) {
		std::unordered_set<uint32_t> KeysToBlock = {};
		std::unordered_set<std::uint32_t> gameInputKeys = {};
		RE::InputEvent* event = *a_event;
		RE::InputEvent* prev = nullptr;

		if (Plugin::AnyMenuOpen() || !a_event || !Ready) {
			return;
		}

		//Get Current InputKeys
		for (auto event = *a_event; event; event = event->next) {
			//If the event is not a button, ignore.
			if (event->GetEventType() != INPUT_EVENT_TYPE::kButton) {
				continue;
			}

			//If the event is not a ButtonEvent or it is one but the event is "empty", ignore.
			ButtonEvent* buttonEvent = event->AsButtonEvent();
			if (!buttonEvent || (!buttonEvent->IsPressed() && !buttonEvent->IsUp())) {
				continue;
			}

			//If it is a ButtonEvent add it to to the list of pressed keys
			if (buttonEvent->device.get() == INPUT_DEVICE::kKeyboard) {
				auto key = buttonEvent->GetIDCode();
				gameInputKeys.emplace(key);
			}
			else if (buttonEvent->device.get() == INPUT_DEVICE::kMouse) {
				auto key = buttonEvent->GetIDCode();
				gameInputKeys.emplace(key + MOUSE_OFFSET);
			}
		}

		for (auto& trigger : this->keyTriggers) {
			// Store triggers in here that have been fired this frame
			std::vector<InputEventData*> firedTriggers; 
			auto blockInput = trigger.ShouldBlock();

			//Are all keys pressed for this trigger and are we allowed to selectively block?
			//if never: behavior defaults to old implementation
			if (trigger.AllKeysPressed(gameInputKeys)){
				//log::debug("AllkeysPressed for trigger {}", trigger.GetName());
				//Get the coresponding event data
				try {
					auto& eventData = this->registedInputEvents.at(trigger.GetName());

					if (blockInput == BlockCondition::Force) {
						//If force blocking is set block game input regardless of conditions
						std::unordered_set<uint32_t> KeysToAdd = std::unordered_set<uint32_t>(trigger.GetKeys());
						KeysToBlock.insert(KeysToAdd.begin(), KeysToAdd.end());

						if (eventData.condition != nullptr) {
							if (!eventData.condition()) {
								continue;
							}
						}

					}
					//The condition callback can be null, check before calling it.
					//In the case it's null input blocking or early continuing won't be done and the system will behave like previously unless its forced.
					else if (eventData.condition != nullptr) {
						//log::debug("condition exists {}", fmt::ptr(&eventData.condition));
						//Used to verify wether this trigger will actually end up doing anthing
						if (eventData.condition()) {
							//log::debug("condition is true for {}", trigger.GetName());
							//Need to make a copy here otherwise insert throws an assertion

							if (blockInput != BlockCondition::Never) {
								std::unordered_set<uint32_t> KeysToAdd = std::unordered_set<uint32_t>(trigger.GetKeys());
								//log::debug("ShouldBlock is true for {}", trigger.GetName());
								KeysToBlock.insert(KeysToAdd.begin(), KeysToAdd.end());
							}
						}
						else {
							//log::debug("Condition Was False For Event: {}", trigger.GetName());
							//If False Skip calling ShouldFire as there is no point in processing an event that won't do anything
							continue;
						}
					}
				}

				catch (std::out_of_range e) {
					log::warn("Event {} was triggered but there is no event of that name", trigger.GetName());
					continue;
				}
			}

			//Handles Event tiggering conditions
			if (trigger.ShouldFire(gameInputKeys)) {
				bool groupAlreadyFired = false;
				for (auto firedTrigger : firedTriggers) {
					if (trigger.SameGroup(*firedTrigger)) {
						groupAlreadyFired = true;
						break;
					}
				}
				if (groupAlreadyFired) {
					trigger.Reset();
				}
				else {
					log::debug("Running event {}", trigger.GetName());
					firedTriggers.push_back(&trigger);
					try {
						auto& eventData = this->registedInputEvents.at(trigger.GetName());
						eventData.callback(trigger);
					}
					catch (std::out_of_range e) {
						log::warn("Event {} was triggered but there is no event of that name", trigger.GetName());
						continue;
					}
				}
			}
		}

		while (event != nullptr) {
			bool shouldDispatch = true;
			auto eventType = event->eventType;
			if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
				const auto button = static_cast<RE::ButtonEvent*>(event);
				if (button) {
					uint32_t input = button->GetIDCode();
					if (KeysToBlock.find(input) != KeysToBlock.end()) {
						logger::debug("Blocked Input For Key {}", input);
							shouldDispatch = false;
					}
				}
			}

			RE::InputEvent* nextEvent = event->next;
			if (!shouldDispatch) {
				if (prev != nullptr) {
					prev->next = nextEvent;
				}
				else {
					*a_event = nextEvent;
				}
			}
			else {
				prev = event;
			}
			event = nextEvent;
		}
	}
	std::string InputManager::DebugName() {
		return "InputManager";
	}
}
