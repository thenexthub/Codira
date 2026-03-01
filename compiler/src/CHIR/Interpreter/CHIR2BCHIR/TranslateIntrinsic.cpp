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
 * This file implements a translation from CHIR intrinsics to BCHIR intrinsics.
 */
#include "Codira/CHIR/Interpreter/BCHIR.h"
#include "Codira/CHIR/Interpreter/CHIR2BCHIR.h"
#include "Codira/CHIR/Interpreter/Utils.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Type/StructDef.h"

using namespace Codira::CHIR;
using namespace Interpreter;

template <typename T> void CHIR2BCHIR::TranslateIntrinsicExpression(Context& ctx, const T& intrinsic)
{
    /* One of the following depending on the necessary additional information
        bchir :: INTRINSIC0 :: INTRINSIC_KIND
    or
        bchir :: INTRINSIC0 :: INTRINSIC_KIND :: AUX_INFO1
    or
        bchir :: INTRINSIC2 :: INTRINSIC_KIND :: AUX_INFO2 :: AUX_INFO2

    Note that we insert an additional argument to store additional information. For some intrinsic
    functions we need to store some type information. Depending on the intrinsic operation this can
    represent different things. */

    if (intrinsic.GetIntrinsicKind() == CHIR::IntrinsicKind::CG_UNSAFE_BEGIN ||
        intrinsic.GetIntrinsicKind() == CHIR::IntrinsicKind::CG_UNSAFE_END) {
        return;
    }

    auto isCType = [](const CHIR::Type& ty) {
        // T0D0: is an array of arrays a C type? probably yes!
        return ty.IsPrimitive() || ty.IsCString() ||
            (ty.IsStruct() && StaticCast<const StructType*>(&ty)->GetStructDef()->IsCStruct());
    };

    std::vector<Bchir::ByteCodeContent> auxInfo{};
    switch (intrinsic.GetIntrinsicKind()) {
        case CHIR::IntrinsicKind::ARRAY_BUILT_IN_COPY_TO:
        case CHIR::IntrinsicKind::ARRAY_CLONE:
        case CHIR::IntrinsicKind::ARRAY_ACQUIRE_RAW_DATA: {
            auto refTy = StaticCast<RefType*>(intrinsic.GetOperands()[0]->GetType());
            auto arrayTy = StaticCast<RawArrayType*>(refTy->GetTypeArgs()[0]);
            auto valueTy = arrayTy->GetTypeArgs()[0];
            if (isCType(*valueTy)) {
                // T0D0: can we store the type of the array content instead?
                auxInfo.emplace_back(GetTypeIdx(*arrayTy));
            } else {
                auxInfo.emplace_back(UINT32_MAX);
            }
            break;
        }
        case CHIR::IntrinsicKind::ARRAY_GET:
        case CHIR::IntrinsicKind::ARRAY_GET_UNCHECKED:
        case CHIR::IntrinsicKind::ARRAY_SET:
        case CHIR::IntrinsicKind::ARRAY_SET_UNCHECKED: {
            auto refTy = StaticCast<RefType*>(intrinsic.GetOperands()[0]->GetType());
            auto arrayTy = StaticCast<RawArrayType*>(refTy->GetTypeArgs()[0]);
            auto valueTy = arrayTy->GetTypeArgs()[0];
            if (isCType(*valueTy)) {
                auxInfo.emplace_back(GetTypeIdx(*valueTy));
            } else {
                auxInfo.emplace_back(UINT32_MAX);
            }
            break;
        }
        case CHIR::IntrinsicKind::VARRAY_GET: {
            auto pathSize = static_cast<Bchir::ByteCodeContent>(intrinsic.GetNumOfOperands());
            PushOpCodeWithAnnotations<false, true>(ctx, OpCode::VARRAY_GET, intrinsic, pathSize - 1);
            return;
        }
        case CHIR::IntrinsicKind::BEGIN_CATCH:
            // Behaves like id function.
            return;
        case CHIR::IntrinsicKind::OBJECT_REFEQ: {
            break; // nothing to do here, just trying to be exhaustive
        }
        default: {
            // by default translate unlisted intrinsic as INTRINSIC0
            break;
        }
    }
    auto intrinsicKind = static_cast<Bchir::ByteCodeContent>(intrinsic.GetIntrinsicKind());
    CODEC_ASSERT(auxInfo.size() <= Bchir::FLAG_THREE);
    auto opCode =
        auxInfo.size() == 0 ? OpCode::INTRINSIC0 : (auxInfo.size() == 1 ? OpCode::INTRINSIC1 : OpCode::INTRINSIC2);
    if (intrinsic.GetExprKind() != ExprKind::INTRINSIC) {
        CODEC_ASSERT(intrinsic.GetExprKind() == ExprKind::INTRINSIC_WITH_EXCEPTION);
        opCode = static_cast<OpCode>(static_cast<size_t>(opCode) + Bchir::FLAG_THREE);
    }
    PushOpCodeWithAnnotations<false, true>(ctx, opCode, intrinsic, intrinsicKind);
    if (auxInfo.size() > 0) {
        ctx.def.Push(auxInfo[0]);
        if (auxInfo.size() > 1) {
            ctx.def.Push(auxInfo[1]);
        }
    }
}

// force instantiation of TranslateIntrinsicExpression with Intrinsic and IntrinsicWithException
template void CHIR2BCHIR::TranslateIntrinsicExpression(Context& ctx, const Intrinsic& intrinsic);
