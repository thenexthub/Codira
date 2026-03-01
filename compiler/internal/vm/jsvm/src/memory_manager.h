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


#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <algorithm>

#include "jsvm_util.h"

template<size_t elementSize, size_t sizePerChunk = 8, size_t threshold = 10>
class MemoryChunkList {
private:
    union ElementMemory {
        constexpr ElementMemory() : next(nullptr) {}

        ~ElementMemory() {}

        ElementMemory* next;
        char ele[elementSize];
    };

    struct MemoryChunk;

    struct ElementContainer {
        uintptr_t header;
        ElementMemory memory;

        static MemoryChunk* GetMemoryChunk(const ElementMemory* mem)
        {
            uintptr_t headerPtr = reinterpret_cast<uintptr_t>(mem) - sizeof(header);
            return reinterpret_cast<MemoryChunk*>(*reinterpret_cast<uintptr_t*>(headerPtr));
        }
    };

    struct MemoryChunk {
        MemoryChunk* prev;
        MemoryChunk* next;
        size_t freeCount;
        ElementContainer elements[sizePerChunk];

        MemoryChunk() : prev(nullptr), next(nullptr), freeCount(sizePerChunk)
        {
            for (size_t i = 0; i < sizePerChunk - 1; ++i) {
                elements[i].header = reinterpret_cast<uintptr_t>(this);
                elements[i].memory.next = &(elements[i + 1].memory);
            }
            elements[sizePerChunk - 1].header = reinterpret_cast<uintptr_t>(this);
            elements[sizePerChunk - 1].memory.next = nullptr;
        }

        void Inc()
        {
            ++freeCount;
        }

        void Dec()
        {
            --freeCount;
        }

        bool CanBeFree()
        {
            return freeCount == sizePerChunk;
        }
    };

public:
    MemoryChunkList() : head(nullptr), freeList(nullptr)
    {
        AllocateChunk();
    }

    ~MemoryChunkList()
    {
        auto* ptr = head;
        while (ptr) {
            auto* next = ptr->next;
            if (UNLIKELY(!ptr->CanBeFree())) {
                LOG(Error) << "Memory is in use when free " << std::hex << reinterpret_cast<uintptr_t>(ptr);
                DCHECK(false && "MemoryChunk can not free");
            }

            delete ptr;
            ptr = next;
        }
    }

    template<typename Element, typename... Args>
    Element* New(Args&&... args)
    {
        static_assert(elementSize >= sizeof(Element));
        return new (GetMemory()) Element(std::forward<Args>(args)...);
    }

    template<typename Element>
    void Delete(Element* element)
    {
        element->~Element();
        auto* memory = reinterpret_cast<ElementMemory*>(element);
        auto* chunk = ElementContainer::GetMemoryChunk(memory);
        chunk->Inc();

        if (UNLIKELY(chunkNumber > threshold && chunk->CanBeFree())) {
            FreeChunk(chunk);
            return;
        }

        memory->next = freeList;
        freeList = memory;
    }

private:
    void* GetMemory()
    {
        if (UNLIKELY(!freeList)) {
            AllocateChunk();
        }

        auto* memory = freeList;
        ElementContainer::GetMemoryChunk(memory)->Dec();
        freeList = memory->next;

        return reinterpret_cast<void*>(memory);
    }

    void AllocateChunk()
    {
        DCHECK(freeList == nullptr);
        auto* newChunk = new MemoryChunk();
        if (head) {
            newChunk->next = head;
            head->prev = newChunk;
        }
        head = newChunk;
        ++chunkNumber;

        freeList = &(newChunk->elements[0].memory);
    }

    void FreeChunk(MemoryChunk* chunk)
    {
        // Rebuild freeList
        size_t count = 0;
        auto* newFreeList = freeList;
        ElementMemory* prev = nullptr;
        for (auto* current = freeList; count < sizePerChunk - 1 && current; current = current->next) {
            if (ElementContainer::GetMemoryChunk(current) == chunk) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    newFreeList = current->next;
                }
                ++count;
            } else {
                prev = current;
            }
        }
        DCHECK(count == sizePerChunk - 1);
        freeList = newFreeList;

        // Rebuild chunk list
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
        if (chunk->prev) {
            chunk->prev->next = chunk->next;
        } else {
            // chunk is head
            DCHECK(chunk == head);
            head = chunk->next;
        }
        --chunkNumber;

        // Delete chunk
        delete chunk;
    }

private:
    MemoryChunk* head;
    size_t chunkNumber;
    ElementMemory* freeList;
};

#endif