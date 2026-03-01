/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file declares private functions of desugar macro.
 */
#ifndef CODIRA_SEMA_DESUGAR_MACRO_H
#define CODIRA_SEMA_DESUGAR_MACRO_H

#include "Codira/AST/Node.h"

namespace Codira {
using namespace AST;
/**
 * Create a wrapper func decl like this
 *
 * Attr macro wrapper func
 * """  func macroCall_a_ident (attrPtr: CPointer<UInt8>, attrSize: Int64,
 *          paramPtr: CPointer<UInt8>, paramSize: Int64): CPointer<UInt8> {
 *            try {
 *                let bufAttr: Array<UInt8> = Array<UInt8>(attrSize, repeat: 0)
 *                for (i in 0..attrSize) {
 *                    bufAttr[i] = unsafe{ attrPtr.read(i) }
 *                }
 *                let bufParam: Array<UInt8> = Array<UInt8>(paramSize, repeat: 0)
 *                for (i in 0..paramSize) {
 *                    bufParam[i] = unsafe{ paramPtr.read(i) }
 *                }
 *                let attr: Tokens = Tokens(bufAttr)
 *                let params: Tokens = Tokens(bufParam)
 *                let ret: Tokens = ident(attr, params)
 *                let tBuffer = ret.toBytes()
 *                return unsafePointerCastFromUint8Array(tBuffer)
 *            }
 *            catch (e: Exception) {
 *                e.printStackTrace()
 *                return unsafePointerCastFromUint8Array(@{})
 *            }
 *       }
 * """
 *
 * Common macro wrapper func
 * """  func macroCall_c_ident (paramPtr: CPointer<UInt8>, paramSize: Int64): CPointer<UInt8> {
 *            try {
 *                let bufParam: Array<UInt8> = Array<UInt8>(paramSize, repeat: 0)
 *                for (i in 0..paramSize) {
 *                    bufParam[i] = unsafe{ paramPtr.read(i) }
 *                }
 *                let params: Tokens = Tokens(bufParam)
 *                let ret: Tokens = ident(params)
 *                let tBuffer = ret.toBytes()
 *                return unsafePointerCastFromUint8Array(tBuffer)
 *            }
 *            catch (e: Exception) {
 *                e.printStackTrace()
 *                return unsafePointerCastFromUint8Array(@{})
 *            }
 *       }
 * """
 *
 * @param curFile : CurFile node to add wrapper node.
 * @param pos : Macro decl's end pos.
 * @param ident : Macro decl's ident.
 * @param isAttr : True if is a attr macro.
 */
void DesugarMacroDecl(File& file);

/**
 * Desugar quote expression.
 *
 * Before desugar: quote (1 + $(a) + 3)
 *
 * After desugar: Tokens([Token(1), Token(+)]) + a.ToTokens() + Tokens([Token(+), Token(3)])
 *
 * @param qe : QuoteExpr desugared.
 */
void DesugarQuoteExpr(QuoteExpr& qe);

void AddMacroContextInfo(FuncBody& funcBody);
} // namespace Codira
#endif
