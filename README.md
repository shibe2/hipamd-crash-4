This is a code to reproduce the issue ROCm-Developer-Tools/clr#4

Makefile works with GNU Make.

Assume that the accelerator present on the test system is not gfx801.

The crash only happens if multiple binaries are loaded.

    > ./app native1.so gfx801.so native2.so
    native1.so: Segmentation fault (core dumped)

Expected behavior:

    > ./app native1.so gfx801.so native2.so
    native1.so: ok
    gfx801.so: hipErrorInvalidDeviceFunction
    native2.so: ok

or similar message.

If only one binary is loaded, it does not crash:

    > ./app gfx801.so
    gfx801.so: hipErrorInvalidDeviceFunction

If all binaries have the code for the target architecture, there is no issue:

    > ./app native1.so native2.so native3.so
    native1.so: ok
    native2.so: ok
    native3.so: ok