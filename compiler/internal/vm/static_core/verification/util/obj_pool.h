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

#ifndef PANDA_VERIFIER_UTIL_OBJ_POOL_HPP_
#define PANDA_VERIFIER_UTIL_OBJ_POOL_HPP_

#include "libarkbase/macros.h"

#include <cstdint>
#include <optional>
#include <utility>

namespace ark::verifier {

template <typename T, template <typename...> class Vector, typename InitializerType = void (*)(T &, std::size_t),
          typename CleanerType = void (*)(T &)>
class ObjPool {
public:
    class Accessor {
    public:
        Accessor() : idx_ {0}, pool_ {nullptr}, prev_ {nullptr}, next_ {nullptr} {}
        Accessor(std::size_t index, ObjPool *objPool) : idx_ {index}, pool_ {objPool}, prev_ {nullptr}, next_ {nullptr}
        {
            Insert();
        }
        Accessor(const Accessor &p) : idx_ {p.idx_}, pool_ {p.pool_}, prev_ {nullptr}, next_ {nullptr}
        {
            Insert();
        }
        Accessor(Accessor &&p) : idx_ {p.idx_}, pool_ {p.pool_}, prev_ {p.prev_}, next_ {p.next_}
        {
            p.Reset();
            Rebind();
        }
        Accessor &operator=(const Accessor &p)
        {
            if (&p == this) {
                return *this;
            }
            Erase();
            Reset();
            idx_ = p.idx_;
            pool_ = p.pool_;
            Insert();
            return *this;
        }
        Accessor &operator=(Accessor &&p)
        {
            Erase();
            idx_ = p.idx_;
            pool_ = p.pool_;
            prev_ = p.prev_;
            next_ = p.next_;
            p.Reset();
            Rebind();
            return *this;
        }
        ~Accessor()
        {
            Erase();
        }
        T &operator*()
        {
            return pool_->storage_[idx_];
        }
        const T &operator*() const
        {
            return pool_->storage_[idx_];
        }
        // NOLINTNEXTLINE(google-explicit-constructor)
        operator bool() const
        {
            return pool_ != nullptr;
        }
        void Free()
        {
            Erase();
            Reset();
        }

    private:
        void Reset()
        {
            idx_ = 0;
            pool_ = nullptr;
            prev_ = nullptr;
            next_ = nullptr;
        }
        void Insert()
        {
            if (pool_ != nullptr) {
                next_ = pool_->accessors_[idx_];
                if (next_ != nullptr) {
                    next_->prev_ = this;
                }
                pool_->accessors_[idx_] = this;
            }
        }
        void Erase()
        {
            if (pool_ != nullptr) {
                if (prev_ == nullptr) {
                    pool_->accessors_[idx_] = next_;
                    if (pool_->accessors_[idx_] == nullptr) {
                        pool_->cleaner_(pool_->storage_[idx_]);
                        pool_->free_.push_back(idx_);
                    }
                } else {
                    prev_->next_ = next_;
                }
                if (next_ != nullptr) {
                    next_->prev_ = prev_;
                }
            }
        }
        void Rebind()
        {
            if (pool_ != nullptr) {
                if (prev_ != nullptr) {
                    prev_->next_ = this;
                } else {
                    pool_->accessors_[idx_] = this;
                }
                if (next_ != nullptr) {
                    next_->prev_ = this;
                }
            }
        }

        std::size_t idx_;
        ObjPool *pool_;
        Accessor *prev_;
        Accessor *next_;

        friend class ObjPool;
    };

    ObjPool(InitializerType initializer, CleanerType cleaner) : initializer_ {initializer}, cleaner_ {cleaner} {}
    explicit ObjPool(InitializerType initializer) : initializer_ {initializer}, cleaner_ {[](T &) { return; }} {}
    ObjPool() : initializer_ {[](T &, std::size_t) { return; }}, cleaner_ {[](T &) { return; }} {}

    ~ObjPool() = default;

    DEFAULT_MOVE_SEMANTIC(ObjPool);
    DEFAULT_COPY_SEMANTIC(ObjPool);

    Accessor New()
    {
        std::size_t idx;
        if (FreeCount() > 0) {
            idx = free_.back();
            free_.pop_back();
        } else {
            idx = storage_.size();
            storage_.emplace_back();
            accessors_.emplace_back(nullptr);
        }
        initializer_(storage_[idx], idx);
        return Accessor {idx, this};
    }

    std::size_t FreeCount() const
    {
        return free_.size();
    }

    std::size_t Count() const
    {
        return storage_.size();
    }

    std::size_t AccCount() const
    {
        size_t count = 0;
        for (auto el : accessors_) {
            Accessor *acc = el;
            while (acc != nullptr) {
                ++count;
                acc = acc->next_;
            }
        }
        return count;
    }

    auto AllObjects()
    {
        thread_local size_t idx = 0;
        return [this]() -> std::optional<Accessor> {
            while (idx < storage_.size() && accessors_[idx] == nullptr) {
                ++idx;
            }
            if (idx >= storage_.size()) {
                idx = 0;
                return std::nullopt;
            }
            return {Accessor {idx++, this}};
        };
    }

    void ShrinkToFit()
    {
        size_t idx1 = 0;
        size_t idx2 = storage_.size() - 1;
        while (idx1 < idx2) {
            while ((idx1 < idx2) && (accessors_[idx1] != nullptr)) {
                ++idx1;
            }
            while ((idx1 < idx2) && (accessors_[idx2] == nullptr)) {
                --idx2;
            }
            if (idx1 < idx2) {
                storage_[idx1] = std::move(storage_[idx2]);
                accessors_[idx1] = accessors_[idx2];
                Accessor *acc = accessors_[idx1];
                while (acc != nullptr) {
                    acc->idx_ = idx1;
                    acc = acc->next_;
                }
                --idx2;
            }
        }
        if (accessors_[idx1] != nullptr) {
            ++idx1;
        }
        storage_.resize(idx1);
        storage_.shrink_to_fit();
        accessors_.resize(idx1);
        accessors_.shrink_to_fit();
        free_.clear();
        free_.shrink_to_fit();
    }

private:
    InitializerType initializer_;
    CleanerType cleaner_;

    Vector<T> storage_;
    Vector<std::size_t> free_;
    Vector<Accessor *> accessors_;
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_UTIL_OBJ_POOL_HPP_
