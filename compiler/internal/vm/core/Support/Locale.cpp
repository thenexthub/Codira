#include "vm/core/Support/Locale.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/Unicode.h"

namespace vm::core {
namespace sys {
namespace locale {

int columnWidth(StringRef Text) {
  return toolchain::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(int UCS) {
  return toolchain::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace vm::core
