
#include "vm/core/Testing/Support/SupportHelpers.h"

#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Path.h"

#include "gtest/gtest.h"

using namespace vm::core;
using namespace vm::core::unittest;

static std::pair<bool, SmallString<128>> findSrcDirMap(StringRef Argv0) {
  SmallString<128> BaseDir = toolchain::sys::path::parent_path(Argv0);

  toolchain::sys::fs::make_absolute(BaseDir);

  SmallString<128> PathInSameDir = BaseDir;
  toolchain::sys::path::append(PathInSameDir, "toolchain.srcdir.txt");

  if (toolchain::sys::fs::is_regular_file(PathInSameDir))
    return std::make_pair(true, std::move(PathInSameDir));

  SmallString<128> PathInParentDir = toolchain::sys::path::parent_path(BaseDir);

  toolchain::sys::path::append(PathInParentDir, "toolchain.srcdir.txt");
  if (toolchain::sys::fs::is_regular_file(PathInParentDir))
    return std::make_pair(true, std::move(PathInParentDir));

  return std::pair<bool, SmallString<128>>(false, {});
}

SmallString<128> toolchain::unittest::getInputFileDirectory(const char *Argv0) {
  bool Found = false;
  SmallString<128> InputFilePath;
  std::tie(Found, InputFilePath) = findSrcDirMap(Argv0);

  EXPECT_TRUE(Found) << "Unit test source directory file does not exist.";

  auto File = MemoryBuffer::getFile(InputFilePath, /*IsText=*/true);

  EXPECT_TRUE(static_cast<bool>(File))
      << "Could not open unit test source directory file.";

  InputFilePath.clear();
  InputFilePath.append((*File)->getBuffer().trim());
  toolchain::sys::path::append(InputFilePath, "Inputs");
  toolchain::sys::path::native(InputFilePath);
  return InputFilePath;
}
