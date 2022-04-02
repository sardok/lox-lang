cmake_minimum_required(VERSION 3.10)

project(lox)

add_executable(
    ilox 
    interpreter/src/lox.cpp
    interpreter/src/main.cpp
    interpreter/src/scanner.cpp
    interpreter/src/parser.cpp
    interpreter/src/environment.cpp
    interpreter/src/token_type.cpp
    interpreter/src/lox_function.cpp
    interpreter/src/lox_class.cpp
    interpreter/src/lox_instance.cpp
    interpreter/src/object/operators.cpp
    interpreter/src/visitor/interpreter.cpp
    interpreter/src/visitor/resolver.cpp
    interpreter/src/globals/clock.cpp
)

add_executable(
    vlox
    vm/src/main.cpp
    vm/src/chunk.cpp
    vm/src/token.cpp
    vm/src/value.cpp
    vm/src/scanner.cpp
    vm/src/compiler.cpp
    vm/src/disassembler.cpp
    vm/src/vm.cpp
    vm/src/gc.cpp
    vm/src/object.cpp
    vm/src/table.cpp
    vm/src/native.cpp
)

target_include_directories(ilox PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(vlox PUBLIC "${PROJECT_BINARY_DIR}")

# target_compile_definitions(vlox PUBLIC DEBUG_TRACE_EXECUTION)
# target_compile_definitions(vlox PUBLIC DEBUG_PRINT_CODE)
# target_compile_definitions(vlox PUBLIC DEBUG_LOG_GC)

set(CMAKE_BUILD_TYPE Debug)

set_property(TARGET ilox PROPERTY CXX_STANDARD 20)
set_property(TARGET vlox PROPERTY CXX_STANDARD 20)
