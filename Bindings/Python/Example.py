import time
from sral import SRAL, SRALEngine, SRALFeature, SRALParam

def main():
    sral = SRAL()
    print("=== SRAL Library Demo ===")
    if not sral.is_initialized():
        print("Initializing SRAL...")
        if not sral.initialize():
            print("Failed to initialize SRAL!")
            return
    
    print("SRAL initialized successfully!")
    
    print("\n=== Engine Information ===")
    
    available_engines = sral.get_available_engines()
    active_engines = sral.get_active_engines()
    excluded_engines = sral.get_engines_exclude()
    
    print(f"Available engines: {available_engines:#x}")
    print(f"Active engines: {active_engines:#x}")
    print(f"Excluded engines: {excluded_engines:#x}")
    
    print("\nAvailable engine names:")
    for engine in SRALEngine:
        if engine != SRALEngine.NONE and available_engines & engine.value:
            engine_name = sral.get_engine_name(engine)
            print(f"  - {engine_name} (Flag: {engine.value:#x})")
    
    print("\n=== Current Engine Operations ===")
    
    current_engine = sral.get_current_engine()
    print(f"Current engine: {current_engine}")
    
    features = sral.get_engine_features()
    print(f"Current engine features: {features:#x}")
    
    if features & SRALFeature.SPEECH.value:
        print("  - Speech support: YES")
    if features & SRALFeature.BRAILLE.value:
        print("  - Braille support: YES")
    if features & SRALFeature.SSML.value:
        print("  - SSML support: YES")
    if features & SRALFeature.SPEAK_TO_MEMORY.value:
        print("  - Speak to memory: YES")
    
    print("\n=== Basic Speech Operations ===")
    
    print("Speaking basic text...")
    sral.speak("Hello, this is a test of the SRAL library.", interrupt=True)
    
    print(f"Is speaking: {sral.is_speaking()}")
    
    time.sleep(2)
    
    print("\n=== Speech Control ===")
    
    sral.speak("This is a long text that we will interrupt and control.", interrupt=True)
    
    time.sleep(1)
    print("Pausing speech...")
    sral.pause_speech()
    
    print(f"Is speaking after pause: {sral.is_speaking()}")
    
    time.sleep(1)
    print("Resuming speech...")
    sral.resume_speech()
    
    time.sleep(1)
    print("Stopping speech...")
    sral.stop_speech()
    
    print("\n=== Engine Parameters ===")
    
    current_rate = sral.get_engine_parameter(current_engine, SRALParam.SPEECH_RATE)
    if current_rate is not None:
        print(f"Current speech rate: {current_rate}")
    
    if features & SRALFeature.SPEECH_RATE.value:
        print("Setting speech rate to 50...")
        sral.set_engine_parameter(current_engine, SRALParam.SPEECH_RATE, 50)
        sral.speak("This is spoken at a different rate.", interrupt=True)
        time.sleep(2)
        
        sral.set_engine_parameter(current_engine, SRALParam.SPEECH_RATE, current_rate)
    
    print("\n=== Voice Management ===")
    
    voice_count = sral.get_engine_parameter(current_engine, SRALParam.VOICE_COUNT)
    if voice_count:
        print(f"Available voices: {voice_count}")
        
        voices = sral.get_engine_parameter(current_engine, SRALParam.VOICE_PROPERTIES)
        if voices:
            print("Available voices:")
            for i, voice in enumerate(voices):
                print(f"  {i+1}. {voice['name']} - {voice['language']} ({voice['gender']})")
            
            if len(voices) > 1:
                print("Switching to second voice...")
                sral.set_engine_parameter(current_engine, SRALParam.VOICE_INDEX, 1)
                sral.speak("This is spoken with a different voice.", interrupt=True)
                time.sleep(2)
                
                sral.set_engine_parameter(current_engine, SRALParam.VOICE_INDEX, 0)
    
    print("\n=== SSML Support ===")
    
    if features & SRALFeature.SSML.value:
        ssml_text = """
        <speak>
            This is <prosody rate="slow">slow speech</prosody>
            and this is <prosody rate="fast">fast speech</prosody>.
            <break time="500ms"/>
            Now with <prosody pitch="high">high pitch</prosody>.
        </speak>
        """
        print("Speaking SSML content...")
        sral.speak_ssml(ssml_text, interrupt=True)
        time.sleep(3)
    else:
        print("SSML not supported by current engine")
    
    print("\n=== Memory Speech ===")
    
    if features & SRALFeature.SPEAK_TO_MEMORY.value:
        print("Generating speech to memory...")
        pcm_data, buffer_size, channels, sample_rate, bits_per_sample = sral.speak_to_memory(
            "This text is converted to PCM audio data."
        )
        
        if pcm_data:
            print(f"Generated PCM data: {buffer_size} bytes")
            print(f"Audio format: {channels} channels, {sample_rate} Hz, {bits_per_sample} bits per sample")
            
        else:
            print("Failed to generate PCM data")
    else:
        print("Speak to memory not supported by current engine")
    
    print("\n=== Braille Output ===")
    
    if features & SRALFeature.BRAILLE.value:
        print("Outputting text to Braille display...")
        sral.braille("Hello Braille World!")
    else:
        print("Braille output not supported by current engine")
    
    print("\n=== Combined Output ===")
    
    print("Using combined output (speech and braille if available)...")
    sral.output("This message goes to both speech and braille outputs.", interrupt=True)
    time.sleep(2)
    
    print("\n=== Specific Engine Operations ===")
    
    for engine in [SRALEngine.NVDA, SRALEngine.JAWS, SRALEngine.SAPI]:
        if available_engines & engine.value:
            engine_name = sral.get_engine_name(engine)
            print(f"Testing {engine_name}...")
            
            engine_features = sral.get_engine_features(engine)
            
            if engine_features & SRALFeature.SPEECH.value:
                sral.speak_ex(engine, f"Hello from {engine_name}", interrupt=True)
                time.sleep(2)
                
                print(f"{engine_name} is speaking: {sral.is_speaking_ex(engine)}")
    
    print("\n=== Keyboard Hooks ===")
    
    print("Registering keyboard hooks for global control...")
    if sral.register_keyboard_hooks():
        print("Keyboard hooks registered. Use Ctrl to interrupt, Shift to pause speech globally.")
        print("Speaking a long text - try the keyboard controls...")
        sral.speak("This is a long text. You can now use Ctrl to interrupt me or Shift to pause and resume.", interrupt=True)
        
        time.sleep(4)
        
        print("Unregistering keyboard hooks...")
        sral.unregister_keyboard_hooks()
    else:
        print("Failed to register keyboard hooks")
    
    print("\n=== Engine Exclusion ===")
    
    exclude_mask = SRALEngine.SAPI.value | SRALEngine.NARRATOR.value
    print(f"Setting engine exclude mask: {exclude_mask:#x}")
    if sral.set_engines_exclude(exclude_mask):
        print("Engine exclusion updated")
    
    print("\n=== Delay Function ===")
    
    print("Testing delay function...")
    sral.speak("First message.", interrupt=True)
    sral.delay(1000)
    sral.speak("Second message after delay.", interrupt=True)
    time.sleep(2)
    
    print("\n=== Advanced Parameter Settings ===")
    
    if sral.set_engine_parameter(current_engine, SRALParam.ENABLE_SPELLING, True):
        print("Spelling mode enabled")
        sral.speak("This will be spelled out character by character.", interrupt=True)
        time.sleep(3)
        sral.set_engine_parameter(current_engine, SRALParam.ENABLE_SPELLING, False)
    
    print("\n=== Cleanup ===")
    
    sral.speak("SRAL demonstration completed. Thank you!", interrupt=True)
    time.sleep(2)
    
    print("Uninitializing SRAL...")
    sral.uninitialize()
    print("SRAL uninitialized successfully!")

def error_handling_demo():
    """Demonstrate error handling scenarios"""
    print("\n=== Error Handling Demo ===")
    
    try:
        sral = SRAL()
        
        print("Attempting operation without initialization...")
        result = sral.speak("This should fail", interrupt=True)
        print(f"Result: {result} (should be False)")
        
    except Exception as e:
        print(f"Error caught: {e}")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Demo failed with error: {e}")
    
    error_handling_demo()
