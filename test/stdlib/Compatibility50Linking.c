// RUN: %empty-directory(%t)
// RUN: %target-clang %s -all_load %test-resource-dir/%target-sdk-name/liblanguageCompatibility50.a %test-resource-dir/%target-sdk-name/liblanguageCompatibility51.a -lobjc -o %t/main
// RUN: %target-codesign %t/main
// RUN: %target-run %t/main
// REQUIRES: objc_interop
// REQUIRES: executable_test

// The compatibility library needs to have no build-time dependencies on
// liblanguageCore so it can be linked into a program that doesn't link
// liblanguageCore, but will load it at runtime, such as xctest.
//
// Test this by linking it into a plain C program and making sure it builds.

int main(void) {}
