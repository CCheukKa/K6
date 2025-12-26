cmake_minimum_required(VERSION 3.20)
project(K6 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_definitions(-DUNICODE -D_UNICODE)

if (MSVC)
    add_compile_options(/utf-8)
endif()

# Build a COM in-proc server (DLL)
add_library(K6 SHARED
    src/dllmain.cpp
    src/ClassFactory.cpp
    src/ClassFactory.h
    src/TextService.cpp
    src/TextService.h
    src/EditSession.cpp
    src/EditSession.h
    src/CandidateWindow.cpp
    src/CandidateWindow.h
    src/IndicatorWindow.cpp
    src/IndicatorWindow.h
    src/Dictionary.cpp
    src/Dictionary.h
    src/Suggestions.cpp
    src/Suggestions.h
    src/Punctuation.cpp
    src/Punctuation.h
    src/Stroke.h
    src/guid.h
    src/Registration.cpp
    src/Registration.h
    src/InputStateMachine.cpp
    src/InputStateMachine.h
    src/Debug.cpp
    src/Debug.h
    resources/resource.rc
)

# Link with required Windows libs
# ole32 for COM, uuid for GUIDs, advapi32 for registry

target_link_libraries(K6
    PRIVATE
    ole32
    uuid
    oleaut32
    advapi32
    shlwapi
)

# Define output name
set_target_properties(K6 PROPERTIES
    OUTPUT_NAME "K6"
)

# Place all built artifacts (DLL, lib, etc.) into a single output folder inside the build tree
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Ensure per-config directories also go to the same place for multi-config generators
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${OUTPUTCONFIG}" OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR})
endforeach()

# Stage target: produces a minimal output folder containing only the DLL and required data files
add_custom_target(stage ALL
    COMMENT "Staging K6 DLL and data into ${OUTPUT_DIR}"
)

add_custom_command(TARGET stage POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
    # Copy the built DLL (or executable) into the output folder
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:K6> ${OUTPUT_DIR}/$<TARGET_FILE_NAME:K6>
    # Copy required data files (if present)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/data/strokeData.txt ${OUTPUT_DIR}/strokeData.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/data/suggestionsData.txt ${OUTPUT_DIR}/suggestionsData.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/scripts/README-Install.txt ${OUTPUT_DIR}/README-Install.txt
    VERBATIM
)