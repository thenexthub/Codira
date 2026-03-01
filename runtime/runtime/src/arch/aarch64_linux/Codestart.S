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

    .text
    .p2align        2               // -- Begin function __CODE_Init_Start
    .type   __CODE_Init_Start,@function
__CODE_Init_Start:                        // @__CODE_Init_Start
    .cfi_startproc
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    paciasp
#endif
// %bb.0:
    str     x30, [sp, #-16]!        // 8-byte Folded Spill
    .cfi_def_cfa_offset 16
    .cfi_offset w30, -16
    adrp    x0, __CODEMetadataStart
    add     x0, x0, :lo12:__CODEMetadataStart
    mov     w1, #1
    bl      MRT_LibraryOnLoad
    ldr     x30, [sp], #16          // 8-byte Folded Reload
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    autiasp
#endif
    ret
.Lfunc_end12:
    .size   __CODE_Init_Start, .Lfunc_end12-__CODE_Init_Start
    .cfi_endproc

    .section        .init_array.0,"aw",@init_array
    .p2align        3
    .xword  __CODE_Init_Start

    .text
    .p2align        2               // -- Begin function __CODE_Fini_End
    .type   __CODE_Fini_End,@function
__CODE_Fini_End:                        // @__CODE_Fini_End
    .cfi_startproc
// %bb.0:
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    paciasp
#endif
    str     x30, [sp, #-16]!        // 8-byte Folded Spill
    .cfi_def_cfa_offset 16
    .cfi_offset w30, -16
    adrp    x0, __CODEMetadataStart
    add     x0, x0, :lo12:__CODEMetadataStart
    bl      MRT_LibraryUnLoad
    ldr     x30, [sp], #16          // 8-byte Folded Reload
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    autiasp
#endif
    ret
.Lfunc_end:
    .size   __CODE_Fini_End, .Lfunc_end-__CODE_Fini_End
    .cfi_endproc

    .section        .fini_array.0,"aw",@fini_array
    .p2align        3
    .xword  __CODE_Fini_End

    .section  .codemetadata.rw.header
    .p2align  4
    .long 0x12345678
    .long 0x80000000
    .long 0
    .long __CODEMetaDataSize
    .long __CODESDKVersion
    .long __CODEMethodInfo
    .long __CODEMethodInfoSize
    .long __CODEGlobalInitFunc
    .long __CODEGlobalInitFuncSize
    .long __CODEStringPoolDict
    .long __CODEStringPoolDictSize
    .long __CODEStringPool
    .long __CODEStringPoolSize
    .long __CODEStackMap
    .long __CODEStackMapSize
    .long __CODEGCTib
    .long __CODEGCTibSize
    .long __CODEGCRoots
    .long __CODEGCRootsSize
    .long __CODETypeTemplate
    .long __CODETypeTemplateSize
    .long __CODETypeInfo
    .long __CODETypeInfoSize
    .long __CODETypeFields
    .long __CODETypeFieldsSize
    .long __CODEMTable
    .long __CODEMTableSize
    .long __CODEInnerTypeExtensions
    .long __CODEInnerTypeExtensionsSize
    .long __CODEOuterTypeExtensions
    .long __CODEOuterTypeExtensionsSize
    .long __CODEStaticGITable
    .long __CODEStaticGITableSize
    .long __CODEGCFlags
    .long __CODEGCFlagsSize
    .long __CODEReflectPkgInfo
    .long __CODEGCReflectPkgInfoSize
    .long __CODEReflectGV
    .long __CODEReflectGVSize
    .long __CODEReflectGI
    .long __CODEReflectGISize
    .long __CODETypeExt
    .long __CODETypeExtSize
