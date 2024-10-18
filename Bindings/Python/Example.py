import time

import sral

def sleep_ms(milliseconds):
    time.sleep(milliseconds / 1000.0)  # Convert milliseconds to seconds

def main():
    text = ""
    # Initialize the SRAL library
    instance = sral.Sral(32)

    instance.register_keyboard_hooks()

    # Speak some text
    if instance.get_engine_features(0) & 128:
        text = input("Enter the text you want to be spoken:\n")
        instance.speak(text, False)

    # Output text to a Braille display
    if instance.get_engine_features(0) & 256:
        text = input("Enter the text you want to be shown on braille display:\n")
        instance.braille(text)

    # Delay example
    instance.output("Delay example: Enter any text", False)
    instance.delay(5000)
    instance.output("Press enter to continue", False)
    input()  # Wait for user to press enter

    instance.stop_speech()  # Stops the delay thread

    # Speech rate
    if instance.get_engine_features(0) & 512:
        rate = instance.get_rate()
        max_rate = rate + 10
        for rate in range(rate, max_rate):
            instance.set_rate(rate)
            instance.speak(text, False)
            sleep_ms(500)

    # Uninitialize the SRAL library
    instance = None
if __name__ == "__main__":
    main() # invoke_main()

