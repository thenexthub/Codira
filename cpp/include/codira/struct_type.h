#ifndef CODIRA_STRUCT_INFO_H
#define CODIRA_STRUCT_INFO_H

#include <cassert>

#include "codira/field_info.h"
#include "codira/runtime_capi.h"

namespace codira {
/**
 * @brief A wrapper around a Codira struct information handle.
 */
class StructType : public Type {
    /**
     * @brief Constructs struct information from a `CodiraStructInfo` and its associated Type.
     */
    constexpr StructType(CodiraType type_handle, CodiraStructInfo struct_info) noexcept
        : Type(type_handle), m_struct_info(struct_info) {}

public:
    /**
     * @brief Tries to cast the specified `Type` into a `StructType`.
     * Returns `std::nullopt` if the `Type` does not represent a struct.
     * \param ty The `Type` to cast
     * \return The StructType if the cast was successful.
     */
    static std::optional<StructType> try_cast(Type ty) {
        CodiraTypeKind kind;
        CODIRA_ASSERT(codira_type_kind(ty.type_handle(), &kind));
        if (kind.tag == CODIRA_TYPE_KIND_STRUCT) {
            return std::make_optional(
                StructType(std::move(ty).release_type_handle(), kind.struct_));
        } else {
            return std::nullopt;
        }
    }

    /**
     * @brief Returns the struct's fields.
     */
    StructFields fields() const noexcept {
        CodiraFields fields;
        CODIRA_ASSERT(codira_struct_type_fields(m_struct_info, &fields));
        return StructFields(fields);
    }

    /**
     * @brief Returns the struct's memory kind.
     */
    [[nodiscard]] CodiraStructMemoryKind memory_kind() const noexcept {
        CodiraStructMemoryKind memory_kind;
        CODIRA_ASSERT(codira_struct_type_memory_kind(m_struct_info, &memory_kind));
        return memory_kind;
    }

private:
    CodiraStructInfo m_struct_info;
};
}  // namespace codira

#endif  // CODIRA_STRUCT_INFO_H
