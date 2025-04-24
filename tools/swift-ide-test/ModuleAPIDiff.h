#ifndef SWIFT_IDE_TEST_MODULE_API_DIFF_H
#define SWIFT_IDE_TEST_MODULE_API_DIFF_H

#include "language/Basic/LLVM.h"
#include <string>

namespace language {

int doGenerateModuleAPIDescription(StringRef DriverPath,
                                   StringRef MainExecutablePath,
                                   ArrayRef<std::string> Args);

} // end namespace language

#endif // SWIFT_IDE_TEST_MODULE_API_DIFF_H

