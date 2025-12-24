cmake_minimum_required(VERSION 3.20)
project(ChineseIMEHKExample LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_definitions(-DUNICODE -D_UNICODE)

# Build a COM in-proc server (DLL)
add_library(ChineseIMEHKExample SHARED
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
    src/guid.h
    src/Registration.cpp
    src/Registration.h
    resources/resource.rc
)

# Link with required Windows libs
# ole32 for COM, uuid for GUIDs, advapi32 for registry

target_link_libraries(ChineseIMEHKExample
    PRIVATE
    ole32
    uuid
    oleaut32
    advapi32
    shlwapi
)

# Define output name
set_target_properties(ChineseIMEHKExample PROPERTIES
    OUTPUT_NAME "ChineseIMEHKExample"
)

# Install step (optional)
install(TARGETS ChineseIMEHKExample DESTINATION bin)
