// This is a virtual audio interface for testing SRAL capabilities.
#include <stdint.h>
#include <iostream>
#include <memory>
#include <map>
#include <functional>
#include <exception>

// Define SRAL_STATIC if linking against a static SRAL library
#define SRAL_STATIC
#include <SRAL.h>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <signal.h>
#include <fcntl.h>
#endif

void sleep_ms(int milliseconds) {
#ifdef _WIN32
	Sleep(milliseconds);
#else
	usleep((useconds_t)milliseconds * 1000);
#endif
}

enum EKeyCode : unsigned int {
	KEY_CODE_NONE = 0,
	KEY_CODE_ESCAPE = 27,
	KEY_CODE_TAB = 9,
	KEY_CODE_SHIFT_TAB = KEY_CODE_TAB + 0x41f,
	KEY_CODE_ENTER = 13,
	KEY_CODE_SPACE = 32,
	KEY_CODE_Q = 'q',
	KEY_CODE_H = 'h',
	KEY_CODE_ARROW_UP = 256,
	KEY_CODE_ARROW_DOWN,
	KEY_CODE_ARROW_LEFT,
	KEY_CODE_ARROW_RIGHT
};

#ifndef _WIN32
struct STerminal {
	struct termios m_oldTermios, m_newTermios;
	bool m_termiosInitialized{false};

	STerminal() {
		if (!m_termiosInitialized) {
			tcgetattr(STDIN_FILENO, &m_oldTermios);
			m_newTermios = m_oldTermios;
			m_newTermios.c_lflag &= ~(ICANON | ECHO);
			m_newTermios.c_cc[VMIN] = 0;
			m_newTermios.c_cc[VTIME] = 0;
			tcsetattr(STDIN_FILENO, TCSANOW, &m_newTermios);
			m_termiosInitialized = true;
		}
	}

	~STerminal() {
		if (m_termiosInitialized) {
			tcsetattr(STDIN_FILENO, TCSANOW, &m_oldTermios);
			m_termiosInitialized = false;
		}
	}
} g_terminal;

bool _kbhit_linux() {
	struct timeval tv = {0L, 0L};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}

int _getch_linux() {
	char c;
	ssize_t bytesRead = read(STDIN_FILENO, &c, 1);
	if (bytesRead == 1) {
		return static_cast<unsigned char>(c);
	}
	return EOF;
}

int read_arrow_key() {
	if (!_kbhit_linux()) {
		return static_cast<int>(KEY_CODE_NONE);
	}

	int c = _getch_linux();
	if (c == EOF) {
		return static_cast<int>(KEY_CODE_NONE);
	}

	if (c == 27) {
		if (_kbhit_linux()) {
			int c2 = _getch_linux();
			if (c2 == '[') {
				if (_kbhit_linux()) {
					int c3 = _getch_linux();
					switch (c3) {
						case 'A': return static_cast<int>(KEY_CODE_ARROW_UP);
						case 'B': return static_cast<int>(KEY_CODE_ARROW_DOWN);
						case 'C': return static_cast<int>(KEY_CODE_ARROW_RIGHT);
						case 'D': return static_cast<int>(KEY_CODE_ARROW_LEFT);
						case 'Z': return static_cast<int>(KEY_CODE_SHIFT_TAB);

						default: return c;
					}
				}
			}
			return c;
		}
		return static_cast<int>(KEY_CODE_ESCAPE);
	}

	return c;
}

#define _getch() _getch_linux()
#define _kbhit() _kbhit_linux()

#else
int read_arrow_key() {
	if (!_kbhit()) {
		return static_cast<int>(KEY_CODE_NONE);
	}

	int c = _getch();
	if (c == EOF) {
		return static_cast<int>(KEY_CODE_NONE);
	}

	if (c == 0 || c == 224) {
		if (_kbhit()) {
			int c2 = _getch();
			switch (c2) {
				case 72: return static_cast<int>(KEY_CODE_ARROW_UP);
				case 80: return static_cast<int>(KEY_CODE_ARROW_DOWN);
				case 75: return static_cast<int>(KEY_CODE_ARROW_LEFT);
				case 77: return static_cast<int>(KEY_CODE_ARROW_RIGHT);
				default: return c;
			}
		}

		return c;
	}

	if (c == 27) return static_cast<int>(KEY_CODE_ESCAPE);
	if (c == 9) return static_cast<int>(KEY_CODE_TAB);
	if (c == 13) return static_cast<int>(KEY_CODE_ENTER);
	if (c == 32) return static_cast<int>(KEY_CODE_SPACE);

	return c;
}

#endif

namespace Gui {
	class CVirtualAudioInterface {
	private:
		int m_currentEngine{0};
		std::map<std::string, int> m_engines;

	public:
		CVirtualAudioInterface() {
		}

		int GetCurrentEngine() const {return m_currentEngine; }

		bool Initialize() {
			if (!SRAL_Initialize(0)) {
				return false;
			}

			m_currentEngine = SRAL_GetCurrentEngine();

			std::cout << "SRAL Initialized successfully!\n";
			std::cout << "Current engine: " << GetEngineName(m_currentEngine) << "\n";

			return true;
		}

		void Shutdown() {
			m_engines.clear();
			SRAL_Uninitialize();
		}

		~CVirtualAudioInterface() {
		}

		void Speak(const std::string& text, bool interrupt = true) {
			SRAL_SpeakEx(m_currentEngine, text.c_str(), interrupt);
		}

		void SpeakStatus(const std::string& text) {
			SRAL_SpeakEx(m_currentEngine, text.c_str(), false);
		}

		void StopSpeech() {
			SRAL_StopSpeechEx(m_currentEngine);
		}

		void ListAvailableEngines(bool announce = false) {
			if (announce) StopSpeech();
			m_engines.clear();
			int available = SRAL_GetAvailableEngines();
			int active = SRAL_GetActiveEngines();

			if (announce) SpeakStatus("Available engines:");
			for (int engine = SRAL_ENGINE_NVDA; engine <= SRAL_ENGINE_AV_SPEECH; engine <<= 1) {
				if (available & engine) {
					std::string status = (active & engine) ? " (Active)" : " (Available)";
					if (announce) SpeakStatus(std::string(SRAL_GetEngineName(engine)) + status);
					m_engines[GetEngineName(engine)] = engine;
				}
			}
		}

		void SwitchEngine(int engine) {
			StopSpeech();
			if (SRAL_GetAvailableEngines() & engine) {
				if (SRAL_SpeakEx(engine, "Testing engine", true)) {
					m_currentEngine = engine;
					SpeakStatus("Switched to " + GetEngineName(engine));
				} else {
					SpeakStatus("Failed to switch to " + GetEngineName(engine));
				}
			} else {
				SpeakStatus("Engine not available");
			}
		}

		std::string GetEngineName(int engine) {
			return std::string(SRAL_GetEngineName(engine));
		}

		void SetSpeechRate(int rate) {
			SRAL_SetEngineParameter(m_currentEngine, SRAL_PARAM_SPEECH_RATE, &rate);
		}

		void SetVolume(int volume) {
			SRAL_SetEngineParameter(m_currentEngine, SRAL_PARAM_SPEECH_VOLUME, &volume);
		}
		std::map<std::string, int>& GetEngineList() {return m_engines;}

	};

	static CVirtualAudioInterface s_audio;

	class IControl {
	protected:
		std::string m_name, m_nameSpoken;
		std::string m_type{"Unknown"}, m_typeSpoken;
		std::string m_state{""}, m_stateSpoken;
		std::string m_suggestions, m_suggestionsSpoken;
		bool m_isFocused{false};
	public:
		IControl* m_parent{nullptr};

		enum EFocusState : unsigned char {
			FOCUS_SET = 0,
			FOCUS_LOST
		};
		using FocusCallback = EFocusState(*)(void* user_data);
		FocusCallback m_focusCallback;
		static const signed int SUGGESTION_DELAY = 350;

		IControl(const std::string& name) : m_name(name) {}
		virtual ~IControl() = default;

		void Announce() {
		if (!m_isFocused) return;
			s_audio.StopSpeech();
			if (m_name != m_nameSpoken) {
				s_audio.SpeakStatus(m_name);
				m_nameSpoken = m_name;
			}
			if (m_type != m_typeSpoken) {
				s_audio.SpeakStatus(m_type);
				m_typeSpoken = m_type;
			}
			if (m_state != m_stateSpoken) {
				s_audio.SpeakStatus(m_state);
				m_stateSpoken = m_state;
			}

			SRAL_Delay(SUGGESTION_DELAY);
			if (m_suggestions != m_suggestionsSpoken) {
				s_audio.SpeakStatus(m_suggestions);
				m_suggestionsSpoken = m_suggestions;
			}
		}

		void ClearSpoken() {
			m_nameSpoken.clear();
			m_typeSpoken.clear();
			m_stateSpoken.clear();
			m_suggestionsSpoken.clear();
		}

		void SetName(const std::string& name) { m_name = name; m_nameSpoken.clear();}
		void SetType(const std::string& type) { m_type = type; m_typeSpoken.clear();}
		void SetState(const std::string& state) { m_state = state; m_stateSpoken.clear();}
		void SetSuggestions(const std::string& suggestions) { m_suggestions = suggestions; m_suggestionsSpoken.clear();}
		void SetIsFocused(bool focus) { m_isFocused = focus;}

		const std::string& GetName() const { return m_name; }
		const std::string& GetType() const { return m_type; }
		const std::string& GetState() const { return m_state; }
		const bool GetIsFocused() const { return m_isFocused;}
	};

	class CButtonControl : public IControl {
	public:
		using ClickCallback = std::function<void()>;

	protected:
		ClickCallback m_clickCallback;
		bool m_isPressed{false};

	public:
		CButtonControl(const std::string& name) : IControl(name) {
			SetType("Button");
			SetSuggestions("Press Space or Enter to activate");
		}

		void SetClickCallback(ClickCallback callback) {
			m_clickCallback = callback;
		}

		void Press() {
			m_isPressed = true;
			SetState("Pressed");
			Announce();

			if (m_clickCallback) {
				m_clickCallback();
			}

			m_isPressed = false;
			SetState("");
		}

		bool IsPressed() const { return m_isPressed; }
	};

	class CSliderControl : public IControl {
	private:
		int m_minValue{-100};
		int m_maxValue{100};
		int m_currentValue{0};
		int m_step{1};

	public:
		using ValueChangeCallback = std::function<void(int)>;
	protected:
		ValueChangeCallback m_valueChangeCallback;

	public:
		CSliderControl(const std::string& name) : IControl(name) {
			SetType("Slider");
			UpdateState();
			SetSuggestions("Use Left and Right arrows to adjust value");
		}

		void SetRange(int min, int max, int step = 1) {
			m_minValue = min;
			m_maxValue = max;
			m_step = step;
			if (m_currentValue < min) m_currentValue = min;
			if (m_currentValue > max) m_currentValue = max;
			UpdateState();
		}

		void SetValue(int value) {
			if (this->m_valueChangeCallback) {
				this->m_valueChangeCallback(value);
			}
			m_currentValue = std::max(m_minValue, std::min(m_maxValue, value));
			UpdateState();
		}

		void Increase() {
			SetValue(m_currentValue + m_step);
			m_stateSpoken.clear();
			Announce();
		}

		void Decrease() {
			SetValue(m_currentValue - m_step);
			m_stateSpoken.clear();
			Announce();
		}

		void SetValueChangeCallback(ValueChangeCallback callback) {
			m_valueChangeCallback = callback;
		}

		int GetValue() const { return m_currentValue; }
		int GetMin() const { return m_minValue; }
		int GetMax() const { return m_maxValue; }

	private:
		void UpdateState() {
			SetState(std::to_string(m_currentValue) + "%");
		}
	};

	struct SDropdownControlData {
		std::string name;
		int data;
	};

	class CDropdownControl : public CButtonControl {
	private:
		std::vector<SDropdownControlData> m_options;
		int m_selectedIndex{-1};
		bool m_isExpanded{false};

	public:
		CDropdownControl(const std::string& name) : CButtonControl(name) {
			SetType("Dropdown");
			SetSuggestions("Use Up and Down arrows to navigate, Enter to select");
		}

		void AddOption(const std::string& option, int data = -1) {
			m_options.push_back({option, data});
			if (m_selectedIndex == -1 && !m_options.empty()) {
				m_selectedIndex = 0;
				UpdateState();
			}
		}

		void SetSelectedIndex(int index) {
			if (index >= 0 && index < static_cast<int>(m_options.size())) {
				m_selectedIndex = index;
				UpdateState();
			}
		}

		void Expand() {
			m_isExpanded = true;
			SetState("Expanded, " + std::to_string(m_options.size()) + " options available");
			Announce();
		}

		void Collapse() {
			m_isExpanded = false;
			UpdateState();
		}

		void SelectNext() {
			if (m_isExpanded && !m_options.empty()) {
				m_selectedIndex = (m_selectedIndex + 1) % m_options.size();
				UpdateState();
				s_audio.Speak(m_options[m_selectedIndex].name);
			}
		}

		void SelectPrevious() {
			if (m_isExpanded && !m_options.empty()) {
				m_selectedIndex = (m_selectedIndex - 1 + m_options.size()) % m_options.size();
				UpdateState();
				s_audio.Speak(m_options[m_selectedIndex].name);
			}
		}

		bool IsExpanded() const { return m_isExpanded; }
		const SDropdownControlData& GetSelectedOption() const { 
			if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_options.size())) {
				return m_options[m_selectedIndex];
			}
			static struct SDropdownControlData s_dummy;
			return s_dummy;
		}

	private:
		void UpdateState() {
			if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_options.size())) {
				SetState("Selected: " + m_options[m_selectedIndex].name);
			} else {
				SetState("No selection");
			}
		}
	};

	class CTerminalUiFactory {
	private:
		std::vector<std::unique_ptr<IControl>> m_controls;
		std::vector<IControl*> m_focusOrder;
		size_t m_currentFocusIndex{0};
		bool m_running{false};

		CButtonControl* m_engineListBtn{nullptr};
		CButtonControl* m_volumeUpBtn{nullptr};
		CButtonControl* m_volumeDownBtn{nullptr};
		CButtonControl* m_rateUpBtn{nullptr};
		CButtonControl* m_rateDownBtn{nullptr};
		CSliderControl* m_volumeSlider{nullptr};
		CSliderControl* m_rateSlider{nullptr};
		CDropdownControl* m_engineDropdown{nullptr};

	public:
		CTerminalUiFactory() {
			CreateUi();
		}

		~CTerminalUiFactory() {
		}

		void Run() {
			m_running = true;

			std::cout << "=== SRAL Virtual Audio Interface ===\n";
			std::cout << "Controls: Tab to navigate, Arrow keys to adjust, Space/Enter to activate\n";
			std::cout << "Press 'q' to quit, 'h' for help\n\n";
			s_audio.Speak("SRAL Virtual Audio Interface started. Use Tab to navigate between controls.");
			while (m_running) {
				int key = read_arrow_key();
				if (key != 0) {
					HandleInput(key);
				}
				sleep_ms(1);
			}
			s_audio.Shutdown();
		}

	private:
		void CreateUi() {
		s_audio.Initialize();
			auto engineBtn = std::make_unique<CButtonControl>("List Available Engines");
			m_engineListBtn = engineBtn.get();
			engineBtn->SetClickCallback([this]() {
				s_audio.ListAvailableEngines(true);
			});
			
			auto volUpBtn = std::make_unique<CButtonControl>("Volume Up");
			m_volumeUpBtn = volUpBtn.get();
			volUpBtn->SetClickCallback([this]() {
				m_volumeSlider->Increase();
			});

			auto volDownBtn = std::make_unique<CButtonControl>("Volume Down");
			m_volumeDownBtn = volDownBtn.get();
			volDownBtn->SetClickCallback([this]() {
				m_volumeSlider->Decrease();
			});

			auto rateUpBtn = std::make_unique<CButtonControl>("Rate Up");
			m_rateUpBtn = rateUpBtn.get();
			rateUpBtn->SetClickCallback([this]() {
				m_rateSlider->Increase();
			});

			auto rateDownBtn = std::make_unique<CButtonControl>("Rate Down");
			m_rateDownBtn = rateDownBtn.get();
			rateDownBtn->SetClickCallback([this]() {
				m_rateSlider->Decrease();
			});

			auto volumeSlider = std::make_unique<CSliderControl>("Volume Level");
			m_volumeSlider = volumeSlider.get();
			int currentVolume = 0;

			if (SRAL_GetEngineParameter(s_audio.GetCurrentEngine(), SRAL_PARAM_SPEECH_VOLUME, &currentVolume)) {
				volumeSlider->SetValue(currentVolume);
			}
			volumeSlider->SetValueChangeCallback([this](int value) {
				s_audio.SetVolume(value);
			});
			auto rateSlider = std::make_unique<CSliderControl>("Speech Rate");
			m_rateSlider = rateSlider.get();
			int currentRate = 0;

			if (SRAL_GetEngineParameter(s_audio.GetCurrentEngine(), SRAL_PARAM_SPEECH_RATE, &currentRate)) {
				rateSlider->SetValue(currentRate);
			}
			rateSlider->SetValueChangeCallback([this](int value) {
				s_audio.SetSpeechRate(value);
			});

			auto engineDropdown = std::make_unique<CDropdownControl>("Select Engine");
			m_engineDropdown = engineDropdown.get();
			s_audio.ListAvailableEngines();
			auto& engines = s_audio.GetEngineList();
			for (auto& [name, engineId] : engines) {
				m_engineDropdown->AddOption(name, engineId);
			}

			engineDropdown->SetClickCallback([this]() {
				if (!m_engineDropdown->IsExpanded()) {
					m_engineDropdown->Expand();
				} else {
					m_engineDropdown->Collapse();
					s_audio.SwitchEngine(m_engineDropdown->GetSelectedOption().data);
				}
			});

			m_controls.push_back(std::move(engineBtn));
			m_controls.push_back(std::move(volUpBtn));
			m_controls.push_back(std::move(volDownBtn));
			m_controls.push_back(std::move(rateUpBtn));
			m_controls.push_back(std::move(rateDownBtn));
			m_controls.push_back(std::move(volumeSlider));
			m_controls.push_back(std::move(rateSlider));
			m_controls.push_back(std::move(engineDropdown));

			for (auto& control : m_controls) {
				m_focusOrder.push_back(control.get());
			}

			if (!m_focusOrder.empty()) {
				m_focusOrder[m_currentFocusIndex]->Announce();
			}
		}

		void HandleInput(int key) {
			if (key == 'q' || key == 'Q') {
				m_running = false;
				s_audio.Speak("Exiting SRAL Virtual Audio Interface");
				return;
			}

			if (key == 'h' || key == 'H') {
				ShowHelp();
				return;
			}

			IControl* currentControl = m_focusOrder[m_currentFocusIndex];

			switch (key) {
				case KEY_CODE_TAB:
					MoveFocusForward();
					break;
				case KEY_CODE_SHIFT_TAB:
					MoveFocusBackward();
					break;
				case KEY_CODE_ENTER:
				case '\n': 
				case KEY_CODE_SPACE:
					if (auto* button = dynamic_cast<CButtonControl*>(currentControl)) {
						button->Press();
					} else if (auto* dropdown = dynamic_cast<CDropdownControl*>(currentControl)) {
						if (dropdown->IsExpanded()) {
							dropdown->Collapse();
							s_audio.Speak("Selected: " + dropdown->GetSelectedOption().name);
						} else {
							dropdown->Expand();
						}
					}
					break;
				case KEY_CODE_ARROW_UP:
				case 'w':
				case 'W':
					if (auto* dropdown = dynamic_cast<CDropdownControl*>(currentControl)) {
						if (dropdown->IsExpanded()) {
							dropdown->SelectPrevious();
						}
						else {
							dropdown->Expand();
							HandleInput(key);
							return;
						}

					} else if (auto* slider = dynamic_cast<CSliderControl*>(currentControl)) {
						slider->Increase();
					}
					break;
				case KEY_CODE_ARROW_DOWN:
				case 's':
				case 'S':
					if (auto* dropdown = dynamic_cast<CDropdownControl*>(currentControl)) {
						if (dropdown->IsExpanded()) {
							dropdown->SelectNext();
						}
						else {
							dropdown->Expand();
							HandleInput(key);
							return;
						}

					} else if (auto* slider = dynamic_cast<CSliderControl*>(currentControl)) {
						slider->Decrease();
					}
					break;
				case KEY_CODE_ARROW_LEFT:
				case 'a':
				case 'A':
					if (auto* slider = dynamic_cast<CSliderControl*>(currentControl)) {
						slider->Decrease();
					}
					break;
				case KEY_CODE_ARROW_RIGHT:
				case 'd':
				case 'D':
					if (auto* slider = dynamic_cast<CSliderControl*>(currentControl)) {
						slider->Increase();
					}
					break;
			}
		}

		void MoveFocusForward() {
			m_focusOrder[m_currentFocusIndex]->ClearSpoken();
			m_focusOrder[m_currentFocusIndex]->SetIsFocused(false);
			m_currentFocusIndex = (m_currentFocusIndex + 1) % m_focusOrder.size();
			m_focusOrder[m_currentFocusIndex]->SetIsFocused(true);
			m_focusOrder[m_currentFocusIndex]->Announce();
		}

		void MoveFocusBackward() {
			m_focusOrder[m_currentFocusIndex]->ClearSpoken();
			m_focusOrder[m_currentFocusIndex]->SetIsFocused(false);
			m_currentFocusIndex = (m_currentFocusIndex - 1 + m_focusOrder.size()) % m_focusOrder.size();
			m_focusOrder[m_currentFocusIndex]->SetIsFocused(true);
			m_focusOrder[m_currentFocusIndex]->Announce();
		}

		void ShowHelp() {
			std::string helpText = 
				"SRAL Virtual Audio Interface Help: "
				"Tab - Navigate between controls, "
				"Enter/Space - Activate button or select dropdown, "
				"Arrow keys - Adjust sliders or navigate dropdowns, "
				"Q - Quit application, "
				"H - Show this help";
			s_audio.Speak(helpText);
		}
	};
}

int main() {
	Gui::CTerminalUiFactory ui;
	ui.Run();
	return 0;
}
