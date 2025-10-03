from enum import IntEnum
import ctypes
import os

# --- Load the SRAL C Library ---
try:
    if os.name == 'nt':
        _sral_lib = ctypes.CDLL('./SRAL.dll')
    elif sys.platform == 'darwin':
        _sral_lib = ctypes.CDLL('./libSRAL.dylib')  # macOS uses .dylib
    else:
        _sral_lib = ctypes.CDLL('./libSRAL.so')
except OSError as e:
    print(f"Error loading SRAL library: {e}")
    _sral_lib = None


class SRALEngine(IntEnum):
    """
    Defines bit flags representing various accessibility engines.
    """
    NONE = 0
    NVDA = 1 << 1
    JAWS = 1 << 2
    ZDSR = 1 << 3
    NARRATOR = 1 << 4
    UIA = 1 << 5
    SAPI = 1 << 6
    SPEECH_DISPATCHER = 1 << 7
    VOICE_OVER = 1 << 8
    AV_SPEECH = 1 << 9

class SRALFeature(IntEnum):
    """
    Enumeration of supported features in the engines.
    """
    SPEECH = 1 << 1
    BRAILLE = 1 << 2
    SPEECH_RATE = 1 << 3
    SPEECH_VOLUME = 1 << 4
    SELECT_VOICE = 1 << 5
    PAUSE_SPEECH = 1 << 6
    SSML = 1 << 7
    SPEAK_TO_MEMORY = 1 << 8
    SPELLING = 1 << 9

class SRALParam(IntEnum):
    """
    Enumeration of engine parameters.
    """
    SPEECH_RATE = 0
    SPEECH_VOLUME = 1
    VOICE_INDEX = 2
    VOICE_PROPERTIES = 3
    VOICE_COUNT = 4
    SYMBOL_LEVEL = 5
    SAPI_TRIM_THRESHOLD = 6
    ENABLE_SPELLING = 7
    USE_CHARACTER_DESCRIPTIONS = 8
    NVDA_IS_CONTROL_EX = 9


class SRALVoiceInfo(ctypes.Structure):
    """
    Voice information values.
    """
    _fields_ = [
        ("index", ctypes.c_int),
        ("name", ctypes.c_char_p),
        ("language", ctypes.c_char_p),
        ("gender", ctypes.c_char_p),
        ("vendor", ctypes.c_char_p),
    ]

    def __repr__(self):
        return (f"SRALVoiceInfo(index={self.index}, name='{self.name.decode('utf-8') if self.name else 'N/A'}',"
                f" language='{self.language.decode('utf-8') if self.language else 'N/A'}',"
                f" gender='{self.gender.decode('utf-8') if self.gender else 'N/A'}',"
                f" vendor='{self.vendor.decode('utf-8') if self.vendor else 'N/A'}')")

# --- Helper for Memory Management ---
# This class acts as a context manager for memory allocated by SRAL_malloc
class SRALMemory:
    def __init__(self, size_or_ptr, is_ptr=False):
        if is_ptr:
            self._ptr = size_or_ptr
            self._allocated_by_python = False
        else:
            if _sral_lib:
                self._ptr = _sral_lib.SRAL_malloc(size_or_ptr)
                self._allocated_by_python = True
            else:
                self._ptr = None
                self._allocated_by_python = False

    def __enter__(self):
        return self._ptr

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self._ptr and self._allocated_by_python and _sral_lib:
            _sral_lib.SRAL_free(self._ptr)
        self._ptr = None

    @property
    def ptr(self):
        return self._ptr

if _sral_lib:
    _sral_lib.SRAL_malloc.argtypes = [ctypes.c_size_t]
    _sral_lib.SRAL_malloc.restype = ctypes.c_void_p

    _sral_lib.SRAL_free.argtypes = [ctypes.c_void_p]
    _sral_lib.SRAL_free.restype = None

    _sral_lib.SRAL_Speak.argtypes = [ctypes.c_char_p, ctypes.c_bool]
    _sral_lib.SRAL_Speak.restype = ctypes.c_bool

    _sral_lib.SRAL_SpeakToMemory.argtypes = [
        ctypes.c_char_p,
        ctypes.POINTER(ctypes.c_uint64),
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int)
    ]
    _sral_lib.SRAL_SpeakToMemory.restype = ctypes.c_void_p

    _sral_lib.SRAL_SpeakSsml.argtypes = [ctypes.c_char_p, ctypes.c_bool]
    _sral_lib.SRAL_SpeakSsml.restype = ctypes.c_bool

    _sral_lib.SRAL_Braille.argtypes = [ctypes.c_char_p]
    _sral_lib.SRAL_Braille.restype = ctypes.c_bool

    _sral_lib.SRAL_Output.argtypes = [ctypes.c_char_p, ctypes.c_bool]
    _sral_lib.SRAL_Output.restype = ctypes.c_bool

    _sral_lib.SRAL_StopSpeech.argtypes = []
    _sral_lib.SRAL_StopSpeech.restype = ctypes.c_bool

    _sral_lib.SRAL_PauseSpeech.argtypes = []
    _sral_lib.SRAL_PauseSpeech.restype = ctypes.c_bool

    _sral_lib.SRAL_ResumeSpeech.argtypes = []
    _sral_lib.SRAL_ResumeSpeech.restype = ctypes.c_bool

    _sral_lib.SRAL_IsSpeaking.argtypes = []
    _sral_lib.SRAL_IsSpeaking.restype = ctypes.c_bool

    _sral_lib.SRAL_GetCurrentEngine.argtypes = []
    _sral_lib.SRAL_GetCurrentEngine.restype = ctypes.c_int

    _sral_lib.SRAL_GetEngineFeatures.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_GetEngineFeatures.restype = ctypes.c_int

    _sral_lib.SRAL_SetEngineParameter.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_void_p]
    _sral_lib.SRAL_SetEngineParameter.restype = ctypes.c_bool

    _sral_lib.SRAL_GetEngineParameter.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_void_p]
    _sral_lib.SRAL_GetEngineParameter.restype = ctypes.c_bool

    _sral_lib.SRAL_Initialize.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_Initialize.restype = ctypes.c_bool

    _sral_lib.SRAL_Uninitialize.argtypes = []
    _sral_lib.SRAL_Uninitialize.restype = None

    _sral_lib.SRAL_SpeakEx.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_bool]
    _sral_lib.SRAL_SpeakEx.restype = ctypes.c_bool

    _sral_lib.SRAL_SpeakToMemoryEx.argtypes = [
        ctypes.c_int,
        ctypes.c_char_p,
        ctypes.POINTER(ctypes.c_uint64),
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int)
    ]
    _sral_lib.SRAL_SpeakToMemoryEx.restype = ctypes.c_void_p

    _sral_lib.SRAL_SpeakSsmlEx.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_bool]
    _sral_lib.SRAL_SpeakSsmlEx.restype = ctypes.c_bool

    _sral_lib.SRAL_BrailleEx.argtypes = [ctypes.c_int, ctypes.c_char_p]
    _sral_lib.SRAL_BrailleEx.restype = ctypes.c_bool

    _sral_lib.SRAL_OutputEx.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_bool]
    _sral_lib.SRAL_OutputEx.restype = ctypes.c_bool

    _sral_lib.SRAL_StopSpeechEx.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_StopSpeechEx.restype = ctypes.c_bool

    _sral_lib.SRAL_PauseSpeechEx.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_PauseSpeechEx.restype = ctypes.c_bool

    _sral_lib.SRAL_ResumeSpeechEx.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_ResumeSpeechEx.restype = ctypes.c_bool

    _sral_lib.SRAL_IsSpeakingEx.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_IsSpeakingEx.restype = ctypes.c_bool

    _sral_lib.SRAL_IsInitialized.argtypes = []
    _sral_lib.SRAL_IsInitialized.restype = ctypes.c_bool

    _sral_lib.SRAL_Delay.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_Delay.restype = None

    _sral_lib.SRAL_RegisterKeyboardHooks.argtypes = []
    _sral_lib.SRAL_RegisterKeyboardHooks.restype = ctypes.c_bool

    _sral_lib.SRAL_UnregisterKeyboardHooks.argtypes = []
    _sral_lib.SRAL_UnregisterKeyboardHooks.restype = None

    _sral_lib.SRAL_GetAvailableEngines.argtypes = []
    _sral_lib.SRAL_GetAvailableEngines.restype = ctypes.c_int

    _sral_lib.SRAL_GetActiveEngines.argtypes = []
    _sral_lib.SRAL_GetActiveEngines.restype = ctypes.c_int

    _sral_lib.SRAL_GetEngineName.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_GetEngineName.restype = ctypes.c_char_p

    _sral_lib.SRAL_SetEnginesExclude.argtypes = [ctypes.c_int]
    _sral_lib.SRAL_SetEnginesExclude.restype = ctypes.c_bool

    _sral_lib.SRAL_GetEnginesExclude.argtypes = []
    _sral_lib.SRAL_GetEnginesExclude.restype = ctypes.c_int


class SRAL:
    """
    A Python wrapper for the Screen Reader Abstraction Library (SRAL).

    """
    def __init__(self):
        if not _sral_lib:
            raise RuntimeError("SRAL C library not loaded.")

    @staticmethod
    def _check_initialized():
        if not SRAL.is_initialized():
            print("Warning: SRAL is not initialized. Call SRAL.initialize() first.")

    # --- Memory Management ---
    @staticmethod
    def sral_malloc(size: int) -> SRALMemory:
        """
        Allocate memory using SRAL's internal allocator.
        The returned SRALMemory object can be used as a context manager
        to ensure memory is freed.

        Args:
            size: The size in bytes to allocate.

        Returns:
            An SRALMemory object containing a pointer to the allocated memory.
        """
        if not _sral_lib: return None
        return SRALMemory(size)

    @staticmethod
    def sral_free(ptr: ctypes.c_void_p):
        """
        Free memory allocated by SRAL_malloc.
        Typically, you'd use SRALMemory as a context manager and not call this directly.

        Args:
            ptr: A ctypes void pointer to the memory to free.
        """
        if not _sral_lib: return
        _sral_lib.SRAL_free(ptr)

    # --- Core Functions (Auto-updating engine) ---

    def speak(self, text: str, interrupt: bool = True) -> bool:
        """
        Speak the given text using the currently active engine.

        Args:
            text: The string to be spoken.
            interrupt: If True, stop current speech before speaking new text.

        Returns:
            True if speaking was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_Speak(text.encode('utf-8'), interrupt)

    def speak_to_memory(self, text: str) -> tuple[memoryview | None, int, int, int]:
        """
        Speak the given text into a PCM audio buffer in memory.

        Args:
            text: The string to be spoken.

        Returns:
            A tuple containing:
            - A memoryview object of the PCM buffer (or None if failed).
            - The buffer size in bytes.
            - The number of channels.
            - The sample rate in HZ.
            - The bits per sample.
            The memoryview is managed by a context manager and will be freed upon exit.
        """
        self._check_initialized()
        if not _sral_lib: return None, 0, 0, 0

        buffer_size = ctypes.c_uint64(0)
        channels = ctypes.c_int(0)
        sample_rate = ctypes.c_int(0)
        bits_per_sample = ctypes.c_int(0)

        with SRALMemory(
            _sral_lib.SRAL_SpeakToMemory(
                text.encode('utf-8'),
                ctypes.byref(buffer_size),
                ctypes.byref(channels),
                ctypes.byref(sample_rate),
                ctypes.byref(bits_per_sample)
            ), is_ptr=True
        ) as pcm_buffer_ptr:
            if pcm_buffer_ptr:
                mem_view = memoryview(
                    (ctypes.c_char * buffer_size.value).from_address(pcm_buffer_ptr)
                )
                return mem_view, buffer_size.value, channels.value, sample_rate.value, bits_per_sample.value
            return None, 0, 0, 0, 0


    def speak_ssml(self, ssml: str, interrupt: bool = True) -> bool:
        """
        Speak the given text using SSML tags with the currently active engine.

        Args:
            ssml: The SSML string to be spoken.
            interrupt: If True, stop current speech before speaking new text.

        Returns:
            True if speaking was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_SpeakSsml(ssml.encode('utf-8'), interrupt)

    def braille(self, text: str) -> bool:
        """
        Output text to a Braille display using the currently active engine.

        Args:
            text: The string to be output in Braille.

        Returns:
            True if Braille output was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_Braille(text.encode('utf-8'))

    def output(self, text: str, interrupt: bool = True) -> bool:
        """
        Output text using all currently supported speech engine methods (speech and/or braille).

        Args:
            text: The string to be output.
            interrupt: If True, stop current speech before speaking new text.

        Returns:
            True if output was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_Output(text.encode('utf-8'), interrupt)

    def stop_speech(self) -> bool:
        """
        Stop speech if it is active for the currently active engine.

        Returns:
            True if speech was stopped successfully, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_StopSpeech()

    def pause_speech(self) -> bool:
        """
        Pause speech if it is active and the current speech engine supports this.

        Returns:
            True if speech was paused successfully, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_PauseSpeech()

    def resume_speech(self) -> bool:
        """
        Resume speech if it was active and the current speech engine supports this.

        Returns:
            True if speech was resumed successfully, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_ResumeSpeech()

    def is_speaking(self) -> bool:
        """
        Get status: is the currently active engine speaking now?

        Returns:
            True if the engine is currently speaking, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_IsSpeaking()

    def get_current_engine(self) -> SRALEngine:
        """
        Get the current speech engine in use.

        Returns:
            The identifier of the current speech engine as an SRALEngine enum.
        """
        self._check_initialized()
        if not _sral_lib: return SRALEngine.NONE
        return SRALEngine(_sral_lib.SRAL_GetCurrentEngine())

    def get_engine_features(self, engine: SRALEngine = SRALEngine.NONE) -> int:
        """
        Get features supported by the specified engine.

        Args:
            engine: The identifier of the engine to query. Defaults to SRALEngine.NONE
                    (current engine).

        Returns:
            An integer representing the features supported by the engine as a bitmask
            of SRALFeature enums.
        """
        self._check_initialized()
        if not _sral_lib: return 0
        return _sral_lib.SRAL_GetEngineFeatures(engine.value)

    def set_engine_parameter(self, engine: SRALEngine, param: SRALParam, value: any) -> bool:
        """
        Set a parameter for the specified speech engine.

        Args:
            engine: The engine to set the parameter for.
            param: The desired parameter as an SRALParam enum.
            value: The value to set (int, bool, or a specific structure/pointer for advanced cases).

        Returns:
            True if the parameter was set successfully, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        c_value = None
        if isinstance(value, bool):
            c_value = ctypes.byref(ctypes.c_bool(value))
        elif isinstance(value, int):
            c_value = ctypes.byref(ctypes.c_int(value))
        elif isinstance(value, str):
             c_value = ctypes.byref(ctypes.c_char_p(value.encode('utf-8')))
        else:
            print(f"Warning: Unsupported parameter value type for set_engine_parameter: {type(value)}")
            return False

        return _sral_lib.SRAL_SetEngineParameter(engine.value, param.value, c_value)

    def get_engine_parameter(self, engine: SRALEngine, param: SRALParam) -> any:
        """
        Get a parameter for the specified speech engine.

        Args:
            engine: The engine to get the parameter for.
            param: The desired parameter as an SRALParam enum.

        Returns:
            The retrieved value, or None if unsuccessful.
            Special handling for VOICE_COUNT and VOICE_PROPERTIES.
        """
        self._check_initialized()
        if not _sral_lib: return None

        if param == SRALParam.VOICE_COUNT:
            count_val = ctypes.c_int()
            success = _sral_lib.SRAL_GetEngineParameter(engine.value, param.value, ctypes.byref(count_val))
            return count_val.value if success else None
        elif param == SRALParam.VOICE_PROPERTIES:
            voice_count = self.get_engine_parameter(engine, SRALParam.VOICE_COUNT)
            if voice_count is None or voice_count <= 0:
                return []

            voice_info_array_type = SRALVoiceInfo * voice_count
            p_voice_info_array = ctypes.POINTER(voice_info_array_type)()

            success = _sral_lib.SRAL_GetEngineParameter(
                engine.value,
                param.value,
                ctypes.byref(p_voice_info_array)
            )

            if success and p_voice_info_array:
                voices = []
                for i in range(voice_count):
                    voice_c = p_voice_info_array.contents[i]
                    voices.append({
                        "index": voice_c.index,
                        "name": voice_c.name.decode('utf-8') if voice_c.name else None,
                        "language": voice_c.language.decode('utf-8') if voice_c.language else None,
                        "gender": voice_c.gender.decode('utf-8') if voice_c.gender else None,
                        "vendor": voice_c.vendor.decode('utf-8') if voice_c.vendor else None,
                    })
                _sral_lib.SRAL_free(p_voice_info_array)
                return voices
            return []
        else:
            val = ctypes.c_int()
            success = _sral_lib.SRAL_GetEngineParameter(engine.value, param.value, ctypes.byref(val))
            if success:
                if param in [SRALParam.ENABLE_SPELLING, SRALParam.USE_CHARACTER_DESCRIPTIONS, SRALParam.NVDA_IS_CONTROL_EX]:
                    return bool(val.value)
                return val.value
            return None

    def initialize(self, engines_exclude: int = 0) -> bool:
        """
        Initialize the library and optionally exclude certain engines from auto-update.

        Args:
            engines_exclude: A bitmask of SRALEngine enums specifying engines to exclude.
                             Defaults to 0 (include all).

        Returns:
            True if initialization was successful, False otherwise.
        """
        if not _sral_lib: return False
        return _sral_lib.SRAL_Initialize(engines_exclude)

    def uninitialize(self):
        """
        Uninitialize the library, freeing resources.
        """
        if not _sral_lib: return
        _sral_lib.SRAL_Uninitialize()

    # --- Extended Functions (Specific engine operations) ---

    def speak_ex(self, engine: SRALEngine, text: str, interrupt: bool = True) -> bool:
        """
        Speak the given text with the specified engine.

        Args:
            engine: The engine to use for speaking.
            text: The string to be spoken.
            interrupt: If True, stop current speech before speaking new text.

        Returns:
            True if speaking was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_SpeakEx(engine.value, text.encode('utf-8'), interrupt)

    def speak_to_memory_ex(self, engine: SRALEngine, text: str) -> tuple[memoryview | None, int, int, int]:
        """
        Speak the given text into a PCM audio buffer in memory using a specific engine.

        Args:
            engine: The engine to use for speaking.
            text: The string to be spoken.

        Returns:
            A tuple containing:
            - A memoryview object of the PCM buffer (or None if failed).
            - The buffer size in bytes.
            - The number of channels.
            - The sample rate in HZ.
            - The bits per sample.
            The memoryview is managed by a context manager and will be freed upon exit.
        """
        self._check_initialized()
        if not _sral_lib: return None, 0, 0, 0

        buffer_size = ctypes.c_uint64(0)
        channels = ctypes.c_int(0)
        sample_rate = ctypes.c_int(0)
        bits_per_sample = ctypes.c_int(0)

        with SRALMemory(
            _sral_lib.SRAL_SpeakToMemoryEx(
                engine.value,
                text.encode('utf-8'),
                ctypes.byref(buffer_size),
                ctypes.byref(channels),
                ctypes.byref(sample_rate),
                ctypes.byref(bits_per_sample)
            ), is_ptr=True
        ) as pcm_buffer_ptr:
            if pcm_buffer_ptr:
                mem_view = memoryview(
                    (ctypes.c_char * buffer_size.value).from_address(pcm_buffer_ptr)
                )
                return mem_view, buffer_size.value, channels.value, sample_rate.value, bits_per_sample.value
            return None, 0, 0, 0, 0


    def speak_ssml_ex(self, engine: SRALEngine, ssml: str, interrupt: bool = True) -> bool:
        """
        Speak the given text using SSML tags with the specified engine.

        Args:
            engine: The engine to use for speaking.
            ssml: The SSML string to be spoken.
            interrupt: If True, stop current speech before speaking new text.

        Returns:
            True if speaking was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_SpeakSsmlEx(engine.value, ssml.encode('utf-8'), interrupt)

    def braille_ex(self, engine: SRALEngine, text: str) -> bool:
        """
        Output text to a Braille display using the specified engine.

        Args:
            engine: The engine to use for Braille output.
            text: The string to be output in Braille.

        Returns:
            True if Braille output was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_BrailleEx(engine.value, text.encode('utf-8'))

    def output_ex(self, engine: SRALEngine, text: str, interrupt: bool = True) -> bool:
        """
        Output text using the specified engine (speech and/or braille).

        Args:
            engine: The engine to use for output.
            text: The string to be output.
            interrupt: If True, stop current speech before speaking new text.

        Returns:
            True if output was successful, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_OutputEx(engine.value, text.encode('utf-8'), interrupt)

    def stop_speech_ex(self, engine: SRALEngine) -> bool:
        """
        Stop speech for the specified engine.

        Args:
            engine: The engine to stop speech for.

        Returns:
            True if speech was stopped successfully, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_StopSpeechEx(engine.value)

    def pause_speech_ex(self, engine: SRALEngine) -> bool:
        """
        Pause speech for the specified engine.

        Args:
            engine: The engine to pause speech for.

        Returns:
            True if speech was paused successfully, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_PauseSpeechEx(engine.value)

    def resume_speech_ex(self, engine: SRALEngine) -> bool:
        """
        Resume speech for the specified engine.

        Args:
            engine: The engine to resume speech for.

        Returns:
            True if speech was resumed successfully, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_ResumeSpeechEx(engine.value)

    def is_speaking_ex(self, engine: SRALEngine) -> bool:
        """
        Get status: is the specified engine speaking now?

        Args:
            engine: The engine to get speaking status for.

        Returns:
            True if the engine is currently speaking, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_IsSpeakingEx(engine.value)

    # --- Utility Functions ---

    @staticmethod
    def is_initialized() -> bool:
        """
        Check if the library has been initialized.

        Returns:
            True if the library is initialized, False otherwise.
        """
        if not _sral_lib: return False
        return _sral_lib.SRAL_IsInitialized()

    def delay(self, time_ms: int):
        """
        Delays the next speech or output operation by a given time.

        Args:
            time_ms: Delay time in milliseconds.
        """
        self._check_initialized()
        if not _sral_lib: return
        _sral_lib.SRAL_Delay(time_ms)

    def register_keyboard_hooks(self) -> bool:
        """
        Install speech interruption and pause keyboard hooks for speech engines
        other than screen readers (e.g., SAPI 5, SpeechDispatcher).
        These hooks work globally. (Ctrl - Interrupt, Shift - Pause)

        Returns:
            True if the hooks are successfully installed, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_RegisterKeyboardHooks()

    def unregister_keyboard_hooks(self):
        """
        Uninstall speech interruption and pause keyboard hooks.
        """
        self._check_initialized()
        if not _sral_lib: return
        _sral_lib.SRAL_UnregisterKeyboardHooks()

    def get_available_engines(self) -> int:
        """
        Get all available engines for the current platform.

        Returns:
            A bitmask of SRALEngine enums with available engines.
        """
        self._check_initialized()
        if not _sral_lib: return 0
        return _sral_lib.SRAL_GetAvailableEngines()

    def get_active_engines(self) -> int:
        """
        Get all active engines that can be used.

        Returns:
            A bitmask of SRALEngine enums with active engines.
        """
        self._check_initialized()
        if not _sral_lib: return 0
        return _sral_lib.SRAL_GetActiveEngines()

    def set_engines_exclude(self, engines_exclude: int) -> bool:
        """
        Exclude certain engines from auto-update.
        Args:
            engines_exclude: A bitmask of SRALEngine enums specifying engines to exclude.

        Returns:
            True if excludes was successfully set, False otherwise.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_SetEnginesExclude(engines_exclude)

    def get_engines_exclude(self) -> int:
        """
        Get engines excluded from auto-update.

        Returns:
            A bitmask of SRALEngine enums with excluded engines.
        """
        self._check_initialized()
        if not _sral_lib: return False
        return _sral_lib.SRAL_GetEnginesExclude()

    def get_engine_name(self, engine: SRALEngine) -> str:
        """
        Get the name of the specified engine.

        Args:
            engine: The identifier of the engine to query.

        Returns:
            A string with the name of the engine.
        """
        self._check_initialized()
        if not _sral_lib: return 0
        return _sral_lib.SRAL_GetEngineName(engine.value).decode('utf-8')

