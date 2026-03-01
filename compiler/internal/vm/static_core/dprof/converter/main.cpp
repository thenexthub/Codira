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

#include "libarkbase/macros.h"
#include "features_manager.h"
#include "dprof/storage.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkbase/utils/span.h"
#include "features/hotness_counters.h"

#include <iostream>

#include "generated/converter_options.h"

namespace ark::dprof {
class ArgsParser {
public:
    bool Parse(ark::Span<const char *> args)
    {
        options_.AddOptions(&parser_);
        if (!parser_.Parse(args.Size(), args.Data())) {
            std::cerr << parser_.GetErrorString();
            return false;
        }
        auto err = options_.Validate();
        if (err) {
            std::cerr << err.value().GetMessage() << std::endl;
            return false;
        }
        if (options_.GetStorageDir().empty()) {
            std::cerr << "Option \"storage-dir\" is not set" << std::endl;
            return false;
        }
        return true;
    }

    const Options &GetOptionos() const
    {
        return options_;
    }

    void Help() const
    {
        std::cerr << "Usage: " << appName_ << " [OPTIONS]" << std::endl;
        std::cerr << "optional arguments:" << std::endl;
        std::cerr << parser_.GetHelpString() << std::endl;
    }

private:
    std::string appName_;
    PandArgParser parser_;
    Options options_ {""};
};

int Main(ark::Span<const char *> args)
{
    ArgsParser parser;
    if (!parser.Parse(args)) {
        parser.Help();
        return -1;
    }
    const Options &options = parser.GetOptionos();

    Logger::InitializeStdLogging(Logger::LevelFromString(options.GetLogLevel()), ark::LOGGER_COMPONENT_MASK_ALL);

    auto storage = AppDataStorage::Create(options.GetStorageDir());
    if (!storage) {
        LOG(FATAL, DPROF) << "Cannot init storage";
        return -1;
    }

    FeaturesManager fm;
    HCountersFunctor hcountersFunctor(std::cout);
    if (!fm.RegisterFeature(HCOUNTERS_FEATURE_NAME, hcountersFunctor)) {
        LOG(FATAL, DPROF) << "Cannot register feature: " << HCOUNTERS_FEATURE_NAME;
        return -1;
    }

    storage->ForEachApps([&fm](std::unique_ptr<AppData> &&appData) -> bool { return fm.ProcessingFeatures(*appData); });

    if (hcountersFunctor.ShowInfo(options.GetFormat())) {
        return -1;
    }
    return 0;
}
}  // namespace ark::dprof

int main(int argc, const char *argv[])
{
    ark::Span<const char *> args(argv, argc);
    return ark::dprof::Main(args);
}
