/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_H_
#define PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_H_

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "verification/util/flags.h"
#include "verification/util/saturated_enum.h"
#include "verifier_messages.h"

#include <functional>

namespace ark::verifier {

struct MethodOption {
    enum class InfoType { CONTEXT, REG_CHANGES, CFLOW, JOBFILL };
    enum class MsgClass { ERROR, WARNING, HIDDEN };
    enum class CheckType { CFLOW, RESOLVE_ID, REG_USAGE, TYPING, ABSINT };
    using InfoTypeFlag =
        FlagsForEnum<unsigned, InfoType, InfoType::CONTEXT, InfoType::REG_CHANGES, InfoType::CFLOW, InfoType::JOBFILL>;
    using MsgClassFlag = FlagsForEnum<unsigned, MsgClass, MsgClass::ERROR, MsgClass::WARNING, MsgClass::HIDDEN>;
    using CheckEnum = SaturatedEnum<CheckType, CheckType::ABSINT, CheckType::TYPING, CheckType::REG_USAGE,
                                    CheckType::RESOLVE_ID, CheckType::CFLOW>;
};

class MethodOptions {
public:
    bool ShowContext() const
    {
        return showInfo_[MethodOption::InfoType::CONTEXT];
    }

    bool ShowRegChanges() const
    {
        return showInfo_[MethodOption::InfoType::REG_CHANGES];
    }

    bool ShowCflow() const
    {
        return showInfo_[MethodOption::InfoType::CFLOW];
    }

    bool ShowJobFill() const
    {
        return showInfo_[MethodOption::InfoType::JOBFILL];
    }

    void SetShow(MethodOption::InfoType info)
    {
        showInfo_[info] = true;
    }

    void SetMsgClass(VerifierMessage msgNum, MethodOption::MsgClass klass)
    {
        msgClasses_[msgNum] = klass;
    }

    template <typename Validator>
    void SetMsgClass(Validator validator, size_t msgNum, MethodOption::MsgClass klass)
    {
        if (validator(static_cast<VerifierMessage>(msgNum))) {
            msgClasses_[static_cast<VerifierMessage>(msgNum)] = klass;
        }
    }

    void AddUpLevel(const MethodOptions &up)
    {
        uplevel_.push_back(std::cref(up));
    }

    bool CanHandleMsg(VerifierMessage msgNum) const
    {
        return msgClasses_.count(msgNum) > 0;
    }

    MethodOption::MsgClass MsgClassFor(VerifierMessage msgNum) const;

    bool IsInMsgClass(VerifierMessage msgNum, MethodOption::MsgClass klass) const
    {
        return MsgClassFor(msgNum) == klass;
    }

    bool IsHidden(VerifierMessage msgNum) const
    {
        return IsInMsgClass(msgNum, MethodOption::MsgClass::HIDDEN);
    }

    bool IsWarning(VerifierMessage msgNum) const
    {
        return IsInMsgClass(msgNum, MethodOption::MsgClass::WARNING);
    }

    bool IsError(VerifierMessage msgNum) const
    {
        return IsInMsgClass(msgNum, MethodOption::MsgClass::ERROR);
    }

    template <typename Flag>
    static bool EnumerateFlagsHandler(PandaString &result, Flag flag)
    {
        switch (flag) {
            case MethodOption::InfoType::CONTEXT:
                result += "'context' ";
                break;
            case MethodOption::InfoType::REG_CHANGES:
                result += "'reg-changes' ";
                break;
            case MethodOption::InfoType::CFLOW:
                result += "'cflow' ";
                break;
            case MethodOption::InfoType::JOBFILL:
                result += "'jobfill' ";
                break;
            default:
                result += "<unknown>(";
                result += std::to_string(static_cast<size_t>(flag));
                result += ") ";
                break;
        }
        return true;
    }

    PandaString Image() const
    {
        PandaString result {"\n"};
        result += " Verifier messages config '" + name_ + "'\n";
        result += "  Uplevel configs: ";
        for (const auto &up : uplevel_) {
            result += "'" + up.get().name_ + "' ";
        }
        result += "\n";
        result += "  Show: ";
        showInfo_.EnumerateFlags([&](auto flag) { return MethodOptions::EnumerateFlagsHandler(result, flag); });
        result += "\n";
        result += "  Checks: ";
        enabledCheck_.EnumerateValues([&](auto flag) {
            switch (flag) {
                case MethodOption::CheckType::TYPING:
                    result += "'typing' ";
                    break;
                case MethodOption::CheckType::ABSINT:
                    result += "'absint' ";
                    break;
                case MethodOption::CheckType::REG_USAGE:
                    result += "'reg-usage' ";
                    break;
                case MethodOption::CheckType::CFLOW:
                    result += "'cflow' ";
                    break;
                case MethodOption::CheckType::RESOLVE_ID:
                    result += "'resolve-id' ";
                    break;
                default:
                    result += "<unknown>(";
                    result += std::to_string(static_cast<size_t>(flag));
                    result += ") ";
                    break;
            }
            return true;
        });

        result += "\n";
        result += ImageMessages();
        return result;
    }

    explicit MethodOptions(PandaString paramName) : name_ {std::move(paramName)} {}

    const PandaString &GetName() const
    {
        return name_;
    }

    MethodOption::CheckEnum &Check()
    {
        return enabledCheck_;
    }

    const MethodOption::CheckEnum &Check() const
    {
        return enabledCheck_;
    }

private:
    PandaString ImageMessages() const
    {
        PandaString result;
        result += "  Messages:\n";
        for (const auto &m : msgClasses_) {
            const auto &msgNum = m.first;
            const auto &klass = m.second;
            result += "    ";
            result += VerifierMessageToString(msgNum);
            result += " : ";
            switch (klass) {
                case MethodOption::MsgClass::ERROR:
                    result += "E";
                    break;
                case MethodOption::MsgClass::WARNING:
                    result += "W";
                    break;
                case MethodOption::MsgClass::HIDDEN:
                    result += "H";
                    break;
                default:
                    result += "<unknown>(";
                    result += std::to_string(static_cast<size_t>(klass));
                    result += ")";
                    break;
            }
            result += "\n";
        }
        return result;
    }

    const PandaString name_;
    PandaVector<std::reference_wrapper<const MethodOptions>> uplevel_;
    PandaUnorderedMap<VerifierMessage, MethodOption::MsgClass> msgClasses_;
    MethodOption::InfoTypeFlag showInfo_;
    MethodOption::CheckEnum enabledCheck_;

    // In verifier_messages_data.cpp
    struct VerifierMessageDefault {
        VerifierMessage msg;
        MethodOption::MsgClass msgClass;
    };

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static VerifierMessageDefault messageDefaults_[];
};

}  // namespace ark::verifier

#endif  // PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_H_
