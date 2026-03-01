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
    .p2align  4, 0x90
__CODE_Init_Start:                        # @__CODE_Init_Start
.seh_proc __CODE_Init_Start
# %bb.0:
    subq  $40, %rsp
    .seh_stackalloc 40
    .seh_endprologue
    leaq  __CODEMetadataStart(%rip), %rcx
    movq (%rcx), %rcx
    movl  $1, %edx
    callq MRT_LibraryOnLoad
    addq  $40, %rsp
    retq
    .seh_endproc
                                            # -- End function
    .section  .ctors,"dw"
    .p2align  3
    .quad __CODE_Init_Start

    .text
    .p2align  4, 0x90
__CODE_Fini_End:                        # @__CODE_Fini_End
.seh_proc __CODE_Fini_End
# %bb.0:
    subq  $40, %rsp
    .seh_stackalloc 40
    .seh_endprologue
    leaq  __CODEMetadataStart(%rip), %rcx
    movq (%rcx), %rcx
    callq MRT_LibraryUnLoad
    addq  $40, %rsp
    retq
    .seh_endproc
                                            # -- End function
    .section  .dtors,"dw"
    .p2align  3
    .quad __CODE_Fini_End

    .section  .header
    .p2align  4
    .long 0x12345678
    .long 0x80000000
    .quad 0
    .quad __CODEMetadataSize
    .quad __CODESDKVersion
    .quad __CODEMethodInfo
    .quad __CODEMethodInfoSize
    .quad __CODEGlobalInitFunc
    .quad __CODEGlobalInitFuncSize
    .quad __CODEStringPoolDict
    .quad __CODEStringPoolDictSize
    .quad __CODEStringPool
    .quad __CODEStringPoolSize
    .quad __CODEStackMap
    .quad __CODEStackMapSize
    .quad __CODEGCTib
    .quad __CODEGCTibSize
    .quad __CODEGCRoots
    .quad __CODEGCRootsSize
    .quad __CODETypeTemplate
    .quad __CODETypeTemplateSize
    .quad __CODETypeInfo
    .quad __CODETypeInfoSize
    .quad __CODETypeFields
    .quad __CODETypeFieldsSize
    .quad __CODEMTable
    .quad __CODEMTableSize
    .quad __CODEInnerTypeExtensions
    .quad __CODEInnerTypeExtensionsSize
    .quad __CODEOuterTypeExtensions
    .quad __CODEOuterTypeExtensionsSize
    .quad __CODEStaticGITable
    .quad __CODEStaticGITableSize
    .quad __CODEGCFlags
    .quad __CODEGCFlagsSize
    .quad __CODEReflectPkgInfo
    .quad __CODEGCReflectPkgInfoSize
    .quad __CODEReflectGV
    .quad __CODEReflectGVSize
    .quad __CODEReflectGI
    .quad __CODEReflectGISize
    .quad __CODETypeExt
    .quad __CODETypeExtSize
