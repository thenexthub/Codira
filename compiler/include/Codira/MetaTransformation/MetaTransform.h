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
 * This file declares the MetaTransform related classes.
 */

#ifndef CODIRA_METATRANSFORMPLUGINBUILDER_H
#define CODIRA_METATRANSFORMPLUGINBUILDER_H

#include <functional>
#include <memory>
#include <vector>

namespace Codira {
namespace CHIR {
class CHIRBuilder;
class Func;
class Package;
} // namespace CHIR

enum class MetaTransformKind {
    UNKNOWN,
    FOR_CHIR_FUNC,
    FOR_CHIR_PACKAGE,
    FOR_CHIR,
};

struct MetaTransformConcept {
    virtual ~MetaTransformConcept() = default;

    bool IsForCHIR() const
    {
        return kind > MetaTransformKind::UNKNOWN && kind < MetaTransformKind::FOR_CHIR;
    }

    bool IsForFunc() const
    {
        return kind == MetaTransformKind::FOR_CHIR_FUNC;
    }

    bool IsForPackage() const
    {
        return kind == MetaTransformKind::FOR_CHIR_PACKAGE;
    }

protected:
    MetaTransformKind kind = MetaTransformKind::UNKNOWN;
};

/**
 * An abstract concept for MetaTransform
 * @tparam DeclT (any limitations?)
 */
template <typename DeclT> struct MetaTransform : public MetaTransformConcept {
public:
    virtual void Run(DeclT&) = 0;

    MetaTransform() : MetaTransformConcept()
    {
        if constexpr (std::is_same_v<DeclT, CHIR::Func>) {
            kind = MetaTransformKind::FOR_CHIR_FUNC;
        } else if constexpr (std::is_same_v<DeclT, CHIR::Package>) {
            kind = MetaTransformKind::FOR_CHIR_PACKAGE;
        } else {
            kind = MetaTransformKind::UNKNOWN;
        }
    }

    virtual ~MetaTransform() = default;
};

struct MetaKind {
    struct CHIR;
};

/**
 * Manages a sequence plugins over a particular metadata.
 * @tparam MetaKind
 */
template <typename MetaKindT> class MetaTransformPluginManager {
public:
    explicit MetaTransformPluginManager() = default;
    MetaTransformPluginManager(MetaTransformPluginManager&& metaTransformPluginManager)
        : mtConcepts(std::move(metaTransformPluginManager.mtConcepts))
    {
    }
    MetaTransformPluginManager& operator=(MetaTransformPluginManager&& rhs)
    {
        mtConcepts = std::move(rhs.mtConcepts);
        return *this;
    }
    ~MetaTransformPluginManager() = default;

    template <typename MT> void AddMetaTransform(std::unique_ptr<MT> mt)
    {
        mtConcepts.emplace_back(std::move(mt));
    }

    void ForEachMetaTransformConcept(std::function<void(MetaTransformConcept&)> action)
    {
        for (auto& mtc : mtConcepts) {
            action(*mtc);
        }
    }

private:
    std::vector<std::unique_ptr<MetaTransformConcept>> mtConcepts;
};

using CHIRPluginManager = MetaTransformPluginManager<MetaKind::CHIR>;
extern template class MetaTransformPluginManager<MetaKind::CHIR>;

class MetaTransformPluginBuilder {
public:
    void RegisterCHIRPluginCallback(std::function<void(CHIRPluginManager&, CHIR::CHIRBuilder&)> callback)
    {
        chirPluginCallbacks.emplace_back(callback);
    }

    CHIRPluginManager BuildCHIRPluginManager(CHIR::CHIRBuilder& builder);

private:
    std::vector<std::function<void(CHIRPluginManager&, CHIR::CHIRBuilder&)>> chirPluginCallbacks;
};

/**
 * Information of a MetaTransform.
 */
struct MetaTransformPluginInfo {
    const char* codecVersion;
    void (*registerTo)(MetaTransformPluginBuilder&);
    /* some other members: such as name, orders, etc. */
};

#define CHIR_PLUGIN(plugin_name)                                                                        \
    namespace Codira {                                                                                                \
    extern const std::string CODIRA_VERSION;                                                                          \
    }                                                                                                                  \
    extern "C" MetaTransformPluginInfo getMetaTransformPluginInfo()                                                    \
    {                                                                                                                  \
        return {Codira::CODIRA_VERSION.c_str(), [](MetaTransformPluginBuilder& mtBuilder) {                          \
                    mtBuilder.RegisterCHIRPluginCallback([](CHIRPluginManager& mtm, CHIR::CHIRBuilder& builder) {     \
                        mtm.AddMetaTransform(std::make_unique<plugin_name>(builder));                                  \
                    });                                                                                                \
                }};                                                                                                    \
    }

} // namespace Codira

#endif
