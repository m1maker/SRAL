# CMakeList.txt : CMake project for SRAL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(INCLUDES "Include")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SRAL")
add_library(${PROJECT_NAME}_obj OBJECT)
target_sources(${PROJECT_NAME}_obj PRIVATE
  "SRC/SRAL.cpp" "SRC/Engine.h")
target_sources(${PROJECT_NAME}_obj PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${INCLUDES}"
  FILES "${INCLUDES}/SRAL.h")
if(WIN32)
  target_sources(${PROJECT_NAME}_obj PRIVATE
    "SRC/Encoding.h" "SRC/Encoding.cpp"
    "SRC/NVDA.h" "SRC/NVDA.cpp" "SRC/SAPI.h" "SRC/SAPI.cpp"
    "Dep/blastspeak.h" "Dep/blastspeak.c" "Dep/fsapi.h" "Dep/fsapi.c"
    "SRC/Jaws.h" "SRC/Jaws.cpp" "SRC/UIA.cpp"
    "Dep/UIAProvider.h" "Dep/UIAProvider.cpp" "Dep/wasapi.h" "Dep/wasapi.cpp")
elseif(APPLE)
  target_sources(${PROJECT_NAME}_obj PRIVATE
    "SRC/AVSpeech.h" "SRC/AVSpeech.mm")
  set_source_files_properties(
    "SRC/AVSpeech.h"
    "SRC/AVSpeech.mm"
    PROPERTIES
    LANGUAGE Objective-CXX
  )
else()
  target_sources(${PROJECT_NAME}_obj PRIVATE
    "SRC/SpeechDispatcher.h" "SRC/SpeechDispatcher.cpp")
endif()

set_property(TARGET ${PROJECT_NAME}_obj
  PROPERTY POSITION_INDEPENDENT_CODE on)

add_library(${PROJECT_NAME} SHARED
  $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
add_library(${PROJECT_NAME}_static STATIC
  $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_include_directories(${PROJECT_NAME}_static PUBLIC ${INCLUDES})

install(TARGETS ${PROJECT_NAME}_obj FILE_SET HEADERS
  INCLUDES DESTINATION "include")

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "bin"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib")
install(TARGETS ${PROJECT_NAME}_static DESTINATION "lib")

if (WIN32)
  set(LIBS "uiautomationcore.lib")
  target_link_libraries(${PROJECT_NAME} ${LIBS})
  target_link_libraries(${PROJECT_NAME}_static ${LIBS})
elseif (APPLE)
  enable_language(OBJCXX)
  target_link_libraries(${PROJECT_NAME} "-framework Foundation" "-framework AVFoundation")
endif()
