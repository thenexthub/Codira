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

#include "tooling/dynamic/base/pt_returns.h"

namespace panda::ecmascript::tooling {
std::unique_ptr<PtJson> DebuggerEnableReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("debuggerId", std::to_string(debuggerId_).c_str());
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = protocols_.size();
    for (size_t i = 0; i < len; i++) {
        array->Push(protocols_[i].c_str());
    }
    result->Add("protocols", array);

    return result;
}

std::unique_ptr<PtJson> EnableReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = protocols_.size();
    for (size_t i = 0; i < len; i++) {
        array->Push(protocols_[i].c_str());
    }
    result->Add("protocols", array);

    return result;
}

std::unique_ptr<PtJson> SetBreakpointByUrlReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("breakpointId", id_.c_str());
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = locations_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(locations_[i] != nullptr);
        std::unique_ptr<PtJson> location = locations_[i]->ToJson();
        array->Push(location);
    }
    result->Add("locations", array);

    return result;
}

std::unique_ptr<PtJson> GetPossibleAndSetBreakpointByUrlReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = locations_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(locations_[i] != nullptr);
        std::unique_ptr<PtJson> location = locations_[i]->ToJson();
        array->Push(location);
    }
    result->Add("locations", array);

    return result;
}

std::unique_ptr<PtJson> EvaluateOnCallFrameReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    ASSERT(result_ != nullptr);
    result->Add("result", result_->ToJson());
    if (exceptionDetails_) {
        ASSERT(exceptionDetails_.value() != nullptr);
        result->Add("exceptionDetails", exceptionDetails_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<PtJson> GetPossibleBreakpointsReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = locations_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(locations_[i] != nullptr);
        std::unique_ptr<PtJson> location = locations_[i]->ToJson();
        array->Push(location);
    }
    result->Add("locations", array);

    return result;
}

std::unique_ptr<PtJson> GetScriptSourceReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptSource", scriptSource_.c_str());
    if (bytecode_) {
        result->Add("bytecode", bytecode_->c_str());
    }

    return result;
}

std::unique_ptr<PtJson> RestartFrameReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = callFrames_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(callFrames_[i] != nullptr);
        std::unique_ptr<PtJson> location = callFrames_[i]->ToJson();
        array->Push(location);
    }
    result->Add("callFrames", array);

    return result;
}

std::unique_ptr<PtJson> SearchInContentReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = result_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(result_[i] != nullptr);
        std::unique_ptr<PtJson> res = result_[i]->ToJson();
        array->Push(res);
    }
    result->Add("result", array);

    return result;
}

std::unique_ptr<PtJson> SetBreakpointReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("breakpointId", breakpointId_.c_str());
    ASSERT(location_ != nullptr);
    result->Add("actualLocation", location_->ToJson());

    return result;
}

std::unique_ptr<PtJson> SetInstrumentationBreakpointReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("breakpointId", breakpointId_.c_str());

    return result;
}

std::unique_ptr<PtJson> SetScriptSourceReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    if (callFrames_) {
        std::unique_ptr<PtJson> array = PtJson::CreateArray();
        size_t len = callFrames_->size();
        for (size_t i = 0; i < len; i++) {
            ASSERT(callFrames_.value()[i] != nullptr);
            std::unique_ptr<PtJson> location = callFrames_.value()[i]->ToJson();
            array->Push(location);
        }
        result->Add("callFrames", array);
    }
    if (stackChanged_) {
        result->Add("stackChanged", stackChanged_.value());
    }
    if (exceptionDetails_) {
        ASSERT(exceptionDetails_.value() != nullptr);
        result->Add("exceptionDetails", exceptionDetails_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<PtJson> GetPropertiesReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = result_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(result_[i] != nullptr);
        std::unique_ptr<PtJson> location = result_[i]->ToJson();
        array->Push(location);
    }
    result->Add("result", array);
    if (internalPropertyDescripties_) {
        array = PtJson::CreateArray();
        len = internalPropertyDescripties_->size();
        for (size_t i = 0; i < len; i++) {
            ASSERT(internalPropertyDescripties_.value()[i] != nullptr);
            std::unique_ptr<PtJson> location = internalPropertyDescripties_.value()[i]->ToJson();
            array->Push(location);
        }
        result->Add("internalProperties", array);
    }
    if (privateProperties_) {
        array = PtJson::CreateArray();
        len = privateProperties_->size();
        for (size_t i = 0; i < len; i++) {
            ASSERT(privateProperties_.value()[i] != nullptr);
            std::unique_ptr<PtJson> location = privateProperties_.value()[i]->ToJson();
            array->Push(location);
        }
        result->Add("privateProperties", array);
    }
    if (exceptionDetails_) {
        ASSERT(exceptionDetails_.value() != nullptr);
        result->Add("exceptionDetails", exceptionDetails_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<PtJson> CallFunctionOnReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    ASSERT(result_ != nullptr);
    result->Add("result", result_->ToJson());
    if (exceptionDetails_) {
        ASSERT(exceptionDetails_.value() != nullptr);
        result->Add("exceptionDetails", exceptionDetails_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<PtJson> StopSamplingReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    ASSERT(profile_ != nullptr);
    result->Add("profile", profile_->ToJson());

    return result;
}

std::unique_ptr<PtJson> GetHeapObjectIdReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("heapSnapshotObjectId", std::to_string(heapSnapshotObjectId_).c_str());

    return result;
}

std::unique_ptr<PtJson> GetObjectByHeapObjectIdReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    ASSERT(remoteObjectResult_ != nullptr);
    result->Add("result", remoteObjectResult_->ToJson());

    return result;
}

std::unique_ptr<PtJson> StopReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    ASSERT(profile_ != nullptr);
    result->Add("profile", profile_->ToJson());

    return result;
}

std::unique_ptr<PtJson> GetHeapUsageReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("usedSize", usedSize_);
    result->Add("totalSize", totalSize_);

    return result;
}

std::unique_ptr<GetHeapUsageReturns> GetHeapUsageReturns::Create(const PtJson &params)
{
    auto heapUsageReturns = std::make_unique<GetHeapUsageReturns>();
    std::string error;
    Result ret;

    double usedSize;
    ret = params.GetDouble("usedSize", &usedSize);
    if (ret == Result::SUCCESS) {
        heapUsageReturns->usedSize_ = usedSize;
    } else {
        error += "Unknown 'usedSize';";
    }

    double totalSize;
    ret = params.GetDouble("totalSize", &totalSize);
    if (ret == Result::SUCCESS) {
        heapUsageReturns->totalSize_ = totalSize;
    } else {
        error += "Unknown 'totalSize';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "HeapUsageReturns::Create " << error;
        return nullptr;
    }

    return heapUsageReturns;
}

std::unique_ptr<PtJson> GetBestEffortCoverageReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = result_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(result_[i] != nullptr);
        std::unique_ptr<PtJson> scriptCoverage = result_[i]->ToJson();
        array->Push(scriptCoverage);
    }
    result->Add("result", array);

    return result;
}

std::unique_ptr<PtJson> StartPreciseCoverageReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("timestamp", timestamp_);

    return result;
}

std::unique_ptr<PtJson> TakePreciseCoverageReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = result_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(result_[i] != nullptr);
        std::unique_ptr<PtJson> scriptTypeProfile = result_[i]->ToJson();
        array->Push(scriptTypeProfile);
    }
    result->Add("result", array);
    result->Add("timestamp", timestamp_);

    return result;
}

std::unique_ptr<PtJson> TakeTypeProfileReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = result_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(result_[i] != nullptr);
        std::unique_ptr<PtJson> scriptTypeProfile = result_[i]->ToJson();
        array->Push(scriptTypeProfile);
    }
    result->Add("result", array);

    return result;
}

std::unique_ptr<PtJson> GetCategoriesReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> categories = PtJson::CreateArray();
    size_t len = categories_.size();
    for (size_t i = 0; i < len; i++) {
        categories->Push(categories_[i].c_str());
    }
    result->Add("categories", categories);

    return result;
}

std::unique_ptr<PtJson> RequestMemoryDumpReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("dumpGuid", dumpGuid_.c_str());
    result->Add("success", success_);

    return result;
}

std::unique_ptr<PtJson> GetNavigationHistoryReturns::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();

    result->Add("currentIndex", 0);
    array->Push(PtJson::CreateObject());
    result->Add("entries", array);

    return result;
}
}  // namespace panda::ecmascript::tooling