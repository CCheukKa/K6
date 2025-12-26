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

# Copy dictionary file to output directory
add_custom_command(TARGET K6 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/data/strokeData.txt"
    "$<TARGET_FILE_DIR:K6>/strokeData.txt"
    COMMENT "Copying strokeData.txt to output directory"
)

# Copy suggestions file if present
add_custom_command(TARGET K6 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/data/suggestionsData.txt"
    "$<TARGET_FILE_DIR:K6>/suggestionsData.txt"
    COMMENT "Copying suggestionsData.txt to output directory"
    VERBATIM
)

# Copy punctuation file to output directory
add_custom_command(TARGET K6 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/data/punctuationData.txt"
    "$<TARGET_FILE_DIR:K6>/punctuationData.txt"
    COMMENT "Copying punctuationData.txt to output directory"
    VERBATIM
)

# Install step (optional)
install(TARGETS K6 DESTINATION bin)
install(FILES data/strokeData.txt DESTINATION bin)
install(FILES data/suggestionsData.txt DESTINATION bin OPTIONAL)
