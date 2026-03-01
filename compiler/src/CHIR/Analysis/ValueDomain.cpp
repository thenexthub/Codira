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
 * This file implements the abstract domain of CHIR value.
 */

#include "Codira/CHIR/Analysis/ValueDomain.h"

namespace Codira::CHIR {
AbstractObject::AbstractObject(std::string identifier) : Value(nullptr, identifier, ValueKind::KIND_LOCALVAR)
{
}

std::string AbstractObject::ToString() const
{
    return identifier;
}

AbstractObject* AbstractObject::GetTopObjInstance()
{
    static AbstractObject ins{"TopObj"};
    return &ins;
}

bool AbstractObject::IsTopObjInstance() const
{
    return this == GetTopObjInstance();
}

Ref::Ref(std::string uniqueID, bool isStatic) : isStatic(isStatic), uniqueID(std::move(uniqueID))
{
}

std::string Ref::GetUniqueID() const
{
    return isStatic ? "s" + uniqueID : uniqueID;
}

void Ref::AddRoots(Ref* r1, Ref* r2)
{
    const auto add = [this](Ref* r) {
        if (r->roots.empty()) {
            roots.emplace(r);
        } else {
            roots.insert(r->roots.begin(), r->roots.end());
        }
    };
    add(r1);
    add(r2);
}

bool Ref::IsEquivalent(Ref* r)
{
    return !roots.empty() && roots == r->roots;
}

bool Ref::CanRepresent(Ref* r)
{
    if (auto cacheRes = CheckCache(r); cacheRes.has_value()) {
        return cacheRes.value();
    } else {
        bool res;
        if (roots.empty()) {
            // this is a root ref
            res = false;
        } else if (r->roots.empty()) {
            // rhs is a root ref
            res = roots.find(r) != roots.end();
        } else {
            const auto check = [this](const auto& x) { return roots.find(x) != roots.end(); };
            res = r->roots.size() <= roots.size() && std::all_of(r->roots.begin(), r->roots.end(), check);
        }
        WriteCache(r, res);

        return res;
    }
}

std::optional<bool> Ref::CheckCache(Ref* r)
{
    std::unique_lock<std::mutex> guard(cacheMtx, std::defer_lock);
    if (isStatic) {
        guard.lock();
    }
    if (auto it = cache.find(r); it != cache.end()) {
        return it->second;
    } else {
        return std::nullopt;
    }
}

void Ref::WriteCache(Ref* r, bool res)
{
    std::unique_lock<std::mutex> guard(cacheMtx, std::defer_lock);
    if (isStatic) {
        guard.lock();
    }
    cache.emplace(r, res);
}

Ref* Ref::GetTopRefInstance()
{
    static Ref ins{"TopRef", false};
    return &ins;
}

bool Ref::IsTopRefInstance() const
{
    return this == GetTopRefInstance();
}
}  // namespace Codira::CHIR
