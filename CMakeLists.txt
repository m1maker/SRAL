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
add_library(${CMAKE_PROJECT_NAME}_obj OBJECT  "Include/SRAL.h" "SRC/SRAL.cpp" "SRC/Engine.h" "SRC/Encoding.h" "SRC/Encoding.cpp" "SRC/NVDA.h" "SRC/NVDA.cpp" "SRC/SAPI.h" "SRC/SAPI.cpp" "Dep/blastspeak.h" "Dep/blastspeak.c" "Dep/miniaudio.h" "Dep/fsapi.h" "Dep/fsapi.c" "SRC/Jaws.h" "SRC/Jaws.cpp" "SRC/SpeechDispatcher.h" "SRC/SpeechDispatcher.cpp" "SRC/UIA.cpp" "Dep/UIAProvider.h" "Dep/UIAProvider.cpp")
set_property(TARGET ${CMAKE_PROJECT_NAME}_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(${CMAKE_PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${CMAKE_PROJECT_NAME}_obj>)
set_property(TARGET ${CMAKE_PROJECT_NAME}_static PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(${CMAKE_PROJECT_NAME} SHARED $<TARGET_OBJECTS:${CMAKE_PROJECT_NAME}_obj>)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")

if (WIN32)
  target_link_libraries(${CMAKE_PROJECT_NAME} "uiautomationcore.lib")
else()
  set_source_files_properties("SRC/Encoding.h" "SRC/Encoding.cpp" "SRC/NVDA.h" "SRC/NVDA.cpp" "SRC/SAPI.h" "SRC/SAPI.cpp" "Dep/blastspeak.h" "Dep/blastspeak.c" "Dep/miniaudio.h" "Dep/fsapi.h" "Dep/fsapi.c" "SRC/Jaws.h" "SRC/Jaws.cpp" "SRC/UIA.cpp" "Dep/UIAProvider.h" "Dep/UIAProvider.cpp" PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()
