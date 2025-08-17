#ifndef LLVM_SUPPORT_REVERSEITERATION_H
#define LLVM_SUPPORT_REVERSEITERATION_H

#include <IndexStoreDB_LLVMSupport/toolchain_Config_abi-breaking.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_PointerLikeTypeTraits.h>

namespace toolchain {

template<class T = void *>
bool shouldReverseIterate() {
#if LLVM_ENABLE_REVERSE_ITERATION
  return detail::IsPointerLike<T>::value;
#else
  return false;
#endif
}

}
#endif
