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

#include <cassert>

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/ir_static.h"

#include "libabckit/src/macros.h"

#include <iostream>

#include "isa_dynamic_impl_instr.h"

namespace libabckit {

AbckitIsaApiDynamic g_isaApiDynamicImpl = {
    IgetModule,
    IsetModule,
    IgetDYNAMICConditionCode,
    IsetDYNAMICConditionCode,
    IgetDYNAMICOpcode,
    IgetImportDescriptor,
    IsetImportDescriptor,
    IgetExportDescriptor,
    IsetExportDescriptor,
    IcreateDYNAMICLoadString,
    IcreateDYNAMICLdnan,
    IcreateDYNAMICLdinfinity,
    IcreateDYNAMICLdundefined,
    IcreateDYNAMICLdnull,
    IcreateDYNAMICLdsymbol,
    IcreateDYNAMICLdglobal,
    IcreateDYNAMICLdtrue,
    IcreateDYNAMICLdfalse,
    IcreateDYNAMICLdhole,
    IcreateDYNAMICLdnewtarget,
    IcreateDYNAMICLdthis,
    IcreateDYNAMICPoplexenv,
    IcreateDYNAMICGetunmappedargs,
    IcreateDYNAMICAsyncfunctionenter,
    IcreateDYNAMICLdfunction,
    IcreateDYNAMICDebugger,
    IcreateDYNAMICGetpropiterator,
    IcreateDYNAMICGetiterator,
    IcreateDYNAMICGetasynciterator,
    IcreateDYNAMICLdprivateproperty,
    IcreateDYNAMICStprivateproperty,
    IcreateDYNAMICTestin,
    IcreateDYNAMICDefinefieldbyname,
    IcreateDYNAMICDefinepropertybyname,
    IcreateDYNAMICCreateemptyobject,
    IcreateDYNAMICCreateemptyarray,
    IcreateDYNAMICCreategeneratorobj,
    IcreateDYNAMICCreateiterresultobj,
    IcreateDYNAMICCreateobjectwithexcludedkeys,
    IcreateDYNAMICWideCreateobjectwithexcludedkeys,
    IcreateDYNAMICCreatearraywithbuffer,
    IcreateDYNAMICCreateobjectwithbuffer,
    IcreateDYNAMICNewobjapply,
    IcreateDYNAMICNewobjrange,
    IcreateDYNAMICWideNewobjrange,
    IcreateDYNAMICNewlexenv,
    IcreateDYNAMICWideNewlexenv,
    IcreateDYNAMICNewlexenvwithname,
    IcreateDYNAMICWideNewlexenvwithname,
    IcreateDYNAMICCreateasyncgeneratorobj,
    IcreateDYNAMICAsyncgeneratorresolve,
    IcreateDYNAMICAdd2,
    IcreateDYNAMICSub2,
    IcreateDYNAMICMul2,
    IcreateDYNAMICDiv2,
    IcreateDYNAMICMod2,
    IcreateDYNAMICEq,
    IcreateDYNAMICNoteq,
    IcreateDYNAMICLess,
    IcreateDYNAMICLesseq,
    IcreateDYNAMICGreater,
    IcreateDYNAMICGreatereq,
    IcreateDYNAMICShl2,
    IcreateDYNAMICShr2,
    IcreateDYNAMICAshr2,
    IcreateDYNAMICAnd2,
    IcreateDYNAMICOr2,
    IcreateDYNAMICXor2,
    IcreateDYNAMICExp,
    IcreateDYNAMICTypeof,
    IcreateDYNAMICTonumber,
    IcreateDYNAMICTonumeric,
    IcreateDYNAMICNeg,
    IcreateDYNAMICNot,
    IcreateDYNAMICInc,
    IcreateDYNAMICDec,
    IcreateDYNAMICIstrue,
    IcreateDYNAMICIsfalse,
    IcreateDYNAMICIsin,
    IcreateDYNAMICInstanceof,
    IcreateDYNAMICStrictnoteq,
    IcreateDYNAMICStricteq,
    IcreateDYNAMICCallruntimeNotifyconcurrentresult,
    IcreateDYNAMICCallruntimeDefinefieldbyvalue,
    IcreateDYNAMICCallruntimeDefinefieldbyindex,
    IcreateDYNAMICCallruntimeTopropertykey,
    IcreateDYNAMICCallruntimeCreateprivateproperty,
    IcreateDYNAMICCallruntimeDefineprivateproperty,
    IcreateDYNAMICCallruntimeCallinit,
    IcreateDYNAMICCallruntimeDefinesendableclass,
    IcreateDYNAMICCallruntimeLdsendableclass,
    IcreateDYNAMICCallruntimeLdsendableexternalmodulevar,
    IcreateDYNAMICCallruntimeWideldsendableexternalmodulevar,
    IcreateDYNAMICCallruntimeNewsendableenv,
    IcreateDYNAMICCallruntimeWidenewsendableenv,
    IcreateDYNAMICCallruntimeStsendablevar,
    IcreateDYNAMICCallruntimeWidestsendablevar,
    IcreateDYNAMICCallruntimeLdsendablevar,
    IcreateDYNAMICCallruntimeWideldsendablevar,
    IcreateDYNAMICCallruntimeIstrue,
    IcreateDYNAMICCallruntimeIsfalse,
    IcreateDYNAMICThrow,
    IcreateDYNAMICThrowNotexists,
    IcreateDYNAMICThrowPatternnoncoercible,
    IcreateDYNAMICThrowDeletesuperproperty,
    IcreateDYNAMICThrowConstassignment,
    IcreateDYNAMICThrowIfnotobject,
    IcreateDYNAMICThrowUndefinedifhole,
    IcreateDYNAMICThrowIfsupernotcorrectcall,
    IcreateDYNAMICThrowUndefinedifholewithname,
    IcreateDYNAMICCallarg0,
    IcreateDYNAMICCallarg1,
    IcreateDYNAMICCallargs2,
    IcreateDYNAMICCallargs3,
    IcreateDYNAMICCallrange,
    IcreateDYNAMICWideCallrange,
    IcreateDYNAMICSupercallspread,
    IcreateDYNAMICApply,
    IcreateDYNAMICCallthis0,
    IcreateDYNAMICCallthis1,
    IcreateDYNAMICCallthis2,
    IcreateDYNAMICCallthis3,
    IcreateDYNAMICCallthisrange,
    IcreateDYNAMICWideCallthisrange,
    IcreateDYNAMICSupercallthisrange,
    IcreateDYNAMICWideSupercallthisrange,
    IcreateDYNAMICSupercallarrowrange,
    IcreateDYNAMICWideSupercallarrowrange,
    IcreateDYNAMICDefinegettersetterbyvalue,
    IcreateDYNAMICDefinefunc,
    IcreateDYNAMICDefinemethod,
    IcreateDYNAMICDefineclasswithbuffer,
    IcreateDYNAMICResumegenerator,
    IcreateDYNAMICGetresumemode,
    IcreateDYNAMICGettemplateobject,
    IcreateDYNAMICGetnextpropname,
    IcreateDYNAMICDelobjprop,
    IcreateDYNAMICSuspendgenerator,
    IcreateDYNAMICAsyncfunctionawaituncaught,
    IcreateDYNAMICCopydataproperties,
    IcreateDYNAMICStarrayspread,
    IcreateDYNAMICSetobjectwithproto,
    IcreateDYNAMICLdobjbyvalue,
    IcreateDYNAMICStobjbyvalue,
    IcreateDYNAMICStownbyvalue,
    IcreateDYNAMICLdsuperbyvalue,
    IcreateDYNAMICStsuperbyvalue,
    IcreateDYNAMICLdobjbyindex,
    IcreateDYNAMICWideLdobjbyindex,
    IcreateDYNAMICStobjbyindex,
    IcreateDYNAMICWideStobjbyindex,
    IcreateDYNAMICStownbyindex,
    IcreateDYNAMICWideStownbyindex,
    IcreateDYNAMICAsyncfunctionresolve,
    IcreateDYNAMICAsyncfunctionreject,
    IcreateDYNAMICCopyrestargs,
    IcreateDYNAMICWideCopyrestargs,
    IcreateDYNAMICLdlexvar,
    IcreateDYNAMICWideLdlexvar,
    IcreateDYNAMICStlexvar,
    IcreateDYNAMICWideStlexvar,
    IcreateDYNAMICGetmodulenamespace,
    IcreateDYNAMICWideGetmodulenamespace,
    IcreateDYNAMICStmodulevar,
    IcreateDYNAMICWideStmodulevar,
    IcreateDYNAMICTryldglobalbyname,
    IcreateDYNAMICTrystglobalbyname,
    IcreateDYNAMICLdglobalvar,
    IcreateDYNAMICStglobalvar,
    IcreateDYNAMICLdobjbyname,
    IcreateDYNAMICStobjbyname,
    IcreateDYNAMICStownbyname,
    IcreateDYNAMICLdsuperbyname,
    IcreateDYNAMICStsuperbyname,
    IcreateDYNAMICLdlocalmodulevar,
    IcreateDYNAMICWideLdlocalmodulevar,
    IcreateDYNAMICLdexternalmodulevar,
    IcreateDYNAMICWideLdexternalmodulevar,
    IcreateDYNAMICStownbyvaluewithnameset,
    IcreateDYNAMICStownbynamewithnameset,
    IcreateDYNAMICLdbigint,
    IcreateDYNAMICLdthisbyname,
    IcreateDYNAMICStthisbyname,
    IcreateDYNAMICLdthisbyvalue,
    IcreateDYNAMICStthisbyvalue,
    IcreateDYNAMICWideLdpatchvar,
    IcreateDYNAMICWideStpatchvar,
    IcreateDYNAMICDynamicimport,
    IcreateDYNAMICAsyncgeneratorreject,
    IcreateDYNAMICSetgeneratorstate,
    IcreateDYNAMICReturn,
    IcreateDYNAMICReturnundefined,
    IcreateDYNAMICIf,
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitIsaApiDynamic const *AbckitGetIsaApiDynamicImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockIsaApiDynamicImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_isaApiDynamicImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
