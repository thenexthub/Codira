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

#ifndef BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_LATTICE_ELEMENT_H
#define BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_LATTICE_ELEMENT_H

#include <variant>
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "compiler/optimizer/pass.h"
#include "constant_value.h"
#include "utils/hash.h"

namespace panda::bytecodeopt {

class ConstantElement;
class LatticeElement {
public:
    enum class LatticeType {
        LATTICE_TOP,
        LATTICE_CONSTANT,
        LATTICE_BOTTOM,
    };

    explicit LatticeElement(LatticeType type);
    virtual ~LatticeElement() = default;
    NO_MOVE_SEMANTIC(LatticeElement);
    NO_COPY_SEMANTIC(LatticeElement);

    bool IsConstantElement()
    {
        return type_ == LatticeType::LATTICE_CONSTANT;
    }

    bool IsTopElement()
    {
        return type_ == LatticeType::LATTICE_TOP;
    }

    bool IsBottomElement()
    {
        return type_ == LatticeType::LATTICE_BOTTOM;
    }

    virtual ConstantElement *AsConstant()
    {
        return nullptr;
    }

    virtual LatticeElement *Meet(LatticeElement *other) = 0;
    virtual std::string ToString() = 0;

private:
    LatticeType type_;
};

class TopElement : public LatticeElement {
public:
    NO_MOVE_SEMANTIC(TopElement);
    NO_COPY_SEMANTIC(TopElement);
    static LatticeElement *GetInstance();
    ~TopElement() override = default;
    LatticeElement *Meet(LatticeElement *other) override;
    std::string ToString() override;

protected:
    TopElement();
};

class BottomElement : public LatticeElement {
public:
    NO_MOVE_SEMANTIC(BottomElement);
    NO_COPY_SEMANTIC(BottomElement);
    ~BottomElement() override = default;
    LatticeElement *Meet(LatticeElement *other) override;
    std::string ToString() override;
    static LatticeElement *GetInstance();

protected:
    BottomElement();
};

class ConstantElement : public LatticeElement {
public:
    NO_MOVE_SEMANTIC(ConstantElement);
    NO_COPY_SEMANTIC(ConstantElement);
    explicit ConstantElement(bool val);
    explicit ConstantElement(int32_t val);
    explicit ConstantElement(int64_t val);
    explicit ConstantElement(double val);
    explicit ConstantElement(std::string val);
    explicit ConstantElement(const ConstantValue &val);
    ~ConstantElement() override = default;
    LatticeElement *Meet(LatticeElement *other) override;
    std::string ToString() override;
    ConstantElement *AsConstant() override;

    template<class T>
    T GetValue() const
    {
        return value_.GetValue<T>();
    }

    auto &GetValue() const
    {
        return value_.GetValue();
    }

    ConstantValue::ConstantType GetType() const
    {
        return value_.GetType();
    }

private:
    ConstantValue value_;
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_LATTICE_ELEMENT_H

