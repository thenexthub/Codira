#ifndef CODIRA_RUNTIME_CPP_BINDINGS_H_
#define CODIRA_RUNTIME_CPP_BINDINGS_H_

#include <cassert>
#include <optional>
#include <string_view>

#include "codira/error.h"
#include "codira/function.h"
#include "codira/runtime_capi.h"
#include "codira/runtime_function.h"
#include "codira/type.h"

namespace codira {

struct RuntimeOptions;

/** A wrapper around a `CodiraRuntimeHandle`.
 *
 * Frees the corresponding runtime object on destruction, if it exists.
 */
class Runtime {
    friend std::optional<Runtime> make_runtime(std::string_view library_path,
                                               const RuntimeOptions& options,
                                               Error* out_error) noexcept;

    /** Constructs a runtime from an instantiated `CodiraRuntimeHandle`.
     *
     * \param handle a runtime type_handle
     */
    Runtime(CodiraRuntime handle) noexcept : m_handle(handle) {}

public:
    /** Move constructs a runtime
     *
     * \param other an rvalue reference to a runtime
     */
    Runtime(Runtime&& other) noexcept : m_handle(other.m_handle) { other.m_handle._0 = nullptr; }

    /** Destructs a runtime */
    ~Runtime() noexcept { codira_runtime_destroy(m_handle); }

    /** Retrieves `CodiraFunctionDefinition` from the runtime for the corresponding
     * `fn_name`.
     *
     * \param fn_name the name of the desired function
     * \param out_error a pointer that will optionally return an error
     * \return possibly, the desired `CodiraFunctionDefinition` struct
     */
    std::optional<Function> find_function_info(std::string_view fn_name,
                                               Error* out_error = nullptr) noexcept {
        bool has_fn;
        CodiraFunction fn_info;
        if (auto error = Error(codira_runtime_find_function_definition(
                m_handle, fn_name.data(), fn_name.size(), &has_fn, &fn_info));
            error.is_error()) {
            if (out_error) {
                *out_error = std::move(error);
            }
            return std::nullopt;
        }

        return has_fn ? std::make_optional(Function(fn_info)) : std::nullopt;
    }

    /**
     * Allocates an object in the runtime of the given `type_info`. If
     * successful, `obj` is returned, otherwise the nothing is returned and the
     * `out_error` is set - if it is not null.
     *
     * \param type_info the type to allocate
     * \param out_error a pointer to fill with a potential error
     * \return potentially, the type_handle of an allocated object
     */
    std::optional<CodiraGcPtr> gc_alloc(const Type& type_info,
                                     Error* out_error = nullptr) const noexcept {
        CodiraGcPtr obj;
        if (auto error = Error(codira_gc_alloc(m_handle, type_info.type_handle(), &obj));
            error.is_error()) {
            if (out_error) {
                *out_error = std::move(error);
            }
            return std::nullopt;
        }

        return std::make_optional(obj);
    }

    /** Collects all memory that is no longer referenced by rooted objects.
     *
     * Returns `true` if memory was reclaimed, `false` otherwise. This behavior
     * will likely change in the future.
     */
    bool gc_collect() const noexcept {
        bool reclaimed;
        CODIRA_ASSERT(codira_gc_collect(m_handle, &reclaimed));
        return reclaimed;
    }

    /**
     * Roots the specified `obj`, which keeps it and objects it references
     * alive.
     *
     * Objects marked as root, must call `codira_gc_unroot` before they can
     * be collected. An object can be rooted multiple times, but you must make
     * sure to call `codira_gc_unroot` an equal number of times before the object
     * can be collected. If successful, `obj` has been rooted, otherwise a
     * non-zero error type_handle is returned.
     *
     * \param obj a garbage collection type_handle
     */
    void gc_root_ptr(CodiraGcPtr obj) const noexcept { CODIRA_ASSERT(codira_gc_root(m_handle, obj)); }

    /**
     * Unroots the specified `obj`, potentially allowing it and objects it
     * references to be collected.
     *
     * An object can be rooted multiple times, so you must make sure to call
     * `gc_unroot_ptr` the same number of times as `gc_root_ptr` was called
     * before the object can be collected.
     *
     * \param obj a garbage collection type_handle
     */
    void gc_unroot_ptr(CodiraGcPtr obj) const noexcept { CODIRA_ASSERT(codira_gc_unroot(m_handle, obj)); }

    /**
     * Retrieves the type information for the specified `obj`.
     *
     * \param obj a garbage collection type_handle
     * \return the type_handle's type information
     */
    Type ptr_type(CodiraGcPtr obj) const noexcept {
        CodiraType type_handle;
        CODIRA_ASSERT(codira_gc_ptr_type(m_handle, obj, &type_handle));
        return Type(type_handle);
    }

    /** Checks for updates to hot reloadable assemblies.
     *
     * \param out_error a pointer that will optionally return an error
     * \return whether the runtime was updated
     */
    bool update(Error* out_error = nullptr) {
        bool updated;
        if (auto error = Error(codira_runtime_update(m_handle, &updated)); error.is_error()) {
            if (out_error) {
                *out_error = std::move(error);
            }
            return false;
        }
        return updated;
    }

private:
    CodiraRuntime m_handle;
};

struct RuntimeOptions {
    /**
     * The interval at which changes to the disk are detected. `0` will initialize this value to
     * default.
     */
    uint32_t delay_ms = 0;

    /**
     * A list of functions to add to the runtime, these functions can be called from Codira as *extern*
     * functions.
     */
    std::vector<RuntimeFunction> functions;
};

/** Construct a new runtime that loads the library at `library_path` and its dependencies.
 *
 * On failure, the error is returned through the `out_error` pointer, if set.
 *
 * \param library_path the path to a Codira library
 * \param options Additional options used by the construction of a runtime
 * \param out_error optionally, a pointer to an `Error` instance
 * \return potentially, a runtime
.*/
inline std::optional<Runtime> make_runtime(std::string_view library_path,
                                           const RuntimeOptions& options = {},
                                           Error* out_error = nullptr) noexcept {
    std::vector<CodiraExternalFunctionDefinition> function_definitions(options.functions.size());
    for (size_t i = 0; i < options.functions.size(); ++i) {
        auto& definition = function_definitions[i];
        const auto& func = options.functions[i];
        definition = CodiraExternalFunctionDefinition{
            func.name.c_str(), static_cast<uint32_t>(func.arg_types.size()), func.arg_types.data(),
            func.ret_type.type_handle(), func.fn_ptr};

        // The CodiraExternalFunctionDefinition has ownership over the stored types
        for (const auto& arg_type : func.arg_types) {
            CODIRA_ASSERT(codira_type_add_reference(arg_type));
        }
        CODIRA_ASSERT(codira_type_add_reference(func.ret_type.type_handle()));
    }

    CodiraRuntimeOptions runtime_options;
    runtime_options.functions =
        function_definitions.empty() ? nullptr : function_definitions.data();
    runtime_options.num_functions = static_cast<uint32_t>(function_definitions.size());

    CodiraRuntime handle;
    if (auto error = Error(codira_runtime_create(library_path.data(), runtime_options, &handle));
        error.is_error()) {
        if (out_error) {
            *out_error = std::move(error);
        }
        return std::nullopt;
    }

    return Runtime(handle);
}
}  // namespace codira

#endif /* CODIRA_RUNTIME_CPP_BINDINGS_H_ */
