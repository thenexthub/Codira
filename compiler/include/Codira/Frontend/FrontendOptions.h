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
 * This file declares the FrontendOptions, which parses frontend arguments.
 */

#ifndef CODIRA_FRONTEND_FRONTENDOPTIONS_H
#define CODIRA_FRONTEND_FRONTENDOPTIONS_H

#include <string>

#include "Codira/Option/Option.h"

namespace Codira {
class FrontendOptions : public GlobalOptions {
public:
    virtual ~FrontendOptions() { }

    enum class DumpAction {
        NO_ACTION,        /**< No specific action. */
        DUMP_TOKENS,      /**< Dump tokens. */
        DUMP_SYMBOLS,     /**< Dump symbols after semantic check. */
        TYPE_CHECK,       /**< Parse ast and do typecheck. */
        DUMP_DEP_PKG,      /**< Dump dependent packages of current package. */
        DESERIALIZE_CHIR, /**< Deserialize Chir. */
    };

    FrontendOptions() : dumpAction(DumpAction::NO_ACTION) {}

    /**
     * Indicates the dump action the user requested that the frontend execute.
     */
    DumpAction dumpAction = DumpAction::NO_ACTION;

protected:
    virtual std::optional<bool> ParseOption(OptionArgInstance& arg) override;
};
} // namespace Codira
#endif // CODIRA_FRONTEND_FRONTENDOPTIONS_H
