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

#ifndef LIBPANDABASE_UTILS_PANDARGS_H_
#define LIBPANDABASE_UTILS_PANDARGS_H_

#include <algorithm>
#include <array>
#include <list>
#include <set>
#include <map>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
#include <vector>
#include <cerrno>
#include <optional>
#include <utility>

#include "libarkbase/macros.h"
#include "string_helpers.h"

namespace ark {
class PandArgBase;
// NOLINTNEXTLINE(readability-identifier-naming)
using sub_args_t = std::vector<PandArgBase *>;
// NOLINTNEXTLINE(readability-identifier-naming)
using arg_list_t = std::vector<std::string>;
using std::enable_if_t;
using std::is_same_v;

enum class PandArgType : uint8_t { STRING, INTEGER, DOUBLE, BOOL, LIST, UINT32, UINT64, COMPOUND, NOTYPE };

// Base class for panda argument
class PandArgBase {
public:
    explicit PandArgBase(std::string name, std::string desc, PandArgType type = PandArgType::NOTYPE)
        : name_(std::move(name)), desc_(std::move(desc)), type_(type)
    {
    }

    PandArgType GetType() const
    {
        return type_;
    }

    std::string GetName() const
    {
        return name_;
    }

    std::string GetDesc() const
    {
        return desc_;
    }

    bool WasSet() const
    {
        return wasSet_;
    }

    void SetWasSet(bool value)
    {
        wasSet_ = value;
    }

    virtual void ResetDefaultValue() = 0;

private:
    std::string name_;
    std::string desc_;
    PandArgType type_;
    bool wasSet_ {false};
};

template <typename T>
struct ArgView {
    using Type = T;
};

template <>
struct ArgView<std::string> {
    // Use 'const &' instead of `string_view` because of `c_str()` use-cases.
    using Type = const std::string &;
};

template <>
struct ArgView<arg_list_t> {
    // Use 'const &' instead of `string_view` because of `c_str()` use-cases.
    using Type = const arg_list_t &;
};

template <typename T,
          enable_if_t<is_same_v<std::string, T> || is_same_v<double, T> || is_same_v<bool, T> || is_same_v<int, T> ||
                      // CC-OFFNXT(G.FMT.10) project code style
                      is_same_v<uint32_t, T> || is_same_v<uint64_t, T> || is_same_v<arg_list_t, T>> * = nullptr>
class PandArg : public PandArgBase {
public:
    using ViewT = typename ArgView<T>::Type;

    explicit PandArg(const std::string &name, T defaultVal, const std::string &desc)
        : PandArgBase(name, desc, this->EvalType()), defaultVal_(defaultVal), realVal_(defaultVal)
    {
    }

    explicit PandArg(const std::string &name, T defaultVal, const std::string &desc, PandArgType type)
        : PandArgBase(name, desc, type), defaultVal_(defaultVal), realVal_(defaultVal)
    {
    }

    explicit PandArg(const std::string &name, int defaultVal, const std::string &desc, T minVal, T maxVal)
        : PandArgBase(name, desc, this->EvalType()),
          defaultVal_(defaultVal),
          realVal_(defaultVal),
          minMaxVal_(std::pair<T, T>(minVal, maxVal))
    {
    }

    explicit PandArg(const std::string &name, const arg_list_t &defaultVal, const std::string &desc,
                     std::string delimiter)
        : PandArgBase(name, desc, PandArgType::LIST),
          defaultVal_ {defaultVal},
          realVal_ {defaultVal},
          delimiter_ {std::move(delimiter)}
    {
    }

    ViewT GetValue() const
    {
        return realVal_;
    }

    ViewT GetDefaultValue() const
    {
        return defaultVal_;
    }

    template <bool UPDATE_FLAG = true>
    void SetValue(T val)
    {
        realVal_ = val;
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon)
        if constexpr (UPDATE_FLAG) {  // NOLINT(readability-braces-around-statements)
            SetWasSet(true);
        }
    }

    void ResetDefaultValue() override
    {
        realVal_ = defaultVal_;
    }

    std::optional<std::string> GetDelimiter() const
    {
        return delimiter_;
    }
    std::optional<std::pair<T, T>> GetMinMaxVal()
    {
        return minMaxVal_;
    }

private:
    constexpr PandArgType EvalType()
    {
        // NOLINTNEXTLINE(bugprone-branch-clone)
        if constexpr (is_same_v<std::string, T>) {  // NOLINT(readability-braces-around-statements)
            return PandArgType::STRING;
            // NOLINTNEXTLINE(readability-braces-around-statements,readability-misleading-indentation)
        } else if constexpr (is_same_v<double, T>) {
            return PandArgType::DOUBLE;
            // NOLINTNEXTLINE(readability-braces-around-statements,readability-misleading-indentation)
        } else if constexpr (is_same_v<bool, T>) {
            return PandArgType::BOOL;
            // NOLINTNEXTLINE(readability-braces-around-statements,readability-misleading-indentation)
        } else if constexpr (is_same_v<int, T>) {
            return PandArgType::INTEGER;
            // NOLINTNEXTLINE(readability-braces-around-statements,readability-misleading-indentation)
        } else if constexpr (is_same_v<uint32_t, T>) {
            return PandArgType::UINT32;
            // NOLINTNEXTLINE(readability-braces-around-statements,readability-misleading-indentation)
        } else if constexpr (is_same_v<uint64_t, T>) {
            return PandArgType::UINT64;
            // NOLINTNEXTLINE(readability-braces-around-statements,readability-misleading-indentation)
        } else if constexpr (is_same_v<arg_list_t, T>) {
            return PandArgType::LIST;
        }
        UNREACHABLE();
    }

    T defaultVal_;
    T realVal_;

    // Only for integer arguments with range
    std::optional<std::pair<T, T>> minMaxVal_;

    // Only for strings with delimiter
    std::optional<std::string> delimiter_;
};

class PandArgCompound : public PandArg<bool> {
public:
    PandArgCompound(const std::string &name, const std::string &desc, std::initializer_list<PandArgBase *> subArgs)
        : PandArg<bool>(name, false, desc, PandArgType::COMPOUND), subArgs_ {subArgs}
    {
    }

    PandArgBase *FindSubArg(std::string_view name)
    {
        auto res =
            std::find_if(subArgs_.begin(), subArgs_.end(), [name](const auto &arg) { return arg->GetName() == name; });
        return res == subArgs_.end() ? nullptr : *res;
    }

    void ResetDefaultValue() override
    {
        PandArg<bool>::ResetDefaultValue();
        std::for_each(subArgs_.begin(), subArgs_.end(), [](auto &arg) { arg->ResetDefaultValue(); });
    }

    const auto &GetSubArgs() const
    {
        return subArgs_;
    }

private:
    sub_args_t subArgs_;
};

class PandArgParser {
public:
    bool Add(PandArgBase *arg)
    {
        if (arg == nullptr) {
            errstr_ += "pandargs: Can't add `nullptr` as an argument\n";
            return false;
        }
        bool success = args_.insert(arg).second;
        if (!success) {
            errstr_ += "pandargs: Argument " + arg->GetName() + " has duplicate\n";
        }
        return success;
    }

    bool PushBackTail(PandArgBase *arg)
    {
        if (arg == nullptr) {
            errstr_ += "pandargs: Can't add `nullptr` as a tail argument\n";
            return false;
        }
        if (std::find(tailArgs_.begin(), tailArgs_.end(), arg) != tailArgs_.end()) {
            errstr_ += "pandargs: Tail argument " + arg->GetName() + " is already in tail arguments list\n";
            return false;
        }
        tailArgs_.emplace_back(arg);
        return true;
    }

    bool PopBackTail()
    {
        if (tailArgs_.empty()) {
            errstr_ += "pandargs: Nothing to pop back from tail arguments\n";
            return false;
        }
        tailArgs_.pop_back();
        return true;
    }

    void EraseTail()
    {
        tailArgs_.erase(tailArgs_.begin(), tailArgs_.end());
    }

    bool Parse(const std::vector<std::string> &argvVec)
    {
        InitDefault();
        std::copy(argvVec.begin(), argvVec.end(), std::back_inserter(argvVec_));
        return ParseArgs();
    }

    bool Parse(int argc, const char *const argv[])  // NOLINT(modernize-avoid-c-arrays, hicpp-avoid-c-arrays)
    {
        InitDefault();
        for (int i = 1; i < argc; i++) {
            argvVec_.emplace_back(argv[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        return ParseArgs();
    }

    /// Parses a string to the option's value.
    bool ParseSingleArg(PandArgBase *option, const std::string_view &optionValue)
    {
        ASSERT(option != nullptr);
        argvVec_ = {std::string(optionValue)};
        argvIndex_ = 0;
        argvIndex_ += ParseNextParam(option, argvVec_[argvIndex_]);
        return errstr_.empty();
    }

    /// Parses option's name and returns corresponding pointer.
    PandArgBase *GetPandArg(const std::string_view &argName)
    {
        auto argIt = args_.find(argName);
        return (argIt != args_.end()) ? *argIt : nullptr;
    }

    std::string GetErrorString() const
    {
        return errstr_;
    }

    void EnableTail()
    {
        tailFlag_ = true;
    }

    void DisableTail()
    {
        tailFlag_ = false;
    }

    bool IsTailEnabled() const
    {
        return tailFlag_;
    }

    std::size_t GetTailSize() const
    {
        return tailArgs_.size();
    }

    void EnableRemainder() noexcept
    {
        remainderFlag_ = true;
    }

    void DisableRemainder() noexcept
    {
        remainderFlag_ = false;
    }

    bool IsRemainderEnabled() const
    {
        return remainderFlag_;
    }

    arg_list_t GetRemainder()
    {
        return remainder_;
    }

    bool IsArgSet(PandArgBase *arg) const
    {
        return args_.find(arg) != args_.end();
    }

    bool IsArgSet(const std::string &argName) const
    {
        return args_.find(argName) != args_.end();
    }

    std::string GetHelpString() const
    {
        std::ostringstream helpstr;
        for (auto i : args_) {
            if (i->GetType() == PandArgType::COMPOUND) {
                auto arg = static_cast<PandArgCompound *>(i);
                helpstr << DOUBLE_DASH << i->GetName() << ": " << i->GetDesc() << "\n";
                helpstr << "  Sub arguments:\n";
                for (auto subArg : arg->GetSubArgs()) {
                    helpstr << "    " << subArg->GetName() << ": " << subArg->GetDesc() << "\n";
                }
            } else {
                helpstr << DOUBLE_DASH << i->GetName() << ": " << i->GetDesc() << "\n";
            }
        }
        return helpstr.str();
    }

    std::string GetTailArgs() const
    {
        std::ostringstream helpstr;
        if (!tailArgs_.empty()) {
            for (auto i : tailArgs_) {
                std::cerr << i->GetName() << ": " << i->GetDesc() << "\n";
            }
        }
        return helpstr.str();
    }

    std::string GetRegularArgs()
    {
        std::string argsStr;
        std::string value;
        for (auto i : args_) {
            switch (i->GetType()) {
                case PandArgType::STRING:
                    value = static_cast<PandArg<std::string> *>(i)->GetValue();
                    break;
                case PandArgType::INTEGER:
                    value = std::to_string(static_cast<PandArg<int> *>(i)->GetValue());
                    break;
                case PandArgType::DOUBLE:
                    value = std::to_string(static_cast<PandArg<double> *>(i)->GetValue());
                    break;
                case PandArgType::BOOL:
                    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
                    value = std::to_string(static_cast<PandArg<bool> *>(i)->GetValue());
                    break;
                case PandArgType::UINT32:
                    value = std::to_string(static_cast<PandArg<uint32_t> *>(i)->GetValue());
                    break;
                case PandArgType::UINT64:
                    value = std::to_string(static_cast<PandArg<uint64_t> *>(i)->GetValue());
                    break;
                case PandArgType::LIST: {
                    value = "";
                    std::vector<std::string> valuesBuf = static_cast<PandArg<arg_list_t> *>(i)->GetValue();
                    for (const auto &j : valuesBuf) {
                        value += j + ", ";
                    }
                    break;
                }
                default:
                    errstr_ += "Invalid argument type \"" + i->GetName() + "\"\n";
                    break;
            }
            argsStr += DOUBLE_DASH + i->GetName() + "=" + value + "\n";
        }
        return argsStr;
    }

    std::vector<std::string> GetNamesOfParsedArgs() const
    {
        std::vector<std::string> names;
        for (auto arg : args_) {
            names.emplace_back(arg->GetName());
            if (arg->GetType() == PandArgType::COMPOUND) {
                auto argCompound = static_cast<PandArgCompound *>(arg);
                for (auto subArg : argCompound->GetSubArgs()) {
                    std::stringstream ss;
                    ss << arg->GetName() << ':' << subArg->GetName();
                    names.emplace_back(ss.str());
                }
            }
        }
        return names;
    }

private:
    struct PandArgPtrComparator {
        // NOLINTNEXTLINE(readability-identifier-naming)
        using is_transparent = void;
        bool operator()(const PandArgBase *lhs, const PandArgBase *rhs) const
        {
            return lhs->GetName() < rhs->GetName();
        }
        bool operator()(std::string_view lhs, const PandArgBase *rhs) const
        {
            return lhs < rhs->GetName();
        }
        bool operator()(const PandArgBase *lhs, std::string_view rhs) const
        {
            return lhs->GetName() < rhs;
        }
    };

    bool ParseArgs()
    {
        while (argvIndex_ < argvVec_.size()) {
            PandArgBase *parsedArg = ParseNextArg();
            if (!errstr_.empty()) {
                return false;
            }
            if (argvIndex_ < argvVec_.size()) {
                argvIndex_ += ParseNextParam(parsedArg, argvVec_[argvIndex_]);
                if (!errstr_.empty()) {
                    return false;
                }
            }
        }
        return true;
    }

    void InitDefault()
    {
        equalFlag_ = false;
        tailParsedFlag_ = false;
        argvVec_.clear();
        argvIndex_ = 0;
        errstr_ = "";
        // reset tail
        for (auto tailArg : tailArgs_) {
            switch (tailArg->GetType()) {
                case PandArgType::STRING:
                    static_cast<PandArg<std::string> *>(tailArg)->ResetDefaultValue();
                    break;
                case PandArgType::INTEGER:
                    static_cast<PandArg<int> *>(tailArg)->ResetDefaultValue();
                    break;
                case PandArgType::DOUBLE:
                    static_cast<PandArg<double> *>(tailArg)->ResetDefaultValue();
                    break;
                case PandArgType::BOOL:
                    static_cast<PandArg<bool> *>(tailArg)->ResetDefaultValue();
                    break;
                case PandArgType::UINT32:
                    static_cast<PandArg<uint32_t> *>(tailArg)->ResetDefaultValue();
                    break;
                case PandArgType::UINT64:
                    static_cast<PandArg<uint64_t> *>(tailArg)->ResetDefaultValue();
                    break;
                case PandArgType::LIST:
                    static_cast<PandArg<arg_list_t> *>(tailArg)->ResetDefaultValue();
                    break;
                default:
                    break;
            }
        }
        // reset remainder
        remainder_ = arg_list_t();
    }

    PandArgBase *FindArg(std::string_view argName)
    {
        auto argIt = args_.find(argName);
        if (argIt == args_.end()) {
            errstr_.append("pandargs: Invalid option \"");
            errstr_.append(argName);
            errstr_.append("\"\n");
            return nullptr;
        }
        return *argIt;
    }

    bool ParseSubArgument(PandArgCompound *parentArg, std::string_view str)
    {
        size_t assignPos = str.find('=');
        std::string_view argName = str.substr(0, assignPos);
        auto arg = parentArg->FindSubArg(argName);
        if (arg == nullptr) {
            errstr_.append("pandargs: Invalid sub-argument \"");
            errstr_.append(argName);
            errstr_.append("\"\n");
            return false;
        }

        if (assignPos != std::string_view::npos) {
            std::string_view valueStr = str.substr(assignPos + 1);
            ParseNextParam(arg, valueStr);
            if (!errstr_.empty()) {
                return false;
            }
        } else {
            if (arg->GetType() != PandArgType::BOOL) {
                errstr_.append("pandargs: Only boolean arguments might have no value \"");
                errstr_.append(argName);
                errstr_.append("\"\n");
                return false;
            }
            static_cast<PandArg<bool> *>(arg)->SetValue(true);
        }

        return true;
    }

    PandArgBase *ParseCompoundArg(std::string_view argstr, size_t sepPos)
    {
        auto argName = argstr.substr(0, sepPos);

        auto arg = static_cast<PandArgCompound *>(FindArg(argName));
        if (arg == nullptr) {
            return nullptr;
        }
        if (arg->GetType() != PandArgType::COMPOUND) {
            errstr_.append("pandargs: Following argument is not compound one or syntax error: \"");
            errstr_.append(argName);
            errstr_.append("\"\n");
            return nullptr;
        }

        arg->SetValue(true);

        auto subArgsStr = argstr.substr(sepPos + 1);
        size_t start = 0;
        for (size_t pos = subArgsStr.find(',', 0); pos != std::string_view::npos;
             start = pos + 1, pos = subArgsStr.find(',', start)) {
            auto argStr = subArgsStr.substr(start, pos - start);
            if (!ParseSubArgument(arg, argStr)) {
                return nullptr;
            }
        }
        if (start < subArgsStr.size()) {
            if (!ParseSubArgument(arg, subArgsStr.substr(start))) {
                return nullptr;
            }
        }
        argvIndex_++;
        return arg;
    }

    PandArgBase *ParseNextRegularArg()
    {
        std::string argstr = argvVec_[argvIndex_];

        const std::size_t sepFound = NextSeparator(argstr);

        const std::size_t sepCompound = argstr.find_first_of(':', 0);
        if (sepCompound != std::string::npos && (sepFound == std::string::npos || sepCompound < sepFound)) {
            return ParseCompoundArg(std::string_view(argstr).substr(DASH_COUNT), sepCompound - DASH_COUNT);
        }

        std::string argName;

        if (sepFound != std::string::npos) {
            equalFlag_ = true;
            argvVec_[argvIndex_] = argstr.substr(sepFound + 1);
            argName = argstr.substr(DASH_COUNT, sepFound - DASH_COUNT);
        } else {
            argName = argstr.substr(DASH_COUNT, sepFound);
            // check if there is next argv element to iterate into
            if (argvIndex_ + 1 < argvVec_.size()) {
                argvIndex_++;
            } else {
                argvVec_[argvIndex_] = "";
            }
        }

        PandArgBase *arg = FindArg(argName);
        if (arg == nullptr) {
            return nullptr;
        }

        if (arg->GetType() == PandArgType::COMPOUND) {
            // It is forbidden to explicitly set compound option, e.g. `--compound=true`, must be `--compound`.
            if (sepFound != std::string::npos) {
                errstr_.append("pandargs: Compound option can not be explicitly set \"");
                errstr_.append(argName);
                errstr_.append("\"\n");
                return nullptr;
            }
            static_cast<PandArgCompound *>(arg)->SetValue(true);
        }

        return arg;
    }

    PandArgBase *ParseNextArg()
    {
        PandArgBase *arg = nullptr;
        std::string argstr = argvVec_[argvIndex_];
        equalFlag_ = false;

        // NOTE: currently we have only double dash argument prefix
        std::size_t dashesFound = argstr.find(DOUBLE_DASH);
        if (dashesFound == 0 && argstr.size() > DASH_COUNT) {
            // regular argument
            return ParseNextRegularArg();
        }

        if (dashesFound == 0 && argstr.size() == DASH_COUNT) {
            // remainder argument
            if (!remainderFlag_) {
                errstr_.append("pandargs: Remainder arguments are not enabled\n");
                errstr_.append("pandargs: Remainder found at literal \"");
                errstr_.append(argstr);
                errstr_.append("\"\n");
                return nullptr;
            }

            argvIndex_++;
            ParseRemainder();
        } else if (dashesFound > 0) {
            // tail argument, N.B. std::string::npos > 0
            if (!tailFlag_) {
                errstr_.append("pandargs: Tail arguments are not enabled\n");
                errstr_.append("pandargs: Tail found at literal \"");
                errstr_.append(argstr);
                errstr_.append("\"\n");
                return nullptr;
            }
            if (tailParsedFlag_) {
                errstr_.append("pandargs: Too many tail arguments\n");
                return nullptr;
            }
            ParseTail();

            if (argvIndex_ < argvVec_.size()) {
                if (argvVec_[argvIndex_] != DOUBLE_DASH && !remainderFlag_) {
                    errstr_ += "pandargs: Too many tail arguments given\n";
                }
            }
        } else {
            errstr_.append("pandargs: Invalid option \"");
            errstr_.append(argstr);
            errstr_.append("\"\n");
            UNREACHABLE();
        }
        return arg;
    }

    void ParseTail()
    {
        for (auto &tailArg : tailArgs_) {
            switch (tailArg->GetType()) {
                case PandArgType::STRING:
                    argvIndex_ +=
                        ParseStringArgParam(static_cast<PandArg<std::string> *>(tailArg), argvVec_[argvIndex_]);
                    break;
                case PandArgType::INTEGER:
                    argvIndex_ += ParseIntArgParam(static_cast<PandArg<int> *>(tailArg), argvVec_[argvIndex_]);
                    break;
                case PandArgType::DOUBLE:
                    argvIndex_ += ParseDoubleArgParam(static_cast<PandArg<double> *>(tailArg), argvVec_[argvIndex_]);
                    break;
                case PandArgType::BOOL:
                    argvIndex_ += ParseBoolArgParam(static_cast<PandArg<bool> *>(tailArg), argvVec_[argvIndex_], true);
                    break;
                case PandArgType::UINT32:
                    argvIndex_ += ParseUint32ArgParam(static_cast<PandArg<uint32_t> *>(tailArg), argvVec_[argvIndex_]);
                    break;
                case PandArgType::UINT64:
                    argvIndex_ += ParseUint64ArgParam(static_cast<PandArg<uint64_t> *>(tailArg), argvVec_[argvIndex_]);
                    break;
                case PandArgType::LIST:
                    argvIndex_ += ParseListArgParam(static_cast<PandArg<arg_list_t> *>(tailArg), argvVec_[argvIndex_]);
                    break;
                default:
                    errstr_.append("pandargs: Invalid tail option type: \"");
                    errstr_.append(tailArg->GetName());
                    errstr_.append("\"\n");
                    UNREACHABLE();
                    break;
            }
            if (argvIndex_ >= argvVec_.size() || !errstr_.empty()) {
                break;
            }
        }
        tailParsedFlag_ = true;
    }

    void ParseRemainder()
    {
        remainder_ = arg_list_t(argvVec_.begin() + static_cast<arg_list_t::iterator::difference_type>(argvIndex_),
                                argvVec_.end());
        argvIndex_ = argvVec_.size();
    }

    size_t ParseNextParam(PandArgBase *arg, std::string_view argstr)
    {
        if (argvIndex_ >= argvVec_.size() || arg == nullptr) {
            return 0;
        }
        switch (arg->GetType()) {
            case PandArgType::STRING:
                return ParseStringArgParam(static_cast<PandArg<std::string> *>(arg), argstr);
            case PandArgType::INTEGER:
                return ParseIntArgParam(static_cast<PandArg<int> *>(arg), argstr);
            case PandArgType::DOUBLE:
                return ParseDoubleArgParam(static_cast<PandArg<double> *>(arg), argstr);
            case PandArgType::BOOL:
                return ParseBoolArgParam(static_cast<PandArg<bool> *>(arg), argstr);
            case PandArgType::UINT32:
                return ParseUint32ArgParam(static_cast<PandArg<uint32_t> *>(arg), argstr);
            case PandArgType::UINT64:
                return ParseUint64ArgParam(static_cast<PandArg<uint64_t> *>(arg), argstr);
            case PandArgType::LIST:
                return ParseListArgParam(static_cast<PandArg<arg_list_t> *>(arg), argstr);
            case PandArgType::COMPOUND:
                return argstr.empty() ? 1 : 0;
            case PandArgType::NOTYPE:
                errstr_.append("pandargs: Invalid option type: \"");
                errstr_.append(arg->GetName());
                errstr_.append("\"\n");
                UNREACHABLE();
                break;
            default:
                UNREACHABLE();
                break;
        }
        return 0;
    }

    std::size_t ParseStringArgParam(PandArg<std::string> *arg, std::string_view argstr)
    {
        arg->SetValue(std::string(argstr));
        return 1;
    }

    std::size_t ParseIntArgParam(PandArg<int> *arg, std::string_view argstr)
    {
        std::string paramStr(argstr);
        if (IsIntegerNumber(paramStr)) {
            int64_t result = 0;
            errno = 0;
            if (StartsWith(paramStr, "0x")) {
                const int hex = 16;
                result = std::strtol(paramStr.c_str(), nullptr, hex);
            } else {
                const int dec = 10;
                result = std::strtol(paramStr.c_str(), nullptr, dec);
            }

            int num = static_cast<int>(result);
            if (num != result || errno == ERANGE) {
                errstr_ +=
                    "pandargs: \"" + arg->GetName() + "\" argument has invalid parameter value \"" + paramStr + "\"\n";

                return 1;
            }

            if (IsIntegerArgInRange(arg, num)) {
                arg->SetValue(num);
            } else {
                errstr_ += "pandargs: \"" + arg->GetName() + "\" argument has out of range parameter value \"" +
                           paramStr + "\"\n";
            }
        } else {
            errstr_ +=
                "pandargs: \"" + arg->GetName() + "\" argument has out of range parameter value \"" + paramStr + "\"\n";
        }
        return 1;
    }

    std::size_t ParseDoubleArgParam(PandArg<double> *arg, std::string_view argstr)
    {
        std::string paramStr(argstr);
        if (IsRationalNumber(paramStr)) {
            arg->SetValue(strtod(paramStr.data(), nullptr));
        } else {
            errstr_ +=
                "pandargs: \"" + arg->GetName() + "\" argument has invalid parameter value \"" + paramStr + "\"\n";
        }
        return 1;
    }

    std::size_t ParseBoolArgParam(PandArg<bool> *arg, std::string_view argstr, bool isTailParam = false)
    {
        std::string paramStr(argstr);

        // if not a tail argument, assume two following cases
        if (!isTailParam) {
            arg->SetValue(true);
            // bool with no param, next argument comes right after
            if (StartsWith(paramStr, DOUBLE_DASH)) {
                // check that bool param comes without "="
                if (equalFlag_) {
                    SetBoolUnexpectedValueError(arg, paramStr);
                }
                return 0;
            }
            // OR bool arg at the end of arguments line
            if (paramStr.empty()) {
                // check that bool param comes without "="
                if (equalFlag_) {
                    SetBoolUnexpectedValueError(arg, paramStr);
                }
                return 1;
            }
        }

        constexpr std::array<std::string_view, 3> TRUE_VALUES = {"on", "true", "1"};
        constexpr std::array<std::string_view, 3> FALSE_VALUES = {"off", "false", "0"};

        for (const auto &i : TRUE_VALUES) {
            if (paramStr == i) {
                arg->SetValue(true);
                return 1;
            }
        }
        for (const auto &i : FALSE_VALUES) {
            if (paramStr == i) {
                arg->SetValue(false);
                return 1;
            }
        }

        // if it's not a part of tail argument,
        // assume that it's bool with no param,
        // preceiding tail argument
        if (!isTailParam) {
            // check that bool param came without "="
            if (equalFlag_) {
                SetBoolUnexpectedValueError(arg, paramStr);
            } else {
                arg->SetValue(true);
            }
        } else {
            errstr_ +=
                "pandargs: Tail argument " + arg->GetName() + " has unexpected parameter value " + paramStr + "\n";
            arg->ResetDefaultValue();
        }

        return 0;
    }

    std::size_t ParseUint64ArgParam(PandArg<uint64_t> *arg, std::string_view argstr)
    {
        std::string paramStr(argstr);
        if (IsUintNumber(paramStr)) {
            errno = 0;
            uint64_t num;
            if (StartsWith(paramStr, "0x")) {
                const int hex = 16;
                num = std::strtoull(paramStr.c_str(), nullptr, hex);
            } else {
                const int dec = 10;
                num = std::strtoull(paramStr.c_str(), nullptr, dec);
            }
            if (errno == ERANGE) {
                errstr_ +=
                    "pandargs: \"" + arg->GetName() + "\" argument has invalid parameter value \"" + paramStr + "\"\n";

                return 1;
            }

            if (IsIntegerArgInRange<uint64_t>(arg, num)) {
                arg->SetValue(num);
            } else {
                errstr_ += "pandargs: \"" + arg->GetName() + "\" argument has out of range parameter value \"" +
                           paramStr + "\"\n";
            }
        } else {
            errstr_ +=
                "pandargs: \"" + arg->GetName() + "\" argument has invalid parameter value \"" + paramStr + "\"\n";
        }
        return 1;
    }

    std::size_t ParseUint32ArgParam(PandArg<uint32_t> *arg, std::string_view argstr)
    {
        std::string paramStr(argstr);
        if (IsUintNumber(paramStr)) {
            uint64_t result = 0;
            errno = 0;
            if (StartsWith(paramStr, "0x")) {
                const int hex = 16;
                result = std::strtoull(paramStr.c_str(), nullptr, hex);
            } else {
                const int dec = 10;
                result = std::strtoull(paramStr.c_str(), nullptr, dec);
            }

            auto num = static_cast<uint32_t>(result);
            if (num != result || errno == ERANGE) {
                errstr_ +=
                    "pandargs: \"" + arg->GetName() + "\" argument has invalid parameter value \"" + paramStr + "\"\n";

                return 1;
            }

            if (IsIntegerArgInRange<uint32_t>(arg, num)) {
                arg->SetValue(num);
            } else {
                errstr_ += "pandargs: \"" + arg->GetName() + "\" argument has out of range parameter value \"" +
                           paramStr + "\"\n";
            }
        } else {
            errstr_ +=
                "pandargs: \"" + arg->GetName() + "\" argument has invalid parameter value \"" + paramStr + "\"\n";
        }
        return 1;
    }

    std::size_t ParseListArgParam(PandArg<arg_list_t> *arg, std::string_view argstr)
    {
        std::string paramStr(argstr);
        arg_list_t value;
        if (arg->WasSet()) {
            value = arg->GetValue();
        } else {
            value = arg_list_t();
        }
        if (!arg->GetDelimiter().has_value()) {
            value.push_back(paramStr);
            arg->SetValue(value);
            return 1;
        }
        std::string delimiter = arg->GetDelimiter().value();
        std::size_t paramStrIndex = 0;
        std::size_t pos = paramStr.find_first_of(delimiter, paramStrIndex);
        while (pos < paramStr.size()) {
            value.push_back(paramStr.substr(paramStrIndex, pos - paramStrIndex));
            paramStrIndex = pos;
            paramStrIndex = paramStr.find_first_not_of(delimiter, paramStrIndex);
            pos = paramStr.find_first_of(delimiter, paramStrIndex);
        }

        if (paramStrIndex <= paramStr.size()) {
            value.push_back(paramStr.substr(paramStrIndex, pos - paramStrIndex));
        }
        arg->SetValue(value);
        return 1;
    }

    static std::size_t NextSeparator(std::string_view argstr, std::size_t pos = 0,
                                     const std::string &separarors = EQ_SEPARATOR)
    {
        return argstr.find_first_of(separarors, pos);
    }

    static bool IsIntegerNumber(const std::string_view &str)
    {
        if (str.empty()) {
            return false;
        }
        std::size_t pos = 0;
        // look for dash if it's negative one
        if (str[0] == '-') {
            pos++;
        }
        // look for hex-style integer
        if ((str.size() > pos + 2U) && (str.compare(pos, 2U, "0x") == 0)) {
            pos += HEX_PREFIX_WIDTH;
            return str.find_first_not_of("0123456789abcdefABCDEF", pos) == std::string::npos;
        }
        return str.find_first_not_of("0123456789", pos) == std::string::npos;
    }

    static bool IsRationalNumber(const std::string_view &str)
    {
        if (str.empty()) {
            return false;
        }
        std::size_t pos = 0;
        // look for dash if it's negative one
        if (str[0] == '-') {
            pos++;
        }
        return str.find_first_not_of(".0123456789", pos) == std::string::npos;
    }

    static bool IsUintNumber(const std::string_view &str)
    {
        if (str.empty()) {
            return false;
        }

        constexpr std::string_view CHAR_SEARCH = "0123456789";
        constexpr std::string_view CHAR_SEARCH_HEX = "0123456789abcdef";
        std::size_t pos = 0;
        // look for hex-style uint_t integer
        if ((str.size() > 2U) && (str.compare(0, 2U, "0x") == 0)) {
            pos += HEX_PREFIX_WIDTH;
        }
        return str.find_first_not_of((pos != 0) ? CHAR_SEARCH_HEX.data() : CHAR_SEARCH.data(), pos) ==
               std::string::npos;
    }

    template <typename T,
              // CC-OFFNXT(G.FMT.10) project code style
              enable_if_t<is_same_v<T, int> || is_same_v<T, uint32_t> || is_same_v<T, uint64_t>> * = nullptr>
    bool IsIntegerArgInRange(PandArg<T> *arg, T num)
    {
        if (!(arg->GetMinMaxVal().has_value())) {
            return true;
        }
        std::pair<T, T> minMax = arg->GetMinMaxVal().value();
        return ((num >= std::get<0>(minMax)) && (num <= std::get<1>(minMax)));
    }

    static bool StartsWith(const std::string &haystack, const std::string &needle)
    {
        return std::equal(needle.begin(), needle.end(), haystack.begin());
    }

    void SetBoolUnexpectedValueError(PandArg<bool> *arg, const std::string &wrongvalue)
    {
        errstr_ += "pandargs: Bool argument " + arg->GetName() + " has unexpected parameter value " + wrongvalue + "\n";
        arg->ResetDefaultValue();
    }

    constexpr static size_t HEX_PREFIX_WIDTH = 2;
    constexpr static unsigned int DASH_COUNT = 2;

    std::vector<std::string> argvVec_ {};
    std::size_t argvIndex_ = 0;
    std::string errstr_ {};
    bool tailFlag_ = false;
    bool remainderFlag_ = false;
    bool equalFlag_ = false;
    bool tailParsedFlag_ = false;
    static constexpr const char *DOUBLE_DASH = "--";
    static constexpr const char *EQ_SEPARATOR = "=";
    std::set<PandArgBase *, PandArgPtrComparator> args_ {};
    std::vector<PandArgBase *> tailArgs_ {};
    arg_list_t remainder_ = arg_list_t();
};

}  // namespace ark

#endif  // LIBPANDABASE_UTILS_PANDARGS_H_
