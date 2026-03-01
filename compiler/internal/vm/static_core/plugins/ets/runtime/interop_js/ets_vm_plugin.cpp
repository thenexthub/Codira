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

#include <node_api.h>
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

#include "libarkbase/panda_gen_options/generated/logger_options.h"
#include "compiler_options.h"
#include "compiler/compiler_logger.h"
#include "interop_js/napi_impl/napi_impl.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "runtime/include/runtime.h"

#include "libarkbase/os/thread.h"

#include "plugins/ets/runtime/interop_js/interop_context_api.h"

namespace ark::ets::interop::js {

static napi_value Version(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    constexpr std::string_view MSG = "0.1";

    napi_value result;
    [[maybe_unused]] napi_status status = napi_create_string_utf8(env, MSG.data(), MSG.size(), &result);
    ASSERT(status == napi_ok);

    return result;
}

static napi_value GetEtsFunction(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 2U && jsArgc != 1U) {
        InteropCtx::ThrowJSError(env, "GetEtsFunction: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }

    std::array<napi_value, 2U> jsArgv {};
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv.data(), nullptr, nullptr));

    napi_value jsFunctionName = jsArgc == 1 ? jsArgv[0] : jsArgv[1];

    std::string packageName;
    if (jsArgc == 2U) {
        napi_value jsPackageName = jsArgv[0];
        if (GetValueType(env, jsPackageName) != napi_string) {
            InteropCtx::ThrowJSError(env, "GetEtsFunction: package name is not a string");
        }
        packageName = GetString(env, jsPackageName);
    }

    if (GetValueType(env, jsFunctionName) != napi_string) {
        InteropCtx::ThrowJSError(env, "GetEtsFunction: function name is not a string");
        return nullptr;
    }

    std::string methodName = GetString(env, jsFunctionName);

    return ets_proxy::GetETSFunction(env, packageName, methodName);
}

// This function returns napi_value, thus it is possible to get accessto it`s fields
// It gives opportunity to get acces to global variables from ETSGLOBAL
// test.ets
// export let x = 1
// test.js
// etsvm.getClass("ETSGLOBAL").x
static napi_value GetEtsClass(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        InteropCtx::ThrowJSError(env, "GetEtsClass: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }

    napi_value jsClassDescriptor {};
    ASSERT(jsArgc == 1);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, &jsClassDescriptor, nullptr, nullptr));

    std::string classDescriptor = GetString(env, jsClassDescriptor);
    return ets_proxy::GetETSClass(env, classDescriptor);
}

static napi_value GetEtsInstance(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        InteropCtx::ThrowJSError(env, "GetEtsInstance: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }
    napi_value jsClassDescriptor {};
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, &jsClassDescriptor, nullptr, nullptr));

    std::string classDescriptor = GetString(env, jsClassDescriptor);
    return ets_proxy::GetETSInstance(env, classDescriptor);
}

static napi_value GetEtsModule(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    std::array<napi_value, 1> argv {};
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &argc, argv.data(), nullptr, nullptr));
    if (argc != 1) {
        InteropCtx::ThrowJSError(env, "GetEtsModule: expects exactly one argument (module name)");
        return nullptr;
    }

    std::string moduleName = GetString(env, argv[0]);
    return ets_proxy::GetETSModule(env, moduleName);
}

static std::optional<std::vector<std::string>> GetArgStrings(napi_env env, napi_value options,
                                                             bool needFakeArgv0 = true)
{
    uint32_t numOptions;
    std::vector<std::string> argStrings;
    if (napi_get_array_length(env, options, &numOptions) == napi_ok) {
        // options passed as an array
        argStrings.reserve(numOptions + 1);
        argStrings.emplace_back("argv[0] placeholder");

        for (uint32_t i = 0; i < numOptions; ++i) {
            napi_value option;
            NAPI_ASSERT_OK(napi_get_element(env, options, i, &option));
            if (napi_coerce_to_string(env, option, &option) != napi_ok) {
                LogError("Option values must be coercible to string");
                return std::nullopt;
            }
            argStrings.push_back(GetString(env, option));
        }
    } else {
        // options passed as a map
        napi_value propNames;
        NAPI_ASSERT_OK(napi_get_property_names(env, options, &propNames));
        NAPI_ASSERT_OK(napi_get_array_length(env, propNames, &numOptions));

        argStrings.reserve(needFakeArgv0 ? numOptions + 1 : numOptions);
        if (needFakeArgv0) {
            argStrings.emplace_back("argv[0] placeholder");
        }

        for (uint32_t i = 0; i < numOptions; ++i) {
            napi_value key;
            napi_value value;
            NAPI_ASSERT_OK(napi_get_element(env, propNames, i, &key));
            NAPI_ASSERT_OK(napi_get_property(env, options, key, &value));
            if (napi_coerce_to_string(env, value, &value) != napi_ok) {
                LogError("Option values must be coercible to string");
                return std::nullopt;
            }
            argStrings.push_back("--" + GetString(env, key) + "=" + GetString(env, value));
        }
    }

    return argStrings;
}

static bool AddOptions(logger::Options *loggerOptions, ark::RuntimeOptions *runtimeOptions,
                       const std::vector<std::string> &argStrings)
{
    ark::PandArgParser paParser;
    loggerOptions->AddOptions(&paParser);
    runtimeOptions->AddOptions(&paParser);
    ark::compiler::g_options.AddOptions(&paParser);

    std::vector<const char *> fakeArgv;
    fakeArgv.reserve(argStrings.size());
    for (auto const &arg : argStrings) {
        fakeArgv.push_back(arg.c_str());  // Be careful, do not reallocate referenced strings
    }

    if (!paParser.Parse(fakeArgv.size(), fakeArgv.data())) {
        LogError("Parse options failed. Optional arguments:\n" + paParser.GetHelpString());
        return false;
    }

    auto runtimeOptionsErr = runtimeOptions->Validate();
    if (runtimeOptionsErr) {
        LogError("Parse options failed: " + runtimeOptionsErr.value().GetMessage());
        return false;
    }
    ark::compiler::CompilerLogger::SetComponents(ark::compiler::g_options.GetCompilerLog());
    return true;
}

[[maybe_unused]] static napi_value CreateRuntimeLegacy(napi_env env, napi_callback_info info)
{
    // NOTE(konstanting): this is needed for a sole purpose of introducing a dependency on libarktsbase.so.
    // In the standalone hybrid release build we have a REALLY REALLY strange behavior in the XGC tests:
    // libarktsbase's mem hooks REQUIRE the libarksbase.so to be loaded into the process address space
    // BEFORE libarkruntime.so. Otherwise dlsym would return NULL and everything will break. Since the
    // ets_vm_plugin module gets loaded first, we have to forcefully introduce a dependency on
    // libartktsbase.so for it, so it can be loaded before libarkruntime.
    // Sorry. I will try to find a more clean way to handle this ugly situation later on.
    volatile auto tid = ark::os::thread::GetCurrentThreadId();
    ++tid;
    // end of NOTE(konstanting)

    static size_t constexpr defaultArgc = 1;
    std::array<napi_value, defaultArgc> argv {};

    size_t argc = defaultArgc;
    NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, argv.data(), nullptr, nullptr));

    napi_value napiFalse;
    NAPI_ASSERT_OK(napi_get_boolean(env, false, &napiFalse));

    if (argc != defaultArgc) {
        LogError("CreateRuntimeLegacy: bad args number");
        return napiFalse;
    }

    napi_value options = argv[0];
    if (GetValueTypeNoScope(env, options) != napi_object) {
        LogError("CreateRuntimeLegacy: argument is not an object");
        return napiFalse;
    }

    auto argStrings = GetArgStrings(env, options);
    if (argStrings == std::nullopt) {
        return napiFalse;
    }

    auto addOpts = [&argStrings](logger::Options *loggerOptions, ark::RuntimeOptions *runtimeOptions) {
        return AddOptions(loggerOptions, runtimeOptions, argStrings.value());
    };

    bool res = ets::CreateRuntime(addOpts);
    if (res) {
        res = ark::ets::interop::js::CreateMainInteropContext(EtsCoroutine::GetCurrent(), env);
    }
    napi_value napiRes;
    NAPI_ASSERT_OK(napi_get_boolean(env, res, &napiRes));
    return napiRes;
}

static napi_value CreateRuntimeViaAni(napi_env env, napi_callback_info info)
{
    // NOTE(konstanting): we need to migrate various CreateRuntime features to this function in order
    // to fully migrate to ANI_CreateVM. The obvious examples are compiler-specific options and logging,
    // XGC-related checks and so on. See ets_vm_api.cpp:CreateRuntime and AddOptions function
    // above for reference.

    static size_t constexpr defaultArgc = 1;
    std::array<napi_value, defaultArgc> argv {};

    size_t argc = defaultArgc;
    NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, argv.data(), nullptr, nullptr));

    napi_value napiFalse;
    NAPI_ASSERT_OK(napi_get_boolean(env, false, &napiFalse));

    if (argc != defaultArgc) {
        LogError("CreateRuntimeViaAni: bad args number");
        return napiFalse;
    }

    napi_value options = argv[0];
    if (GetValueTypeNoScope(env, options) != napi_object) {
        LogError("CreateRuntimeViaAni: argument is not an object");
        return napiFalse;
    }

    auto argStrings = GetArgStrings(env, options, false);
    if (argStrings == std::nullopt) {
        LogError("CreateRuntimeViaAni: cannot parse options");
        return napiFalse;
    }

    // NOTE(konstanting, #23205): kind of messy, need to prettify
    const std::string optionPrefix = "--ext:";
    std::vector<std::string> extraVmAniStrings;
    for (auto &opt : *argStrings) {
        extraVmAniStrings.push_back(optionPrefix + opt);
    }
    std::vector<ani_option> optVector;
    for (auto &aniOptStr : extraVmAniStrings) {
        ani_option aniOpt = {aniOptStr.data(), nullptr};
        optVector.push_back(aniOpt);
    }
    optVector.push_back(ani_option {"--ext:interop", env});
    ani_options aniOptions = {optVector.size(), optVector.data()};

    ani_vm *panda;
    ani_status aniStatus = ANI_CreateVM(&aniOptions, ANI_VERSION_1, &panda);
    if (aniStatus != ANI_OK) {
        LogError("CreateRuntimeViaAni: ANI_CreateVM failed");
    }

    napi_value napiRes;
    NAPI_ASSERT_OK(napi_get_boolean(env, aniStatus == ANI_OK, &napiRes));
    return napiRes;
}

static napi_value Init(napi_env env, napi_value exports)
{
    auto STValueCtor = GetSTValueClass(env);
    auto STypeObj = CreateSTypeObject(env);
    const std::array desc = {
        napi_property_descriptor {"STValue", 0, 0, 0, 0, STValueCtor, napi_default, 0},
        napi_property_descriptor {"SType", 0, 0, 0, 0, STypeObj, napi_default, 0},
        napi_property_descriptor {"version", 0, Version, 0, 0, 0, napi_enumerable, 0},
        // NOTE(konstanting, #23205): to be deleted
        napi_property_descriptor {"createRuntimeLegacy", 0, CreateRuntimeLegacy, 0, 0, 0, napi_enumerable, 0},
        // NOTE(konstanting, #23205): to be renamed once migration is complete
        napi_property_descriptor {"createRuntime", 0, CreateRuntimeViaAni, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"getFunction", 0, GetEtsFunction, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"getClass", 0, GetEtsClass, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"getInstance", 0, GetEtsInstance, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"getModule", 0, GetEtsModule, 0, 0, 0, napi_enumerable, 0},
    };

    NAPI_CHECK_FATAL(napi_define_properties(env, exports, desc.size(), desc.data()));

    NapiImpl::InitNapi();

    return exports;
}

}  // namespace ark::ets::interop::js

NAPI_MODULE(ETS_INTEROP_JS_NAPI, ark::ets::interop::js::Init)
