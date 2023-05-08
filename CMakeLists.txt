# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(Energy)

LIST( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

find_package(LLVM 15 REQUIRED CONFIG)
message("LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

# compiler must be 11 or 14?
SET (CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set variable pointing to the antlr tool that supports C++
set(ANTLR4CPP_JAR_LOCATION ${PROJECT_SOURCE_DIR}/thirdparty/antlr/antlr-4.7-complete.jar)
# add external build for antlrcpp
include( ExternalAntlr4Cpp )
# add antrl4cpp artifacts to project environment
include_directories( ${ANTLR4CPP_INCLUDE_DIRS} )
link_directories( ${ANTLR4CPP_LIBS} )
message(STATUS "Found antlr4cpp libs: ${ANTLR4CPP_LIBS} and includes: ${ANTLR4CPP_INCLUDE_DIRS} ")

# Call macro to add lexer and grammar to your build dependencies.
antlr4cpp_process_grammar(demo antlrparser
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/TestLexer.g4
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/TestParser.g4)
# include generated files in project environment
include_directories(${antlr4cpp_include_dirs_antlrparser})

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_link_options("-lLLVM-15")

# add generated grammar to demo binary target
add_executable(energyc main.cpp src/IR/astvisitor.cpp ${antlr4cpp_src_files_antlrparser})
add_dependencies(energyc antlr4cpp antlr4cpp_generation_antlrparser)

target_link_libraries(energyc antlr4-runtime)