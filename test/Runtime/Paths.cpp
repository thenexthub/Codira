// RUN: %empty-directory(%t)

// RUN: mkdir -p %t/language-root/libexec/language %t/language-root/bin
// RUN: touch %t/language-root/libexec/language/Foo
// RUN: touch %t/language-root/libexec/language/Foo.exe
// RUN: touch %t/language-root/bin/Foo.exe

// RUN: %target-clang %s -std=c++11 -I %language_src_root/include -I %language_obj_root/include -I %language_src_root/stdlib/public/CodiraShims -I %clang-include-dir -isysroot %sdk -L%language-lib-dir/language/%target-sdk-name/%target-arch -L%toolchain_obj_root/lib/language/%target-sdk-name/%target-arch -llanguageCore -o %t/paths-test
// RUN: %target-codesign %t/paths-test
// RUN: %target-run %t/paths-test | %FileCheck %s
// RUN: env %env-LANGUAGE_ROOT=%t/language-root %target-run %t/paths-test | %FileCheck %s --check-prefix CHECK-FR
// REQUIRES: executable_test

// UNSUPPORTED: use_os_stdlib
// UNSUPPORTED: back_deployment_runtime

// This can't be done in unittests, because that statically links the runtime
// so we get the wrong paths.  We explicitly want to test that we get the
// path we expect (that is, the path to the runtime, and paths relative to
// that).

#include "language/Runtime/Paths.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(__CYGWIN)
#define stat _stat
#define S_IFDIR _S_IFDIR
#endif

static bool
exists(const char *path) {
  struct stat st;

  return stat(path, &st) == 0;
}

static bool
isfile(const char *path) {
  struct stat st;

  return stat(path, &st) == 0 && !(st.st_mode & S_IFDIR);
}

static bool
isdir(const char *path) {
  struct stat st;

  return stat(path, &st) == 0 && (st.st_mode & S_IFDIR);
}

static bool
containsLibCodira(const char *path) {
  const char *posix = "/lib/language/";
  const char *windows = "\\lib\\language\\";

  return strstr(path, posix) || strstr(path, windows);
}

int main(void) {
  const char *runtimePath = language_getRuntimeLibraryPath();

  // Runtime path must point to liblanguageCore and must be a file.

  // CHECK: runtime path: {{.*[\\/](lib)?}}languageCore.{{so|dylib|dll}}
  // CHECK-NEXT: runtime is a file: yes

  // CHECK-FR: runtime path: {{.*[\\/](lib)?}}languageCore.{{so|dylib|dll}}
  // CHECK-FR-NEXT: runtime is a file: yes
  printf("runtime path: %s\n", runtimePath ? runtimePath: "<NULL>");
  printf("runtime is a file: %s\n", isfile(runtimePath) ? "yes" : "no");

  const char *rootPath = language_getRootPath();

  // Root path must end in a separator and must be a directory

  // CHECK: root path: {{.*[\\/]$}}
  // CHECK-NEXT: root is a directory: yes

  // CHECK-FR: root path: {{.*[\\/]$}}
  // CHECK-FR-NEXT: root is a directory: yes
  printf("root path: %s\n", rootPath ? rootPath : "<NULL>");
  printf("root is a directory: %s\n", isdir(rootPath) ? "yes" : "no");

  // CHECK: root path contains /lib/language/: no
  // CHECK-FR: root path contains /lib/language/: no
  printf("root path contains /lib/language/: %s\n",
         containsLibCodira(rootPath) ? "yes" : "no");

  const char *auxPath = language_copyAuxiliaryExecutablePath("Foo");

  // CHECK: aux path: <NULL>
  // CHECK-FR: aux path: {{.*[\\/]libexec[\\/]language[\\/]Foo(\.exe)?}}

  printf("aux path: %s\n", auxPath ? auxPath : "<NULL>");

  free((void *)auxPath);

  return 0;
}
