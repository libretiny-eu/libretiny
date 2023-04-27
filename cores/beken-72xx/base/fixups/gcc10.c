/* Copyright (c) Kuba Szczodrzyński 2023-03-13. */

// Compiling with GCC 10.3.1 requires this or it will produce linker errors.
// See:
// - https://github.com/purduesigbots/pros/issues/153#issuecomment-519335375
// - https://stackoverflow.com/a/64664385
void __sync_synchronize(void) {}
