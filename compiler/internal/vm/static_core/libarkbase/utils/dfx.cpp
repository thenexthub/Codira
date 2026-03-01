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

#include "dfx.h"
#include "logger.h"
#include "string_helpers.h"

namespace ark {
DfxController *DfxController::dfxController_ = nullptr;
os::memory::Mutex DfxController::mutex_;  // NOLINT(fuchsia-statically-constructed-objects)

/* static */
void DfxController::SetDefaultOption()
{
    ASSERT(IsInitialized());
    for (auto option = DfxOptionHandler::DfxOption(0); option < DfxOptionHandler::END_FLAG;
         option = DfxOptionHandler::DfxOption(option + 1)) {
        switch (option) {
#ifdef PANDA_TARGET_UNIX
            case DfxOptionHandler::COMPILER_NULLCHECK:
                dfxController_->optionMap_[DfxOptionHandler::COMPILER_NULLCHECK] = 1;
                break;
            case DfxOptionHandler::REFERENCE_DUMP:
                dfxController_->optionMap_[DfxOptionHandler::REFERENCE_DUMP] = 1;
                break;
            case DfxOptionHandler::SIGNAL_CATCHER:
                dfxController_->optionMap_[DfxOptionHandler::SIGNAL_CATCHER] = 1;
                break;
            case DfxOptionHandler::SIGNAL_HANDLER:
                dfxController_->optionMap_[DfxOptionHandler::SIGNAL_HANDLER] = 1;
                break;
            case DfxOptionHandler::ARK_SIGQUIT:
                dfxController_->optionMap_[DfxOptionHandler::ARK_SIGQUIT] = 1;
                break;
            case DfxOptionHandler::ARK_SIGUSR1:
                dfxController_->optionMap_[DfxOptionHandler::ARK_SIGUSR1] = 1;
                break;
            case DfxOptionHandler::ARK_SIGUSR2:
                dfxController_->optionMap_[DfxOptionHandler::ARK_SIGUSR2] = 1;
                break;
            case DfxOptionHandler::MOBILE_LOG:
                dfxController_->optionMap_[DfxOptionHandler::MOBILE_LOG] = 1;
                break;
            case DfxOptionHandler::HUNG_UPDATE:
                dfxController_->optionMap_[DfxOptionHandler::HUNG_UPDATE] = 0;
                break;
#endif
            case DfxOptionHandler::DFXLOG:
                dfxController_->optionMap_[DfxOptionHandler::DFXLOG] = 0;
                break;
            default:
                break;
        }
    }
}

/* static */
void DfxController::ResetOptionValueFromString(const std::string &s)
{
    constexpr int BASE10 = 10;
    size_t lastPos = s.find_first_not_of(';', 0);
    size_t pos = s.find(';', lastPos);
    while (lastPos != std::string::npos) {
        std::string arg = s.substr(lastPos, pos - lastPos);
        lastPos = s.find_first_not_of(';', pos);
        pos = s.find(';', lastPos);
        std::string optionStr = arg.substr(0, arg.find(':'));
        std::string valueStr((arg.substr(arg.find(':') + 1)));
        auto value = static_cast<uint8_t>(strtoul(valueStr.data(), nullptr, BASE10));

        auto dfxOption = DfxOptionHandler::DfxOptionFromString(optionStr);
        if (dfxOption != DfxOptionHandler::END_FLAG) {
            DfxController::SetOptionValue(dfxOption, value);
#ifdef PANDA_TARGET_UNIX
            if (dfxOption == DfxOptionHandler::MOBILE_LOG) {
                Logger::SetMobileLogOpenFlag(value != 0);
            }
#endif
        } else {
            LOG(ERROR, DFX) << "Unknown Option " << optionStr;
        }
    }
}

/* static */
void DfxController::PrintDfxOptionValues()
{
    ASSERT(IsInitialized());
    for (auto &iter : dfxController_->optionMap_) {
        LOG(ERROR, DFX) << "DFX option: " << DfxOptionHandler::StringFromDfxOption(iter.first)
                        << ", option values: " << std::to_string(iter.second);
    }
}

/* static */
void DfxController::Initialize(std::map<DfxOptionHandler::DfxOption, uint8_t> optionMap)
{
    if (IsInitialized()) {
        dfxController_->SetDefaultOption();
        return;
    }
    {
        os::memory::LockHolder lock(mutex_);
        if (IsInitialized()) {
            dfxController_->SetDefaultOption();
            return;
        }
        dfxController_ = new DfxController(std::move(optionMap));
    }
}

std::map<DfxOptionHandler::DfxOption, uint8_t> SetOption()
{
    std::map<DfxOptionHandler::DfxOption, uint8_t> optionMap;
    for (auto option = DfxOptionHandler::DfxOption(0); option < DfxOptionHandler::END_FLAG;
         option = DfxOptionHandler::DfxOption(option + 1)) {
        switch (option) {
#ifdef PANDA_TARGET_UNIX
            case DfxOptionHandler::COMPILER_NULLCHECK:
                optionMap[DfxOptionHandler::COMPILER_NULLCHECK] = 1;
                break;
            case DfxOptionHandler::REFERENCE_DUMP:
                optionMap[DfxOptionHandler::REFERENCE_DUMP] = 1;
                break;
            case DfxOptionHandler::SIGNAL_CATCHER:
                optionMap[DfxOptionHandler::SIGNAL_CATCHER] = 1;
                break;
            case DfxOptionHandler::SIGNAL_HANDLER:
                optionMap[DfxOptionHandler::SIGNAL_HANDLER] = 1;
                break;
            case DfxOptionHandler::ARK_SIGQUIT:
                optionMap[DfxOptionHandler::ARK_SIGQUIT] = 1;
                break;
            case DfxOptionHandler::ARK_SIGUSR1:
                optionMap[DfxOptionHandler::ARK_SIGUSR1] = 1;
                break;
            case DfxOptionHandler::ARK_SIGUSR2:
                optionMap[DfxOptionHandler::ARK_SIGUSR2] = 1;
                break;
            case DfxOptionHandler::MOBILE_LOG:
                optionMap[DfxOptionHandler::MOBILE_LOG] = 1;
                break;
            case DfxOptionHandler::HUNG_UPDATE:
                optionMap[DfxOptionHandler::HUNG_UPDATE] = 0;
                break;
#endif
            case DfxOptionHandler::DFXLOG:
                optionMap[DfxOptionHandler::DFXLOG] = 0;
                break;
            default:
                break;
        }
    }
    return optionMap;
}

/* static */
void DfxController::Initialize()
{
    if (IsInitialized()) {
        dfxController_->SetDefaultOption();
        return;
    }
    {
        os::memory::LockHolder lock(mutex_);
        if (IsInitialized()) {
            dfxController_->SetDefaultOption();
            return;
        }

        dfxController_ = new DfxController(SetOption());
    }
}

/* static */
void DfxController::Destroy()
{
    if (!IsInitialized()) {
        return;
    }
    DfxController *d = nullptr;
    {
        os::memory::LockHolder lock(mutex_);
        if (!IsInitialized()) {
            return;
        }
        d = dfxController_;
        dfxController_ = nullptr;
    }
    delete d;
}

}  // namespace ark
