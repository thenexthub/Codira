/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_EVENTS_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_EVENTS_H

#include "runtime/include/runtime.h"
#include "runtime/mem/refstorage/reference.h"

namespace ark {

using EventId = int32_t;

class CoroutineManager;

/**
 * @brief The base class for coroutine events. Cannot be instantiated directly.
 *
 * These events are used to implement blocking and unblocking the coroutines that are waiting for something.
 * The lifetime of an event is intended to be managed manually or be bound to its owner lifetime (e.g. coroutine).
 */
class CAPABILITY("mutex") CoroutineEvent {
public:
    NO_COPY_SEMANTIC(CoroutineEvent);
    NO_MOVE_SEMANTIC(CoroutineEvent);

    enum class Type { NONE, GENERIC, COMPLETION, CHANNEL, IO, TIMER };

    virtual ~CoroutineEvent()
    {
        ASSERT(!IsLocked());
    };

    Type GetType()
    {
        return type_;
    }

    bool Happened() REQUIRES(this)
    {
        return happened_;
    }

    /// Set event happened and unblock waiters
    void Happen();

    void SetNotHappened()
    {
        Lock();
        happened_ = false;
        Unlock();
    }

    void Lock() ACQUIRE()
    {
        mutex_.Lock();
        locked_ = true;
    }

    void Unlock() RELEASE()
    {
        locked_ = false;
        mutex_.Unlock();
    }

    EventId GetId() const
    {
        return eventId_;
    }

protected:
    static constexpr EventId DEFAULT_EVENT_ID = 0;

    explicit CoroutineEvent(Type t, CoroutineManager *coroManager, EventId id = DEFAULT_EVENT_ID)
        : type_(t), coroManager_(coroManager), eventId_(id)
    {
    }

    bool IsLocked()
    {
        return locked_;
    }

    void SetHappened()
    {
        Lock();
        happened_ = true;
        Unlock();
    }

private:
    Type type_ = Type::NONE;
    bool happened_ GUARDED_BY(this) = false;
    bool locked_ = false;
    CoroutineManager *coroManager_ = nullptr;
    EventId eventId_;

    os::memory::RecursiveMutex mutex_;
};

/**
 * @brief The generic event: just some  event that can be awaited.
 *
 * The only thing that it can do: it can happen.
 */
class GenericEvent : public CoroutineEvent {
public:
    NO_COPY_SEMANTIC(GenericEvent);
    NO_MOVE_SEMANTIC(GenericEvent);

    explicit GenericEvent(CoroutineManager *coroManager) : CoroutineEvent(Type::GENERIC, coroManager) {}
    ~GenericEvent() override = default;

private:
};

/// @brief The coroutine completion event: happens when coroutine is done executing its bytecode.
class CompletionEvent : public CoroutineEvent {
public:
    NO_COPY_SEMANTIC(CompletionEvent);
    NO_MOVE_SEMANTIC(CompletionEvent);

    /**
     * @param returnValueObject A weak reference (from global storage) to the language-dependent language object that
     * will hold the coroutine return value.
     */
    explicit CompletionEvent(mem::Reference *returnValueObject, CoroutineManager *coroManager)
        : CoroutineEvent(Type::COMPLETION, coroManager), returnValueObject_(returnValueObject)
    {
    }
    ~CompletionEvent() override = default;

    mem::Reference *ReleaseReturnValueObject()
    {
        return std::exchange(returnValueObject_, nullptr);
    }

    void SetReturnValueObject(mem::Reference *returnValueObject)
    {
        ASSERT(returnValueObject_ == nullptr);
        returnValueObject_ = returnValueObject;
    }

private:
    mem::Reference *returnValueObject_ = nullptr;
};

class TimerEvent : public CoroutineEvent {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    explicit TimerEvent(CoroutineManager *coroManager, EventId id) : CoroutineEvent(Type::TIMER, coroManager, id) {}

    NO_COPY_SEMANTIC(TimerEvent);
    NO_MOVE_SEMANTIC(TimerEvent);

    bool IsExpired() const
    {
        return expirationTime_ <= currentTime_;
    }

    void SetExpired()
    {
        currentTime_ = std::numeric_limits<uint64_t>::max();
    }

    void SetExpirationTime(uint64_t expirationTime)
    {
        expirationTime_ = expirationTime;
    }

    void SetCurrentTime(uint64_t currentTime)
    {
        currentTime_ = currentTime;
    }

    uint64_t GetDelay() const
    {
        return expirationTime_ - currentTime_;
    }

private:
    /// the time is represented in microseconds
    std::atomic<uint64_t> currentTime_ = 0;
    uint64_t expirationTime_ = 0;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_EVENTS_H */