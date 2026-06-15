cmake_minimum_required(VERSION 3.16)

project(ProcessSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clangd / IntelliSense.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Shared simulation library (everything except the entry point), so the
# executable and the tests link against the same compiled code.
add_library(process_core STATIC
    src/Mesh/MeshCell.cpp
    src/Mesh/MeshGrid.cpp
    src/Process/Etcher.cpp
    src/Process/Depositor.cpp
    src/Process/Photolithography.cpp
    src/Process/ProcessSimulator.cpp
    src/IO/CSVExporter.cpp
    src/IO/VTKExporter.cpp
    src/IO/InputReader.cpp
    src/IO/PathResolver.cpp
)

target_include_directories(process_core PUBLIC include)

if (MSVC)
    target_compile_options(process_core PRIVATE /W4)
else()
    target_compile_options(process_core PRIVATE -Wall -Wextra -Wshadow)
endif()

add_executable(simulator src/main.cpp)
target_link_libraries(simulator PRIVATE process_core)

# Place the binary in output/ so the default relative input/output paths
# resolve the same way as the VS Code build task.
set_target_properties(simulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output"
)

enable_testing()

add_executable(process_tests tests/test_simulator.cpp)
target_link_libraries(process_tests PRIVATE process_core)

add_test(NAME process_tests COMMAND process_tests)
