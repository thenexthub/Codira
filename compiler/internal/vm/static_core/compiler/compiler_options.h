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

#ifndef COMPILER_COMPILER_OPTIONS_H
#define COMPILER_COMPILER_OPTIONS_H

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/cpu_features.h"
#include "compiler_options_gen.h"

#include <regex>

namespace ark::compiler {

#include "cpu_features.inc"

enum CpuFeature : uint8_t {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF(COMPONENT, ...) COMPONENT,
    CPU_FEATURE(DEF)
#undef DEF
        CPU_FEATURES_NUM
};

class CompilerOptions;
PANDA_PUBLIC_API extern CompilerOptions g_options;

/**
 * Extends `compiler::Options`, which may be not sufficient to provide the desired functionality
 * (e.g. store an option-related variable)
 */
class CompilerOptions : public Options {
public:
    explicit CompilerOptions(const std::string &exePath) : Options(exePath) {}
    NO_MOVE_SEMANTIC(CompilerOptions);
    NO_COPY_SEMANTIC(CompilerOptions);
    ~CompilerOptions() = default;

    /**
     * `--compiler-regex` extension.
     * The purpose of this extension is to avoid unnecessary construction of std::regex from
     * `Options::GetCompilerRegex()` on every call to `MatchesRegex()`.
     *
     * Static local variable doesn't suit as soon as `Options::SetCompilerRegex()` is used (e.g. in
     * tests).
     */
    void SetCompilerRegexWithSignature([[maybe_unused]] const std::string &newRegexPattern)
    {
        LOG(FATAL, COMPILER) << "'SetCompilerRegexWithSignature()' is not supported.";
    }

    void SetCompilerRegex(const std::string &newRegexPattern)
    {
        ASSERT(!WasSetCompilerRegexWithSignature());
        Options::SetCompilerRegex(newRegexPattern);
        regex_ = newRegexPattern;
    }

    template <typename T>
    bool MatchesRegex(const T &methodName)
    {
        if (!WasSetCompilerRegex() && !WasSetCompilerRegexWithSignature()) {
            return true;
        }
        if (!regexInitialized_) {
            if (WasSetCompilerRegexWithSignature() && WasSetCompilerRegex()) {
                LOG(FATAL, COMPILER)
                    << "'--compiler-regex' and '--compiler-regex-with-signature' cannot be used together.";
            }
            regex_ = WasSetCompilerRegex() ? GetCompilerRegex() : GetCompilerRegexWithSignature();
            regexInitialized_ = true;
        }
        return std::regex_match(methodName, regex_);
    }

    void AdjustCpuFeatures(bool crossCompilation)
    {
        ParseEnabledCpuFeatures();
        if (crossCompilation || WasSetCompilerCpuFeatures()) {
            return;
        }
        switch (RUNTIME_ARCH) {
            case Arch::AARCH64: {
                if (CpuFeaturesHasCrc32()) {
                    EnableCpuFeature(CRC32);
                }
                if (CpuFeaturesHasJscvt()) {
                    EnableCpuFeature(JSCVT);
                }
                if (CpuFeaturesHasAtomics()) {
                    EnableCpuFeature(ATOMICS);
                }
                break;
            }
            case Arch::AARCH32:
                break;
            case Arch::X86:
                break;
            case Arch::X86_64:
                break;
            case Arch::NONE:
            default:
                break;
        }
    }

    bool IsCpuFeatureEnabled(CpuFeature feature) const
    {
        return features_.test(feature);
    }

private:
    void EnableCpuFeature(CpuFeature feature)
    {
        features_.set(feature);
    }

    void ParseEnabledCpuFeatures()
    {
        for (const auto &arg : GetCompilerCpuFeatures()) {
            if (arg == "none") {
                features_.reset();
                break;
            }
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CONTINUE_DEF(FEATURE, NAME) \
    if ((NAME) == arg) {            \
        EnableCpuFeature(FEATURE);  \
        continue;                   \
    }
            CPU_FEATURE(CONTINUE_DEF)
#undef CONTINUE_DEF

            UNREACHABLE();
        }
    }

    // `--compiler-regex`:
    std::regex regex_;
    bool regexInitialized_ {false};
    std::bitset<CPU_FEATURES_NUM> features_;
};

}  // namespace ark::compiler
#endif  // COMPILER_COMPILER_OPTIONS_H
