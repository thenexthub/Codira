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
 * This file declares the OptionTable related classes.
 */

#ifndef CODIRA_OPTION_OPTIONTABLE_H
#define CODIRA_OPTION_OPTIONTABLE_H

#include <memory>
#include <optional>
#include <vector>

#include "Codira/Utils/Utils.h"

namespace Codira {
class Arg;
class OptionArgInstance;
class ArgList;

namespace Options {
enum class Kind : uint8_t {
    UNKNOWN = 0,
#define KIND(kind) kind,
#include "Codira/Option/Options.inc"
#undef KIND
};

enum class Backend : uint8_t {
    UNKNOWN = 0,
#define BACKEND_DEF(backend) backend,
#include "Codira/Option/Options.inc"
#undef BACKEND_DEF
};

enum class Group : uint8_t {
    UNKNOWN = 0,
#define GROUP_DEF(group) group,
#include "Codira/Option/Options.inc"
#undef GROUP_DEF
};

enum class ID : uint16_t {
#define OPTION(NAME, ID, KIND, BACKENDS, GROUPS, ALIAS, FLAGS, HELPTEXT, OCCURRENCE) ID,
#include "Codira/Option/Options.inc"
#undef OPTION
};

enum class Occurrence : uint16_t {
    UNKNOWN = 0,
#define OCCURRENCE(occurrence) occurrence,
#include "Codira/Option/Options.inc"
#undef OCCURRENCE
};

enum class Visibility : uint8_t { VISIBLE, INVISIBLE };

enum class OptionType { GENERAL, EXPERIMENTAL };

struct OptionValue {
    const std::string value;
    const std::string help;
    std::vector<Options::Backend> backends{0};
    std::vector<Options::Group> groups{0};
};
} // namespace options

/**
 * Option table to handle raw options information, parse arguments.
 */
class OptionTable {
public:
    /**
     * This struct is used to store all information in the definition of the OPTION, whose format is like:
     * OPTION(NAME, ID, KIND, GROUP, ALIAS, FLAGS, HELPTEXT).
     */
    struct OptionInfo {
        const std::string name;                    /**< NAME */
        uint16_t id{0};                            /**< ID */
        Options::Kind kind{0};                     /**< KIND */
        std::vector<Options::Backend> backends{0}; /**< Backends the option supports. */
        std::vector<Options::Group> groups{0};     /**< Groups the option belongs to */
        const char* alias{nullptr};                /**< ALIAS */
        std::vector<Options::OptionValue> values;  /**< FLAGS, pre-defined flags list */
        Options::Occurrence occurrence;            /**< OCCURRENCE, occurrence type, warn multiple time usage or not */
        const std::string help;                    /**< HELPTEXT */
        Options::Visibility visible{0};            /**< VISIBILITY */
        Options::OptionType optionType = Options::OptionType::GENERAL;

        Options::ID GetID() const
        {
            return static_cast<Codira::Options::ID>(id);
        }

        std::string GetAlias() const
        {
            if (alias == nullptr) {
                return "";
            }
            return std::string(alias);
        }

        Options::Kind GetKind() const
        {
            return kind;
        }

        std::vector<Options::Group> GetGroups() const
        {
            return groups;
        }

        bool BelongsGroup(Options::Group group) const
        {
            return std::count(groups.begin(), groups.end(), group) != 0;
        }

        bool BelongsToAnyOfGroup(const std::set<Options::Group>& targets) const
        {
            for (Options::Group target : targets) {
                if (std::count(groups.begin(), groups.end(), target) != 0) {
                    return true;
                }
            }
            return false;
        }

        bool BelongsToBackend(Options::Backend target) const
        {
            return std::any_of(backends.begin(), backends.end(), [target](Options::Backend backend) {
                return backend == Options::Backend::ALL || backend == target;
                });
        }

        std::string GetName() const
        {
            return name;
        }

        std::vector<Options::OptionValue> GetOptionValues() const
        {
            std::vector<Options::OptionValue> optionValues{};
            auto& opvalues = values;
            for (Options::OptionValue optionValue : opvalues) {
                optionValues.emplace_back(optionValue);
            }
            return optionValues;
        }

        Options::Occurrence GetOccurrenceType() const
        {
            return occurrence;
        }
    };

    /**
     * @brief The constructor of class OptionTable.
     *
     * @param infos The option information vector.
     * @param frontendMode The true value is frontend mode.
     * @return OptionTable The instance of OptionTable.
     */
    explicit OptionTable(const std::vector<OptionInfo>& infos, bool frontendMode) : optionInfos(infos),
        frontendMode(frontendMode) {
        enabledGroups.emplace(frontendMode ? Options::Group::FRONTEND : Options::Group::DRIVER);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        enabledBackends.emplace(Options::Backend::CODENATIVE);
#endif
    }

    /**
     * @brief The destructor of class OptionTable.
     */
    ~OptionTable()
    {
    }

    /**
     * @brief Helper function for determining if the objects appear in targets.
     *
     * @param targets The target format information.
     * @param objects The object vector.
     * @param universal The object kind.
     * @return bool if yes, true is returned. Otherwise, false is returned.
     */
    template <typename T>
    static bool BelongsTo(const std::set<T>& targets, const std::vector<T>& objects, const T universal)
    {
        for (const auto& object : objects) {
            if (object == universal) {
                return true;
            }
            if (std::count(targets.begin(), targets.end(), object) != 0) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Parse all arguments from raw arguments string. Including parsing files and separated options.
     */
    bool ParseArgs(const std::vector<std::string>& argsStrs, ArgList& argList);

    /**
     * @brief Print help text.
     */
    void Usage(Options::Backend backend, std::set<Options::Group> groups, bool showExperimental = false);

    enum class ArgType {
        FullyParsed,
        PartiallyParsed,
    };

    /**
     * @brief Option information table.
     */
    const std::vector<OptionInfo> optionInfos;

private:
    const static int OPTION_WIDTH = 28; /**< The option total width. */
    const bool frontendMode = false;

    std::set<Options::Backend> enabledBackends {};
    std::set<Options::Group> enabledGroups { Options::Group::GLOBAL };

    /**
     * @brief A help function for parsing joined arguments.
     * The first element of return is argument name and the second element of return is argument value.
     * Joined argument '-joined-arg=value' has argument name '-joined-arg' and argument value 'value'.
     * For other type of arguments, argument name is itself, argument value is nullopt.
     */
    std::pair<std::string, std::optional<std::string>> ParseArgumentName(const std::string& argStr) const;

    /**
     * @brief A help method for ParseArgs. This argument parser only process an argument string without any
     * spaces in it. As a result, ParseOneArg accepts Flag kind and Joined kind Arguments, for example:
     * -xxx and -yyy=111.
     * In the case of Separated kind arguments, the argument is parsed partically. We have to get the next
     * argument to finish parsing a separated argument option.
     */
    std::optional<OptionArgInstance> ParseOptionArg(const std::string& argStr);

    /**
     * @brief A help method that sets value for an Arg object.
     * It returns nullopt if given value is invalid for the Arg.
     */
    std::optional<OptionArgInstance> SetArgValue(
        OptionArgInstance& arg, const std::string& value, bool isSeparatedOptionWithoutSpace) const;

    /**
     * @brief Consume one or more arguments to get a complete Arg object and insert into given ArgList.
     */
    bool ParseShortTermArgs(const std::vector<std::string>& argsStrs, size_t& idx, ArgList& argList);

    bool ParseSeparatedArgValue(OptionArgInstance& arg, const std::vector<std::string>& argsStrs, size_t& idx) const;

    bool ShouldArgumentBeRecognized(const std::string& argName, bool hasValue, const OptionTable::OptionInfo& i);

    void PrintInfo(const OptionInfo& info, Options::Backend backend, bool showExperimental) const;
};

enum class ArgInstanceType {
    Input,
    Option
};

class ArgInstance {
public:
    const ArgInstanceType argInstanceType;

    /**
     * The string what user actually input for this option.
     */
    std::string str;

    virtual ~ArgInstance() = default;

protected:
    ArgInstance(ArgInstanceType argInstanceType) : argInstanceType(argInstanceType)
    {
    }
};

class InputArgInstance : public ArgInstance {
public:
    /**
     * The value of input
     */
    const std::string value;

    InputArgInstance(const std::string& value) : ArgInstance(ArgInstanceType::Input), value(value)
    {
        str = value;
    }

    ~InputArgInstance() = default;
};

class OptionArgInstance : public ArgInstance {
public:
    /**
     * The metadata of the argument, i.e. which option the argument belongs to.
     */
    const OptionTable::OptionInfo& info;

    /**
     * The name of argument.
     */
    std::string name;

    OptionTable::ArgType argType = OptionTable::ArgType::PartiallyParsed;

    bool hasJoinedValue = false;

    /**
     * The argument of option. For options with no arguments, the value is empty.
     */
    std::string value;

    OptionArgInstance(const OptionTable::OptionInfo& info, const std::string& name, const std::string& value)
        : ArgInstance(ArgInstanceType::Option), info(info), name(name), value(value)
    {
    }

    OptionArgInstance(const OptionTable::OptionInfo& info, const std::string& name)
        : OptionArgInstance(info, name, "")
    {
    }

    ~OptionArgInstance() = default;
};

class ArgList {
public:
    std::vector<std::unique_ptr<ArgInstance>> args;

    ArgList()
    {
        args = std::vector<std::unique_ptr<ArgInstance>>();
    }

    ~ArgList() = default;

    void MarkSpecified(const OptionArgInstance& arg)
    {
        isSpecified.emplace(arg.info.GetID());
    }

    bool IsSpecified(Options::ID id) const
    {
        return isSpecified.find(id) != isSpecified.end();
    }

    void AddInput(const std::string& input)
    {
        inputs.emplace_back(input);
    }

    std::vector<std::string> GetInputs() const
    {
        return inputs;
    }

    void MarkWarned(Options::ID id)
    {
        warnedList.emplace(id);
    }

    bool IsWarned(Options::ID id) const
    {
        return warnedList.find(id) != warnedList.end();
    }

private:
    std::unordered_set<Options::ID> isSpecified;
    std::unordered_set<Options::ID> warnedList;
    std::vector<std::string> inputs;
};

std::unique_ptr<OptionTable> CreateOptionTable(bool frontendMode = false);
} // namespace Codira
#endif // CODIRA_OPTION_OPTIONTABLE_H
