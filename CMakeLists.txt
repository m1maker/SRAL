# CMakeList.txt : CMake project for SRAL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SRAL")

# Add source to this project's executable.
add_library(SRAL SHARED "Include/SRAL.h" "SRC/SRAL.cpp" "SRC/ScreenReader.h" "SRC/Encoding.h" "SRC/Encoding.cpp" "SRC/NVDA.h" "SRC/NVDA.cpp" "SRC/SAPI.h" "src/SAPI.cpp" "Dep/blastspeak.h" "Dep/blastspeak.c" "Dep/miniaudio.h" "Dep/miniaudio.c" "Dep/fsapi.h" "Dep/fsapi.c" "SRC/Jaws.h" "SRC/Jaws.cpp" "SRC/SpeechDispatcher.h" "SRC/SpeechDispatcher.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SRAL PROPERTY CXX_STANDARD 20)
endif()

