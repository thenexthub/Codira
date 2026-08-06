#ifndef CODIRA_FUNCTION_H_
#define CODIRA_FUNCTION_H_

#include <string>
#include <vector>

#include "codira/runtime_capi.h"
#include "codira/static_type_info.h"
#include "codira/type.h"
#include "codira/util.h"

namespace codira {
/**
 * A wrapper around a C function with type information.
 */
struct RuntimeFunction {
    /**
     * Constructs a `RuntimeFunction` from a generic function pointer and a name.
     * \param name The name of the function used when added to the runtime
     * \param fn_ptr The function pointer to add
     */
    template <typename... TArgs>
    RuntimeFunction(std::string_view name, void(CODIRA_CALLTYPE* fn_ptr)(TArgs...))
        : name(name),
          arg_types(
              {Type(StaticTypeInfo<TArgs>::type_info().type_handle()).release_type_handle()...}),
          ret_type(StaticTypeInfo<std::tuple<>>::type_info().type_handle()),
          fn_ptr(reinterpret_cast<const void*>(fn_ptr)) {}

    /**
     * Constructs a `RuntimeFunction` from a generic function pointer and a name.
     * \param name The name of the function used when added to the runtime
     * \param fn_ptr The function pointer to add
     */
    template <typename TRet, typename... TArgs>
    RuntimeFunction(std::string_view name, TRet(CODIRA_CALLTYPE* fn_ptr)(TArgs...))
        : name(name),
          arg_types(
              {Type(StaticTypeInfo<TArgs>::type_info().type_handle()).release_type_handle()...}),
          ret_type(StaticTypeInfo<TRet>::type_info().type_handle()),
          fn_ptr(reinterpret_cast<const void*>(fn_ptr)) {}

    ~RuntimeFunction() {
        for (const auto& arg_type : arg_types) {
            CODIRA_ASSERT(codira_type_release(arg_type));
        }
    }

    RuntimeFunction(const RuntimeFunction& other)
        : name(other.name),
          arg_types(other.arg_types),
          ret_type(other.ret_type),
          fn_ptr(other.fn_ptr) {
        for (const auto& arg_type : arg_types) {
            CODIRA_ASSERT(codira_type_add_reference(arg_type));
        }
    }
    RuntimeFunction(RuntimeFunction&& other) = default;

    RuntimeFunction& operator=(const RuntimeFunction& other) {
        name = other.name;
        for (const auto& arg_type : other.arg_types) {
            CODIRA_ASSERT(codira_type_add_reference(arg_type));
        }
        for (const auto& arg_type : arg_types) {
            CODIRA_ASSERT(codira_type_release(arg_type));
        }
        arg_types = other.arg_types;
        ret_type = other.ret_type;
        fn_ptr = other.fn_ptr;
    };
    RuntimeFunction& operator=(RuntimeFunction&&) = default;

    std::string name;
    std::vector<CodiraType> arg_types;
    Type ret_type;
    const void* fn_ptr;
};
}  // namespace codira

#endif
