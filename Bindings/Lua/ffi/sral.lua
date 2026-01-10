--[[
Bindings for SRAL (Screen Reader Abstraction Library)
Copyright (c) 2024-2025 M_maker
https://github.com/m1maker/sral
The binding written by Denis Shishkin (Outsidepro Arts)
https://github.com/outsidepro-arts/

This binding requires Lua >= 5.1
and requires any popular FFI library: FFI in LuaJIT, Lua-FFI and cffi. If you know another FFI libraries and these libraries are compatible with current functions calling, just add them into the array of FFI realizations.

This binding is crossplatform.

]] --

local ffi = nil

-- Searching for available popular FFI library
for _, variant in ipairs { "ffi", "lua-ffi", "cffi" } do
	local success, maybeffi = pcall(require, variant)
	if success then
		ffi = maybeffi
		break
	end
end

assert(ffi, "Failed to load FFI library")

local sraldll = ffi.load("SRAL")

assert(sraldll, "Failed to load SRAL library")

-- Some aliases for LuaServer
---@alias pointer userdata

-- The Lua version for realize the compatibility
-- For some reason the standard %d+ does not works in lua 5.4
local version = _VERSION:gsub("%D", "")
---@diagnostic disable-next-line: cast-local-type
version = tonumber(version)

ffi.cdef [[
	enum SRAL_Engines {
		SRAL_ENGINE_NONE = 0,
		SRAL_ENGINE_NVDA = 1 << 1,
		SRAL_ENGINE_JAWS = 1 << 2,
		SRAL_ENGINE_ZDSR = 1 << 3,
		SRAL_ENGINE_NARRATOR = 1 << 4,
		SRAL_ENGINE_UIA = 1 << 5,
		SRAL_ENGINE_SAPI = 1 << 6,
		SRAL_ENGINE_SPEECH_DISPATCHER = 1 << 7,
		SRAL_ENGINE_VOICE_OVER = 1 << 8,
		SRAL_ENGINE_AV_SPEECH = 1 << 9
	};
]]

---@enum (Keys) sralEngines
local sralEngines = {
	none = sraldll.SRAL_ENGINE_NONE,
	nvda = sraldll.SRAL_ENGINE_NVDA,
	jaws = sraldll.SRAL_ENGINE_JAWS,
	zdsr = sraldll.SRAL_ENGINE_ZDSR,
	narrator = sraldll.SRAL_ENGINE_NARRATOR,
	uia = sraldll.SRAL_ENGINE_UIA,
	sapi = sraldll.SRAL_ENGINE_SAPI,
	speech_dispatcher = sraldll.SRAL_ENGINE_SPEECH_DISPATCHER,
	voiceover = sraldll.SRAL_ENGINE_VOICE_OVER,
	av_speech = sraldll.SRAL_ENGINE_AV_SPEECH
}

ffi.cdef [[
	enum SRAL_SupportedFeatures {
		SRAL_SUPPORTS_SPEECH = 1 << 1,
		SRAL_SUPPORTS_BRAILLE = 1 << 2,
		SRAL_SUPPORTS_SPEECH_RATE = 1 << 3,
		SRAL_SUPPORTS_SPEECH_VOLUME = 1 << 4,
		SRAL_SUPPORTS_SELECT_VOICE = 1 << 5,
		SRAL_SUPPORTS_PAUSE_SPEECH = 1 << 6,
		SRAL_SUPPORTS_SSML = 1 << 7,
		SRAL_SUPPORTS_SPEAK_TO_MEMORY = 1 << 8,
		SRAL_SUPPORTS_SPELLING = 1 << 9
	};
]]

---@enum (keys) sralFeatures
local sralFeatures = {
	speech = sraldll.SRAL_SUPPORTS_SPEECH,
	braille = sraldll.SRAL_SUPPORTS_BRAILLE,
	speech_rate = sraldll.SRAL_SUPPORTS_SPEECH_RATE,
	speech_volume = sraldll.SRAL_SUPPORTS_SPEECH_VOLUME,
	select_voice = sraldll.SRAL_SUPPORTS_SELECT_VOICE,
	pause_speech = sraldll.SRAL_SUPPORTS_PAUSE_SPEECH,
	ssml = sraldll.SRAL_SUPPORTS_SSML,
	speak_to_memory = sraldll.SRAL_SUPPORTS_SPEAK_TO_MEMORY,
	spelling = sraldll.SRAL_SUPPORTS_SPELLING
}

ffi.cdef [[
	enum SRAL_EngineParams {
		SRAL_PARAM_SPEECH_RATE,
		SRAL_PARAM_SPEECH_VOLUME,
		SRAL_PARAM_VOICE_INDEX,
		SRAL_PARAM_VOICE_PROPERTIES,
		SRAL_PARAM_VOICE_COUNT,
		SRAL_PARAM_SYMBOL_LEVEL,
		SRAL_PARAM_SAPI_TRIM_THRESHOLD,
		SRAL_PARAM_ENABLE_SPELLING,
		SRAL_PARAM_USE_CHARACTER_DESCRIPTIONS,
		SRAL_PARAM_NVDA_IS_CONTROL_EX
	};
]]


---@enum (keys) sralParams
local sralParams = {
	speech_rate = { id = sraldll.SRAL_PARAM_SPEECH_RATE, type = "int" },
	speech_volume = { id = sraldll.SRAL_PARAM_SPEECH_VOLUME, type = "int" },
	voice_index = { id = sraldll.SRAL_PARAM_VOICE_INDEX, type = "int" },
	voice_properties = { id = sraldll.SRAL_PARAM_VOICE_PROPERTIES, type = "SRAL_VoiceInfo", size = "{voice_count}" },
	voice_count = { id = sraldll.SRAL_PARAM_VOICE_COUNT, type = "int" },
	symbol_level = { id = sraldll.SRAL_PARAM_SYMBOL_LEVEL, type = "int" },
	sapi_trim_treshold = { id = sraldll.SRAL_PARAM_SAPI_TRIM_THRESHOLD, type = "int" },
	enable_spelling = { id = sraldll.SRAL_PARAM_ENABLE_SPELLING, type = "bool" },
	use_character_descriptions = { id = sraldll.SRAL_PARAM_USE_CHARACTER_DESCRIPTIONS, type = "bool" },
	nvda_is_control_ex = { id = sraldll.SRAL_PARAM_NVDA_IS_CONTROL_EX, type = "bool" }
}

ffi.cdef [[
	typedef struct SRAL_VoiceInfo {
		int index;
		const char* name;
		const char* language;
		const char* gender;
		const char* vendor;
	} SRAL_VoiceInfo;
]]

local sral = {}

ffi.cdef "void* SRAL_malloc(unsigned long long size);"

---Allocate the memory.
---@param size number The size in bytes.
---@return pointer a pointer to the allocated buffer, if allocation was successful, false otherwise.
function sral.malloc(size)
	-- We will not wrap this return because it needs for raw native operations
	return sraldll.SRAL_malloc(ffi.cast("size_t", size))
end

ffi.cdef "void SRAL_free(void* memory);"

---Free the allocated memory.
---@param mem pointer A pointer to the allocated memory.
function sral.free(mem)
	sraldll.SRAL_free(mem)
end

ffi.cdef "bool SRAL_Speak(const char* text, bool interrupt);"

---Speak the given text.
---@param text string A text string to be spoken.
---@param interrupt boolean? A flag indicating whether to interrupt the current speech.
---@return boolean true if speaking was successful, false otherwise.
function sral.speak(text, interrupt)
	return sraldll.SRAL_Speak(text, type(interrupt) == "boolean" and interrupt or false)
end

ffi.cdef "void* SRAL_SpeakToMemory(const char* text, unsigned long long* buffer_size, int* channels, int* sample_rate, int* bits_per_sample);"

---Speak the given text into memory.
---@param text string A text string to be spoken.
---@return pointer a pointer to the PCM buffer if speaking was successful, nil otherwise.
---@return number PCM buffer size.
---@return number PCM channel count.
---@return number PCM sample rate in HZ.
---@return number bits_per_sample PCM bit size (floating point or signed integer).
function sral.speakToMemory(text)
	local bufferSize = ffi.new("uint64_t[1]")
	local channels = ffi.new("int[1]")
	local sampleRate = ffi.new("int[1]")
	local bitsPerSample = ffi.new("int[1]")
	local pcmBuffer = sraldll.SRAL_SpeakToMemory(text, bufferSize, channels, sampleRate, bitsPerSample)
	return pcmBuffer,
		ffi.tonumber(bufferSize[0]),
		channels[0],
		sampleRate[0],
		bitsPerSample[0]
end

ffi.cdef "bool SRAL_SpeakSsml(const char* ssml, bool interrupt);"

---Speak the given text using SSML tags.
---@param text string A valid SSML string to be spoken.
---@param interrupt boolean? A flag indicating whether to interrupt the current speech.
---@return boolean true if speaking was successful, false otherwise.
function sral.speakSSML(text, interrupt)
	return sraldll.SRAL_SpeakSsml(text, type(interrupt) == "boolean" and interrupt or false)
end

ffi.cdef "bool SRAL_Braille(const char* text);"

---Output text to a Braille display.
---@param text string A text string to be output in Braille.
---@return boolean true if Braille output was successful, false otherwise.
function sral.braille(text)
	return sraldll.SRAL_Braille(text)
end

ffi.cdef "bool SRAL_Output(const char* text, bool interrupt);"

---Output text using all currently supported speech engine methods.
---@param text string A text string to be output.
---@param interrupt boolean? A flag indicating whether to interrupt the current speech.
---@return boolean true if output was successful, false otherwise.
function sral.output(text, interrupt)
	return sraldll.SRAL_Output(text, type(interrupt) == "boolean" and interrupt or false)
end

ffi.cdef "bool SRAL_StopSpeech(void);"

---Stop the current speech.
---@return boolean true if stop was successful, false otherwise.	
function sral.stopSpeech()
	return sraldll.SRAL_StopSpeech()
end

ffi.cdef "bool SRAL_PauseSpeech(void);"

---Pause speech if it is active and the current speech engine supports this.
---@return boolean true if pause was successful, false otherwise.
function sral.pauseSpeech()
	return sraldll.SRAL_PauseSpeech()
end

ffi.cdef "bool SRAL_ResumeSpeech(void);"

---Resume speech if it was active and the current speech engine supports this.
---@return boolean true if resume was successful, false otherwise.
function sral.resumeSpeech()
	return sraldll.SRAL_ResumeSpeech()
end

ffi.cdef "bool SRAL_IsSpeaking(void);"

---Get status, does this engine speak now.
---@return boolean true if the engine is currently speaking, false otherwise.
function sral.isSpeaking()
	return sraldll.SRAL_IsSpeaking()
end

-- The following functions needed for compatibility between  Lua versions
local function band(v1, v2)
	if version > 52 then
		return v1 & v2
	else
		return (bit or bit32).band(v1, v2)
	end
end

local function bor(v1, v2)
	if version > 52 then
		return v1 | v2
	else
		return (bit or bit32).bor(v1, v2)
	end
end


ffi.cdef "bool SRAL_Initialize(int engines_exclude);"

---Initialize the library and optionally exclude certain engines.
---@param ... sralEngines? An optional string values with engine names specifying engines to exclude from auto update.
---@return boolean true if initialization was successful, false otherwise.
function sral.initialize(...)
	local cEngines = sralEngines.none
	for _, engine in ipairs { ... } do
		assert(sralEngines[engine] , string.format("Engine %s not supported", engine))
		cEngines = bor(cEngines, sralEngines[engine])
	end
	return sraldll.SRAL_Initialize(cEngines)
end

ffi.cdef "void SRAL_Uninitialize(void);"

---Uninitialize the library.
function sral.uninitialize()
	sraldll.SRAL_Uninitialize()
end

ffi.cdef "bool SRAL_IsInitialized(void);"

---Check if the library is initialized.
---@return boolean true if library is initialized, false otherwise
function sral.initialized()
	return sraldll.SRAL_IsInitialized()
end

ffi.cdef "void SRAL_Delay(int time);"

---Puts an delay in speech queue between two text speaking
---@param time number Time in milliseconds
function sral.delay(time)
	sraldll.SRAL_Delay(time)
end

ffi.cdef "bool SRAL_RegisterKeyboardHooks(void);"

---Install speech interruption and pause keyboard hooks for speech engines other than screen readers, such as Microsoft SAPI 5 or SpeechDispatcher.
---These hooks work globally in any window.
---Ctrl - Interrupt, Shift - Pause.
---@return boolean true if the hooks are successfully installed, false otherwise.
function sral.registerKeyboardHooks()
	return sraldll.SRAL_RegisterKeyboardHooks()
end

ffi.cdef "void SRAL_UnregisterKeyboardHooks(void);"

---Uninstall speech interruption and pause keyboard hooks for speech engines other than screen readers, such as Microsoft SAPI 5 or SpeechDispatcher.
function sral.unregisterKeyboardHooks()
	sraldll.SRAL_UnregisterKeyboardHooks()
end

-- All engines definition
-- The engines will be presented as objects with properties and methods for its interraction.

-- The standard engine methods. These methods will be available per every engine.
---@class engine
---@field private index number
---@field name string
---@field params {[string]: [number|table[]]}
local engineMethods = {}


ffi.cdef "bool SRAL_SpeakEx(int engine, const char* text, bool interrupt);"

---Speak the given text with the specified engine.
---@param text string A text string to be spoken.
---@param interrupt boolean? A flag indicating whether to interrupt the current speech.
---@return boolean true if speaking was successful, false otherwise.
function engineMethods:speak(text, interrupt)
	return sraldll.SRAL_SpeakEx(self.index, text, type(interrupt) == "boolean" and interrupt or false)
end

ffi.cdef "void* SRAL_SpeakToMemoryEx(int engine, const char* text, unsigned long long* buffer_size, int* channels, int* sample_rate, int* bits_per_sample);"

---Speak the given text into memory with the specified engine.
---@param text string A text string to be spoken.
---@return pointer a pointer to the PCM buffer if speaking was successful, nil otherwise.
---@return number buffer_size PCM buffer size.
---@return number channels PCM channel count.
---@return number sample_rate PCM sample rate in HZ.
---@return number bits_per_sample PCM bit size (floating point or signed integer).
function engineMethods:speakToMemory(text)
	local bufferSize = ffi.new("uint64_t[1]")
	local channels = ffi.new("int[1]")
	local sampleRate = ffi.new("int[1]")
	local bitsPerSample = ffi.new("int[1]")
	local pcmBuffer = sraldll.SRAL_SpeakToMemoryEx(self.index, text, bufferSize, channels, sampleRate,
		bitsPerSample)
	return pcmBuffer,
		ffi.tonumber(bufferSize[0]),
		channels[0],
		sampleRate[0],
		bitsPerSample[0]
end

ffi.cdef "bool SRAL_SpeakSsmlEx(int engine, const char* ssml, bool interrupt);"

---Speak the given text using SSML tags with the specified engine.
---@param text string A pointer to the valid SSML string to be spoken.
---@param interrupt boolean? A flag indicating whether to interrupt the current speech.
---@return boolean true if speaking was successful, false otherwise.
function engineMethods:speakSSML(text, interrupt)
	return sraldll.SRAL_SpeakSsmlEx(self.index, text, type(interrupt) == "boolean" and interrupt or false)
end

ffi.cdef "bool SRAL_BrailleEx(int engine, const char* text);"

---Output text to a Braille display with the specified engine.
---@param text string A pointer to the text string to be output in Braille.
---@return boolean true if Braille output was successful, false otherwise.
function engineMethods:braille(text)
	return sraldll.SRAL_BrailleEx(self.index, text)
end

ffi.cdef "bool SRAL_OutputEx(int engine, const char* text, bool interrupt);"

---Output text using the specified engine.
---@param text string A text string to be output.
---@param interrupt boolean? A flag indicating whether to interrupt the current speech.
---@return boolean true if output was successful, false otherwise.
function engineMethods:output(text, interrupt)
	return sraldll.SRAL_OutputEx(self.index, text, type(interrupt) == "boolean" and interrupt or false)
end

ffi.cdef "bool SRAL_StopSpeechEx(int engine);"

---Stop the current speech with the specified engine.
---@return boolean true if stop was successful, false otherwise.	
function engineMethods:stopSpeech()
	return sraldll.SRAL_StopSpeechEx(self.index)
end

ffi.cdef "bool SRAL_PauseSpeechEx(int engine);"

---Pause speech if it is active and the current speech engine supports this with the specified engine.
---@return boolean true if pause was successful, false otherwise.
function engineMethods:pauseSpeech()
	return sraldll.SRAL_PauseSpeechEx(self.index)
end

ffi.cdef "bool SRAL_ResumeSpeechEx(int engine);"

---Resume speech if it was active and the current speech engine supports this with the specified engine.
---@return boolean true if resume was successful, false otherwise.
function engineMethods:resumeSpeech()
	return sraldll.SRAL_ResumeSpeechEx(self.index)
end

ffi.cdef "bool SRAL_IsSpeakingEx(int engine);"

---Get status, does this engine speak now with the specified engine.
---@return boolean true if the engine is currently speaking, false otherwise.
function engineMethods:isSpeaking()
	return sraldll.SRAL_IsSpeakingEx(self.index)
end

ffi.cdef "int SRAL_GetEngineFeatures(int engine);"

---returns supported features for the engine
---@return {string : boolean} the table with features in the keys and boolean value if feature is supported
function engineMethods:features()
	local rawFeatures = sraldll.SRAL_GetEngineFeatures(self.index)
	local retval = {}
	for featureName, feature in pairs(sralFeatures) do
		retval[featureName] = band(rawFeatures, feature) ~= 0
	end
	return retval
end

ffi.cdef [[
bool SRAL_SetEngineParameter(int engine, int param, const void* value);
bool SRAL_GetEngineParameter(int engine, int param, void* value);
]]

local function initParams(index)
	local params = setmetatable(
		{
			engine = index
		}, {
			__name = "sral_params",
			__index = function(self, param)
				local paramStruct = sralParams[param]
				if not paramStruct then
					return nil
				end
				local size = paramStruct.size or 1
				if type(size) == "string" and size:find("{.+}") then
					size = size:gsub("{(.+)}", self[size:match("{(.+)}")])
				end
				local paramptr = ffi.new(string.format("%s[%u]", paramStruct.type, size))
				if sraldll.SRAL_GetEngineParameter(self.engine, paramStruct.id, ffi.cast("void*", paramptr)) then
					if paramStruct.id == sralParams.voice_properties.id then
						local voices = setmetatable({}, {
							__index = function(_, i)
								if type(i) == "number" and i > 0 and i < self.voice_count then
									return setmetatable({ __voiceinfo = paramptr[i - 1] }, {
										__index = function(self, key)
											if key == "voice_index" then
												return i - 1
											else
												return ffi
													[ffi.istype("const char*", self.__voiceinfo[key]) and "string" or "tonumber"](
														self
														.__voiceinfo[key])
											end
										end
									})
								end
							end,
							__len = function()
								return self.voice_count
							end
						})
						return voices
					else
						return paramptr[0]
					end
				end
				return nil
			end,
			__newindex = function(self, param, value)
				local paramStruct = assert(sralParams[param], string.format("Parameter %s not supported", param))
				local size = paramStruct.size or 1
				if type(size) == "string" and size:find("{.+}") then
					size = size:gsub("{(.+)}", self.params[size:match("{(.+)}")])
				end
				local cValue = ffi.new(string.format("%s[%u]", paramStruct.type, size), value)
				return select(1,
					assert(sraldll.SRAL_SetEngineParameter(self.engine, paramStruct.id, ffi.cast("const void*", cValue)),
						string.format("Parameter %s could not be changed.", param)))
			end,
			__pairs = function(self)
				return function(t, k)
					local nextT = next(sralParams, k)
					return nextT, self[nextT]
				end, sralParams, nil
			end
		}
	)
	return params
end


ffi.cdef "const char* SRAL_GetEngineName(int engine);"

-- The indexing of engine. Used in the engine metatable.
local function engineIndex(self, key)
	if key == "params" then
		return initParams(self.index)
	elseif key == "name" then
		return ffi.string(sraldll.SRAL_GetEngineName(self.index))
	end
	return engineMethods[key]
end

local function initEngine(engine)
	local e = setmetatable({ index = engine }, { __name = "sral_engine", __type = "sral_engine", __index = engineIndex })
	return e
end


ffi.cdef "int SRAL_GetCurrentEngine(void);"

---Get the current speech engine in use.
---@return engine The current engine object.
function sral.currentEngine()
	return initEngine(sraldll.SRAL_GetCurrentEngine())
end

ffi.cdef "int SRAL_GetAvailableEngines(void);"

---Get all available engines for the current platform.
---@return {[string]: engine} A table with all available engine objects.
function sral.availableEngines()
	local data = sraldll.SRAL_GetAvailableEngines()
	local retval = {}
	for engineName, engine in pairs(sralEngines) do
		if band(data, engine) ~= 0 then
			retval[engineName] = initEngine(engine)
		end
	end
	return retval
end

ffi.cdef "int SRAL_GetActiveEngines(void);"

---Get all active engines that can be used.
---@return {[string]: engine} A table with all active engine objects.
function sral.activeEngines(engine)
	local data = sraldll.SRAL_GetActiveEngines()
	local retval = {}
	for engineName, engine in pairs(sralEngines) do
		if band(data, engine) ~= 0 then
			retval[engineName] = initEngine(engine)
		end
	end
	return retval
end

ffi.cdef "bool SRAL_SetEnginesExclude(int engines_exclude);"

---Set excludes for specified engines
---@param ... sralEngines|engine A list of either engine names or engine objects to be excluded from auto update. No engines means that all engines are included.
---@return boolean true if excludes was successful set, false otherwise.
function sral.setEnginesExclude(...)
	local enginesExclude = sralEngines.none
	for _, engine in ipairs { ... } do
		assert(sralEngines[engine], string.format("Engine %s not supported", engine))
		enginesExclude = bor(enginesExclude, sralEngines[engine])
	end
	return sraldll.SRAL_SetEnginesExclude(enginesExclude)
end

ffi.cdef "int SRAL_GetEnginesExclude(void);"

---Get engines excluded from auto update.
---@return {[string]: engine} A table with all excluded engine objects.
function sral.getEnginesExclude()
	local data = sraldll.SRAL_GetEnginesExclude()
	local retval = {}
	for engineName, engine in pairs(sralEngines) do
		if band(data, engine) ~= 0 then
			retval[engineName] = initEngine(engine)
		end
	end
	return retval
end

---Get specified engine object. This function returns an engine even if it is not active or unavailable in this platform.
---@param engine sralEngines The engine name to get.
---@return engine The engine object.
function sral.getEngine(engine)
	if version >= 54 then
		if not sral.activeEngines()[engine] then
			warn(string.format("The engine %s is not active", engine))
		elseif not sral.availableEngines()[engine] then
			warn(string.format("The engine %s is not available", engine))
		end
	end
	assert(sralEngines[engine], string.format("Engine %s not supported", engine))
	return initEngine(sralEngines[engine])
end

-- Automatic uninitialize the library when the module object has destroyed
setmetatable(sral, { __name = "sral", __gc = sral.uninitialize })

return sral
