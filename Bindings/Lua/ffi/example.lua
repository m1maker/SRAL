package.path = ".\\?.lua"


function printcheck(msg, state)
	print(msg, state and "success" or "failed")
end

sral = require"sral"

print("SRAL library demonstration")

printcheck("Initializing SRAL:", sral.initialize())

if not sral.initialized() then os.exit(1) end


features = {}

function features.speech_rate(engine)
	print("\tCurrent speech rate:", engine.params.speech_rate)
end

function features.speech_volume(engine)
	print("\tCurrent speech volume:", engine.params.speech_volume)
end

function features.select_voice(engine)
	print(string.format("\tThere are %u voices.", engine.params.voice_count))
	print("\tAvailable voices:")
	for _, voice in ipairs(engine.params.voice_properties) do
		print("", "",
			voice.name,
			string.format("Language: %s", voice.language or "not provided"),
			string.format("Gender: %s", voice.gender or "not provided"),
			string.format("Vendor: %s", voice.vendor or "not provided"),
			engine.params.voice_index == voice.voice_index and "Default set voice" or ""
		)
	end
	print("\tCurrently selected voice:", engine.params.voice_properties[engine.params.voice_index + 1].name)
end

function printFeatures(engine)
	print("This engine supports the following features:")
	for featureName, state in pairs(engine:features()) do
		if state then
			print("", featureName)
			if features[featureName] then
				features[featureName](engine)
			end
		end
	end
end

print("Available engines:")
for _, engine in pairs(sral.availableEngines()) do
	print(engine.name)
	printFeatures(engine)
end

print("Active engines:")
for _, engine in pairs(sral.activeEngines()) do
	print(engine.name)
	printFeatures(engine)
end

currentEngine = sral.currentEngine()
print("Currently active engine:", currentEngine.name)
printFeatures(currentEngine)

print("We're finished the first stage. Press enter to continue.")
io.read()

printcheck("Attempt to say using common output method", sral.output("This is the common output message"))
print("Press Enter to continue.")
io.read()
printcheck("Attempt to say the message using speech methods", sral.speak("This is the speech testing message", false))
print("Press Enter to continue.")
io.read()
printcheck("attempt to output a  message to Braille display", sral.braille("This is the Braille message"))
print("We're finished the second stage of the demonstration. Press Enter to continue")
io.read()

sapi = sral.activeEngines().sapi
if sapi then
	printcheck("Attempt to say something using Windows SAPI 5 with default parameters", sapi:speak("This is the SAPI 5 speech message."))
	sapi.params.speech_rate = -4
	printcheck(string.format("Attempt to say something with speech rate %i", sapi.params.speech_rate), sapi:output("This is changed speech rate text."))
	print("Press Enter to continue.")
	io.read()
end

print("End SRAL demonstration script")
os.exit(0)