# Energy 🔋

Yay LLVM

## TODO

- [ ] Make a readme
- [ ] Fix CMake
    - Current antlr setup is stolen and not very well working.
    - Causes regenerations and rebuilds every time
    - wonky first time experience
- [ ] Implement the thing
    - From parsing source code to emitting equivalent LLVM IR of a simple subset of a language
- [ ] Grammar current doesn't make sense. You can make a top level function call for example. Statement needs to be revisited
- [ ] Think about nested function definitions/declarations? grammar allows, but will have to think about scoping
- [ ] Add features to the thing
    - Syntactic sugar
    - Types
    - User defined types
    - Standard library
