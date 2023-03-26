## Testing Guide

1. These tests rely on the shared library (libvm.so). Before you run the tests
here, make sure your library code has compiled successfully.
2. To compile these tests, simply type `make` in this directory (pa5/tests).
Executables will be created with the same name as their corresponding .c files.
(e.g. `./alloc_basic`.)
3. Feel free to create your own test cases here. Just run the `make` command
again after creating your own .c file(s).
4. Whenever you make a change to your library code, you have to run `make` in
the parent directory to recompile your library (libvm.so).
5. You don't need to recompile these tests after modifying your library, only
after you modify the tests themselves.
