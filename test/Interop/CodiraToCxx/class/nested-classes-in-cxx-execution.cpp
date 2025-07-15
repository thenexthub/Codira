// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/nested-classes-in-cxx.code -enable-library-evolution -typecheck -module-name Classes -clang-header-expose-decls=all-public -emit-clang-header-path %t/classes.h

// RUN: %target-interop-build-clangxx -std=c++17 -c %s -I %t -o %t/language-classes-execution.o

// RUN: %target-interop-build-language %S/nested-classes-in-cxx.code -enable-library-evolution -o %t/language-classes-execution -Xlinker %t/language-classes-execution.o -module-name Classes -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-classes-execution
// RUN: %target-run %t/language-classes-execution

// REQUIRES: executable_test

#include "classes.h"
#include <cassert>

int main() {
  using namespace Classes;
  auto x = makeRecordConfig();
  RecordConfig::File::Gate y = x.getGate();
  assert(y.getProp() == 80);
  assert(y.computeValue() == 160);
  RecordConfig::AudioFormat z = x.getFile().getFormat();
  assert(z == RecordConfig::AudioFormat::ALAC);
  RecordConfig::File::Gate g = RecordConfig::File::Gate::init();
}
