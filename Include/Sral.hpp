#ifndef SRAL_CPP_HPP
#define SRAL_CPP_HPP
#pragma once

#include <SRAL.h>

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace Sral {

extern "C++" {
	// -----------------------------------------------------------------------------
	// Exception Class
	// -----------------------------------------------------------------------------
	class Exception final : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	inline void Check(bool result, const char* msg = "SRAL operation failed") {
		if (!result) throw Exception(msg);
	}

	// -----------------------------------------------------------------------------
	// Data Structures
	// -----------------------------------------------------------------------------

	struct Voice final {
		int index{0};
		std::string name{""};
		std::string language{""};
		std::string gender{""};
		std::string vendor{""};

		// Constructor from C-struct
		explicit Voice(const SRAL_VoiceInfo& info) 
			: index(info.index),
			  name(info.name ? info.name : ""),
			  language(info.language ? info.language : ""),
			  gender(info.gender ? info.gender : ""),
			  vendor(info.vendor ? info.vendor : "") {}
	};

	struct AudioBuffer final {
		std::vector<uint8_t> data;
		int channels{0};
		int sample_rate{0};
		int bits_per_sample{0};

		[[nodiscard]] double GetDurationSeconds() const {
			if (sample_rate == 0 || channels == 0 || bits_per_sample == 0) return 0.0;
			size_t bytes_per_sample = bits_per_sample / 8;
			return static_cast<double>(data.size()) / (sample_rate * channels * bytes_per_sample);
		}
	};

	// -----------------------------------------------------------------------------
	// Main Wrapper Class
	// -----------------------------------------------------------------------------

	class System final {
	public:
		/**
		 * @brief RAII Constructor. Initializes SRAL.
		 * @param engines_exclude Bitmask of engines to exclude.
		 */
		explicit System(int engines_exclude = 0) {
			if (!SRAL_Initialize(engines_exclude)) {
				throw Exception("Failed to initialize SRAL");
			}
		}

		/**
		 * @brief RAII Destructor. Uninitializes SRAL.
		 */
		~System() {
			SRAL_Uninitialize();
		}

		// Delete copy to prevent double-free logic (SRAL is essentially a singleton state)
		System(const System&) = delete;
		System& operator=(const System&) = delete;
		System(System&&) = default;
		System& operator=(System&&) = default;

		// -------------------------------------------------------------------------
		// Core Speech Functions
		// -------------------------------------------------------------------------

		void Speak(std::string_view text, bool interrupt = true) {
			Check(SRAL_Speak(text.data(), interrupt), "Speak failed");
		}

		void SpeakSsml(std::string_view ssml, bool interrupt = true) {
			Check(SRAL_SpeakSsml(ssml.data(), interrupt), "SpeakSSML failed");
		}

		void Braille(std::string_view text) {
			Check(SRAL_Braille(text.data()), "Braille output failed");
		}

		void Output(std::string_view text, bool interrupt = true) {
			Check(SRAL_Output(text.data(), interrupt), "Output failed");
		}

		void Stop() {
			SRAL_StopSpeech();
		}

		void Pause() {
			SRAL_PauseSpeech();
		}

		void Resume() {
			SRAL_ResumeSpeech();
		}

		[[nodiscard]] bool IsSpeaking() const {
			return SRAL_IsSpeaking();
		}

		// -------------------------------------------------------------------------
		// Audio / Memory
		// -------------------------------------------------------------------------

		[[nodiscard]] AudioBuffer SpeakToMemory(std::string_view text) {
			uint64_t size = 0;
			int channels = 0;
			int rate = 0;
			int bits = 0;

			void* raw_pointer = SRAL_SpeakToMemory(text.data(), &size, &channels, &rate, &bits);

			if (!raw_pointer) {
				throw Exception("SpeakToMemory returned null");
			}

			AudioBuffer buffer;
			buffer.channels = channels;
			buffer.sample_rate = rate;
			buffer.bits_per_sample = bits;

			try {
				const uint8_t* byte_pointer = static_cast<const uint8_t*>(raw_pointer);
				buffer.data.assign(byte_pointer, byte_pointer + size);
			} catch (...) {
				SRAL_free(raw_pointer);
				throw;
			}

			SRAL_free(raw_pointer);
			return buffer;
		}

		// -------------------------------------------------------------------------
		// Engine Management
		// -------------------------------------------------------------------------

		[[nodiscard]] int GetCurrentEngineId() const {
			return SRAL_GetCurrentEngine();
		}

		[[nodiscard]] std::string GetEngineName(int engine_id) const {
			const char* name = SRAL_GetEngineName(engine_id);
			return name ? std::string(name) : std::string("Unknown");
		}

		[[nodiscard]] int GetEngineFeatures(int engine_id = 0) const {
			return SRAL_GetEngineFeatures(engine_id);
		}

		[[nodiscard]] bool HasFeature(int engine_id, SRAL_SupportedFeatures feature) const {
			return (SRAL_GetEngineFeatures(engine_id) & feature) != 0;
		}

		// -------------------------------------------------------------------------
		// Parameters (Template based)
		// -------------------------------------------------------------------------

		template <typename T>
		void SetParameter(int engine_id, SRAL_EngineParams param, T value) {
			if constexpr (std::is_same_v<T, bool>) {
				// API expects boolean as int usually in C void* polymorphism
				int val = value ? 1 : 0;
				Check(SRAL_SetEngineParameter(engine_id, param, &val), "SetEngineParameter failed");
			}
			else {
				Check(SRAL_SetEngineParameter(engine_id, param, &value), "SetEngineParameter failed");
			}
		}

		template <typename T>
		void SetParameter(SRAL_EngineParams param, T value) {
			SetParameter(GetCurrentEngineId(), param, value);
		}

		template <typename T>
		[[nodiscard]] T GetParameter(int engine_id, SRAL_EngineParams param) const {
			T value{};
			Check(SRAL_GetEngineParameter(engine_id, param, &value), "GetEngineParameter failed");
			return value;
		}

		template <typename T>
		[[nodiscard]] T GetParameter(SRAL_EngineParams param) const {
			return GetParameter<T>(GetCurrentEngineId(), param);
		}

		// -------------------------------------------------------------------------
		// Voice Management
		// -------------------------------------------------------------------------

		[[nodiscard]] std::vector<Voice> GetVoices(int engine_id) const {
			std::vector<Voice> result;

			int count = 0;
			if (!SRAL_GetEngineParameter(engine_id, SRAL_PARAM_VOICE_COUNT, &count) || count <= 0) {
				return result; 
			}

			SRAL_VoiceInfo* raw_voice_array = new SRAL_VoiceInfo[count];

			if (!SRAL_GetEngineParameter(engine_id, SRAL_PARAM_VOICE_PROPERTIES, &raw_voice_array)) {
				return result;
			}

			if (!raw_voice_array) return result;

			result.reserve(count);
			for (int i = 0; i < count; ++i) {
				result.emplace_back(raw_voice_array[i]);
			}

			SRAL_free(raw_voice_array);

			return result;
		}

		[[nodiscard]] std::vector<Voice> GetVoices() const {
			return GetVoices(GetCurrentEngineId());
		}

		// -------------------------------------------------------------------------
		// Extended Engine Control
		// -------------------------------------------------------------------------

		class EngineProxy final {
			int id;
			System& sys;
		public:
			EngineProxy(int engine_id, System& system) : id(engine_id), sys(system) {}

			void Speak(std::string_view text, bool interrupt = true) {
				Check(SRAL_SpeakEx(id, text.data(), interrupt), "Speak failed");
			}

			void SpeakSsml(std::string_view ssml, bool interrupt = true) {
				Check(SRAL_SpeakSsmlEx(id, ssml.data(), interrupt), "SpeakSSML failed");
			}

			void Braille(std::string_view text) {
				Check(SRAL_BrailleEx(id, text.data()), "Braille output failed");
			}

			void Output(std::string_view text, bool interrupt = true) {
				Check(SRAL_OutputEx(id, text.data(), interrupt), "Output failed");
			}

			void Stop() {
				SRAL_StopSpeechEx(id);
			}

			void Pause() {
				SRAL_PauseSpeechEx(id);
			}

			void Resume() {
				SRAL_ResumeSpeechEx(id);
			}

			[[nodiscard]] bool IsSpeaking() const {
				return SRAL_IsSpeakingEx(id);
			}

			[[nodiscard]] AudioBuffer SpeakToMemory(std::string_view text) {
				uint64_t size = 0;
				int channels = 0;
				int rate = 0;
				int bits = 0;

				void* raw_pointer = SRAL_SpeakToMemoryEx(id, text.data(), &size, &channels, &rate, &bits);

				if (!raw_pointer) {
					throw Exception("SpeakToMemory returned null");
				}

				AudioBuffer buffer;
				buffer.channels = channels;
				buffer.sample_rate = rate;
				buffer.bits_per_sample = bits;

				try {
					const uint8_t* byte_pointer = static_cast<const uint8_t*>(raw_pointer);
					buffer.data.assign(byte_pointer, byte_pointer + size);
				} catch (...) {
					SRAL_free(raw_pointer);
					throw;
				}

				SRAL_free(raw_pointer);
				return buffer;
			}

		};

		// Returns a lightweight proxy object to control a specific engine
		EngineProxy GetEngine(int engine_id) {
			return EngineProxy(engine_id, *this);
		}

		// -------------------------------------------------------------------------
		// Global Hooks & Utils
		// -------------------------------------------------------------------------

		void Delay(int ms) {
			SRAL_Delay(ms);
		}

		void RegisterKeyboardHooks() {
			Check(SRAL_RegisterKeyboardHooks(), "Failed to register keyboard hooks");
		}

		void UnregisterKeyboardHooks() {
			SRAL_UnregisterKeyboardHooks();
		}

		[[nodiscard]] int GetAvailableEngines() {
			return SRAL_GetAvailableEngines();
		}

		[[nodiscard]] int GetActiveEngines() {
			return SRAL_GetActiveEngines();
		}
	};

} // extern "C++"
} // namespace Sral

#endif // SRAL_CPP_HPP
