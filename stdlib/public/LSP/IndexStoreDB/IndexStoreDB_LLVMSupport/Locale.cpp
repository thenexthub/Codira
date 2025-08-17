#include <IndexStoreDB_LLVMSupport/toolchain_Support_Locale.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Unicode.h>

namespace toolchain {
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
} // namespace toolchain
