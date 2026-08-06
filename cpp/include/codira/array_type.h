#ifndef CODIRA_ARRAY_TYPE_H
#define CODIRA_ARRAY_TYPE_H

#include <cassert>

#include "codira/error.h"
#include "codira/runtime_capi.h"
#include "codira/type.h"

namespace codira {
/**
 * @brief A wrapper around a Codira array type information handle.
 */
class ArrayType : public Type {
    /**
     * @brief Constructs struct information from a `CodiraStructInfo` and its associated Type.
     */
    constexpr ArrayType(CodiraType type_handle, CodiraArrayInfo array_info) noexcept
        : Type(type_handle), m_array_info(array_info) {}

public:
    /**
     * @brief Tries to cast the specified `Type` into a `StructType`.
     * Returns `std::nullopt` if the `Type` does not represent a struct.
     * \param ty The `Type` to cast
     * \return The StructType if the cast was successful.
     */
    static std::optional<ArrayType> try_cast(Type ty) {
        CodiraTypeKind kind;
        CODIRA_ASSERT(codira_type_kind(ty.type_handle(), &kind));
        if (kind.tag == CODIRA_TYPE_KIND_ARRAY) {
            return std::make_optional(ArrayType(std::move(ty).release_type_handle(), kind.array));
        } else {
            return std::nullopt;
        }
    }

    /**
     * @brief Returns the element type
     */
    [[nodiscard]] inline Type element_type() const noexcept {
        CodiraType ty;
        CODIRA_ASSERT(codira_array_type_element_type(m_array_info, &ty));
        return Type(ty);
    }

private:
    CodiraArrayInfo m_array_info;
};
}  // namespace codira

#endif  // CODIRA_ARRAY_TYPE_H
