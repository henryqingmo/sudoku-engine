cmake_minimum_required(VERSION 3.21)
project(sudoku-engine VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Automatically collect source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.h" "include/*.hpp")

# Include directories
include_directories(include)

# Detect Emscripten build
if(EMSCRIPTEN)
    # WebAssembly build
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
    
    # Emscripten-specific settings (build as ES6 module)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".js"
    )
    
    # Emscripten link flags
    target_link_options(${PROJECT_NAME} PRIVATE
        -sWASM=1
        -sALLOW_MEMORY_GROWTH=1
        -sNO_EXIT_RUNTIME=0
        -sNO_DISABLE_EXCEPTION_CATCHING
        -sINVOKE_RUN=0
        -sASSERTIONS=1
        -sEXPORT_ES6=1
        -sMODULARIZE=1
        -sEXPORT_NAME=createSudokuModule
        -sENVIRONMENT=web
        -fsanitize=address
        -fsanitize=undefined
        -g
    )
    
    # Export functions for web interface
    target_link_options(${PROJECT_NAME} PRIVATE
        -sEXPORTED_FUNCTIONS=_initGame,_runSolver,_stepSolver,_getBoardValue,_getBoardDomain,_main
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
    )
    
    # Copy WASM artifacts to Vue frontend
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_SOURCE_DIR}/web/public/wasm
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm
            ${CMAKE_SOURCE_DIR}/web/public/wasm/
        COMMENT "Copying WASM artifacts to web/public/wasm/"
    )
    
else()
    # Native build
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
    
    # Compiler warnings
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
    endif()
endif()

# Platform-agnostic optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(NOT MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
endif()
