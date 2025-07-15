#ifndef LANGUAGE_IDE_TEST_MODULE_API_DIFF_H
#define LANGUAGE_IDE_TEST_MODULE_API_DIFF_H

#include "language/Basic/Toolchain.h"
#include <string>

namespace language {

int doGenerateModuleAPIDescription(StringRef DriverPath,
                                   StringRef MainExecutablePath,
                                   ArrayRef<std::string> Args);

} // end namespace language

#endif // LANGUAGE_IDE_TEST_MODULE_API_DIFF_H

