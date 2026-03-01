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

#ifndef CODIRA_CGENUMTYPE_H
#define CODIRA_CGENUMTYPE_H

#include "Base/CGTypes/CGCustomType.h"

namespace Codira {
namespace CodeGen {

class CGEnumType : public CGCustomType {
    friend class CGTypeMgr;
    friend class CGEnum;
    friend class IRBuilder2;

public:
    enum class CGEnumTypeKind {
        UNKNOWN,        /**< Not a valid kind */

        /* Non-recursive and Non-exhaustive enum types */
        NON_EXHAUSTIVE_UNASSOCIATED, // all constructors have no associated values.
        NON_EXHAUSTIVE_ASSOCIATED,   // some constructors have associated values.

        /* Non-recursive and Exhaustive enum types */
        EXHAUSTIVE_ZERO_SIZE,    // has and only has one constructor, and the size of the associated values is 0.
        EXHAUSTIVE_UNASSOCIATED, // all constructors have no associated values.
        EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_REF, // has and only has two constructors, and only one constructor has only
                                               // one associated value of ref-type.
        EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_NONREF, // has and only has two constructors, and only one constructor has
                                                  // only one associated value of non-ref-type.
        EXHAUSTIVE_ASSOCIATED_NONREF, // The associated values of all constructors are non-ref-type.
        EXHAUSTIVE_OTHER,             // other cases.

        /* Unable to determine the category of T */
        EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_T,   // has and only has two constructors, and only one constructor has only
                                               // one associated value of T-type.
    };

    struct OptionLikeInfo {
        bool isAntiOptionLike{false};
        CHIR::Type* associatedValueType{nullptr};

        OptionLikeInfo(bool isAntiOptionLike, CHIR::Type* associatedValueType)
            : isAntiOptionLike(isAntiOptionLike), associatedValueType(associatedValueType)
        {
        }
    };

    bool IsTrivial() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::NON_EXHAUSTIVE_UNASSOCIATED ||
               cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_UNASSOCIATED;
    }
    bool IsCommonEnum() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::NON_EXHAUSTIVE_ASSOCIATED ||
               cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_OTHER;
    }
    bool IsZeroSizeEnum() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ZERO_SIZE;
    }
    bool IsOptionLikeT() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_T;
    }
    bool IsOptionLikeRef() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_REF;
    }
    bool IsOptionLikeNonRef() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_NONREF;
    }
    bool IsOptionLike() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_T ||
               cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_REF ||
               cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ASSOCIATED_OPTION_LIKE_NONREF;
    }
    bool IsAllAssociatedValuesAreNonRef() const
    {
        return cgEnumTypeKind == CGEnumTypeKind::EXHAUSTIVE_ASSOCIATED_NONREF;
    }
    bool IsAntiOptionLike() const
    {
        CODEC_ASSERT_WITH_MSG(optionLikeInfo != nullptr, "cgEnumType is not OptionLike");
        return optionLikeInfo->isAntiOptionLike;
    }
    CHIR::Type* GetAssociatedValueTypeOfOptionLike() const
    {
        CODEC_ASSERT_WITH_MSG(optionLikeInfo != nullptr, "cgEnumType is not OptionLike");
        return optionLikeInfo->associatedValueType;
    }
    bool PassByReference() const
    {
        return IsOptionLikeNonRef() || IsOptionLikeT() || IsZeroSizeEnum() || IsAllAssociatedValuesAreNonRef();
    }

    llvm::StructType* GetLayoutType() const
    {
        return layoutType;
    }

    CGEnumTypeKind GetCGEnumTypeKind() const
    {
        return cgEnumTypeKind;
    }

    std::size_t GetsizeOfConstructorUnion() const
    {
        CODEC_ASSERT_WITH_MSG(
            sizeOfConstructorUnion.has_value(), "This enum does not belong to EXHAUSTIVE_ASSOCIATED_NONREF.");
        return sizeOfConstructorUnion.value() * 8U; // 8U: bits
    }

    std::string GetEnumTypeName() const;

protected:
    llvm::Type* GenLLVMType() override;
    void GenContainedCGTypes() override;

private:
    CGEnumType() = delete;

    explicit CGEnumType(CGModule& cgMod, CGContext& cgCtx, const CHIR::Type& chirType);

    CGEnumTypeKind CalculateCGEnumTypeKind();

    void CalculateSizeAndAlign() override;

    llvm::Constant* GenFieldsNumOfTypeInfo() override;
    llvm::Constant* GenFieldsOfTypeInfo() override;
    llvm::Constant* GenOffsetsOfTypeInfo() override;

    llvm::Constant* GenFieldsNumOfTypeTemplate() override;
    llvm::Constant* GenFieldsFnsOfTypeTemplate() override;

    llvm::Type* GenZeroSizeEnumLLVMType();
    llvm::Type* GenTrivialLLVMType();
    llvm::Type* GenOptionLikeRefLLVMType();
    llvm::Type* GenOptionLikeNonRefLLVMType();
    llvm::Type* GenAssociatedNonRefEnumLLVMType();
    llvm::Type* GenCommonEnumLLVMType();

private:
    const CHIR::EnumType& chirEnumType;
    std::unique_ptr<OptionLikeInfo> optionLikeInfo;
    std::optional<std::size_t> sizeOfConstructorUnion;
    CGEnumTypeKind cgEnumTypeKind;
};
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_CGENUMTYPE_H
