// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/nested-structs-in-cxx.code -enable-library-evolution -module-name Structs -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/structs.h

// RUN: %target-interop-build-clangxx -std=c++17 -c %s -I %t -o %t/language-structs-execution.o

// RUN: %target-interop-build-language %S/nested-structs-in-cxx.code -enable-library-evolution -o %t/language-structs-execution -Xlinker %t/language-structs-execution.o -module-name Structs -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-structs-execution
// RUN: %target-run %t/language-structs-execution

// REQUIRES: executable_test

#include "structs.h"

int main() {
  using namespace Structs;
  auto x = makeRecordConfig();
  RecordConfig::File::Gate y = x.getGate();
  RecordConfig::AudioFormat z = x.getFile().getFormat();

  auto xx = makeAudioFileType();
  AudioFileType::SubType yy = xx.getCAF();
}
