/*===-- toolchain-c/Support.h - Support C Interface --------------------*- C -*-===*\
|*                                                                            *|
|* Part of the LLVM Project, under the Apache License v2.0 with LLVM          *|
|* Exceptions.                                                                *|
|* See https://toolchain.org/LICENSE.txt for license information.                  *|
|* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This file defines the C interface to the LLVM support library.             *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_C_SUPPORT_H
#define LLVM_C_SUPPORT_H

#include <IndexStoreDB_LLVMSupport/toolchain-c_DataTypes.h>
#include <IndexStoreDB_LLVMSupport/toolchain-c_Types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function parses the given arguments using the LLVM command line parser.
 * Note that the only stable thing about this function is its signature; you
 * cannot rely on any particular set of command line arguments being interpreted
 * the same way across LLVM versions.
 *
 * @see toolchain::cl::ParseCommandLineOptions()
 */
void LLVMParseCommandLineOptions(int argc, const char *const *argv,
                                 const char *Overview);

#ifdef __cplusplus
}
#endif

#endif
