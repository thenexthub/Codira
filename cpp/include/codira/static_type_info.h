#ifndef CODIRA_STATIC_TYPE_INFO_H_
#define CODIRA_STATIC_TYPE_INFO_H_

#include "codira/runtime_capi.h"
#include "codira/type.h"

namespace codira {

template <typename T>
struct StaticTypeInfo;

#define IMPL_PRIMITIVE_TYPE_INFO(ty, codira_primitive_type)                          \
    template <>                                                                   \
    struct StaticTypeInfo<ty> {                                                   \
        static const Type& type_info() {                                          \
            static Type TYPE_INFO = Type(codira_type_primitive(codira_primitive_type)); \
            return TYPE_INFO;                                                     \
        }                                                                         \
    };

IMPL_PRIMITIVE_TYPE_INFO(bool, CODIRA_PRIMITIVE_TYPE_BOOL);
IMPL_PRIMITIVE_TYPE_INFO(float, CODIRA_PRIMITIVE_TYPE_F32);
IMPL_PRIMITIVE_TYPE_INFO(double, CODIRA_PRIMITIVE_TYPE_F64);
IMPL_PRIMITIVE_TYPE_INFO(int8_t, CODIRA_PRIMITIVE_TYPE_I8);
IMPL_PRIMITIVE_TYPE_INFO(int16_t, CODIRA_PRIMITIVE_TYPE_I16);
IMPL_PRIMITIVE_TYPE_INFO(int32_t, CODIRA_PRIMITIVE_TYPE_I32);
IMPL_PRIMITIVE_TYPE_INFO(int64_t, CODIRA_PRIMITIVE_TYPE_I64);
// IMPL_PRIMITIVE_TYPE_REFLECTION(int128_t, MunPrimitiveType_I128);
IMPL_PRIMITIVE_TYPE_INFO(uint8_t, CODIRA_PRIMITIVE_TYPE_U8);
IMPL_PRIMITIVE_TYPE_INFO(uint16_t, CODIRA_PRIMITIVE_TYPE_U16);
IMPL_PRIMITIVE_TYPE_INFO(uint32_t, CODIRA_PRIMITIVE_TYPE_U32);
IMPL_PRIMITIVE_TYPE_INFO(uint64_t, CODIRA_PRIMITIVE_TYPE_U64);
// IMPL_PRIMITIVE_TYPE_REFLECTION(uint128_t, MunPrimitiveType_U128);
IMPL_PRIMITIVE_TYPE_INFO(void, CODIRA_PRIMITIVE_TYPE_VOID);
IMPL_PRIMITIVE_TYPE_INFO(std::tuple<>, CODIRA_PRIMITIVE_TYPE_EMPTY);

}  // namespace codira

#endif  // CODIRA_STATIC_TYPE_INFO_H_
