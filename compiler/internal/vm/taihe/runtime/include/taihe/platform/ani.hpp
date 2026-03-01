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
#ifndef RUNTIME_INCLUDE_TAIHE_PLATFORM_ANI_HPP_
#define RUNTIME_INCLUDE_TAIHE_PLATFORM_ANI_HPP_
// NOLINTBEGIN

#include <taihe/object.hpp>
#include <taihe/runtime.hpp>

#include <taihe.platform.ani.proj.hpp>

namespace taihe {
// convert between ani types and taihe types

template <typename cpp_owner_t>
struct from_ani_t;

template <typename cpp_owner_t>
struct into_ani_t;

template <typename cpp_owner_t>
constexpr inline from_ani_t<cpp_owner_t> from_ani;

template <typename cpp_owner_t>
constexpr inline into_ani_t<cpp_owner_t> into_ani;
}  // namespace taihe

namespace taihe {
// Reference management

class sref_guard {
protected:
    ani_ref ref = nullptr;

public:
    sref_guard(ani_env *env, ani_ref val)
    {
        env->GlobalReference_Create(val, &ref);
    }

    ~sref_guard() {}

    sref_guard(sref_guard const &) = delete;
    sref_guard &operator=(sref_guard const &) = delete;
    sref_guard(sref_guard &&) = delete;
    sref_guard &operator=(sref_guard &&) = delete;

    ani_ref get_ref()
    {
        return ref;
    }
};

class dref_guard : public sref_guard {
public:
    dref_guard(ani_env *env, ani_ref val) : sref_guard(env, val) {}

    ~dref_guard()
    {
        env_guard guard;
        ani_env *env = guard.get_env();
        env->GlobalReference_Delete(ref);
    }
};

template <typename AniRefGuard>
struct same_impl_t<AniRefGuard, std::enable_if_t<std::is_base_of_v<sref_guard, AniRefGuard>>> {
    bool operator()(data_view lhs, data_view rhs) const
    {
        auto lhs_as_ani = ::taihe::platform::ani::weak::AniObject(lhs);
        auto rhs_as_ani = ::taihe::platform::ani::weak::AniObject(rhs);
        if (lhs_as_ani.is_error() || rhs_as_ani.is_error()) {
            return same_impl<void>(lhs, rhs);
        }
        env_guard guard;
        ani_env *env = guard.get_env();
        ani_ref lhs_ref = reinterpret_cast<ani_ref>(lhs_as_ani->getGlobalReference());
        ani_ref rhs_ref = reinterpret_cast<ani_ref>(rhs_as_ani->getGlobalReference());
        ani_boolean result;
        return env->Reference_Equals(lhs_ref, rhs_ref, &result) == ANI_OK && result;
    }
};

template <typename AniRefGuard>
struct hash_impl_t<AniRefGuard, std::enable_if_t<std::is_base_of_v<sref_guard, AniRefGuard>>> {
    std::size_t operator()(data_view val) const
    {
        auto val_as_ani = ::taihe::platform::ani::weak::AniObject(val);
        if (val_as_ani.is_error()) {
            return hash_impl<void>(val);
        }
        TH_THROW(std::runtime_error, "Hashing of ani_ref is not implemented yet.");
    }
};
}  // namespace taihe

namespace taihe {
inline __attribute__((noinline)) ani_module ani_find_module(ani_env *env, char const *descriptor)
{
    ani_module mod;
    if (ANI_OK != env->FindModule(descriptor, &mod)) {
#ifdef DEBUG
        std::cerr << "Module not found: " << descriptor << std::endl;
#endif
        return nullptr;
    }
    return mod;
}

inline __attribute__((noinline)) ani_namespace ani_find_namespace(ani_env *env, char const *descriptor)
{
    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(descriptor, &ns)) {
#ifdef DEBUG
        std::cerr << "Namespace not found: " << descriptor << std::endl;
#endif
        return nullptr;
    }
    return ns;
}

inline __attribute__((noinline)) ani_class ani_find_class(ani_env *env, char const *descriptor)
{
    ani_class cls;
    if (ANI_OK != env->FindClass(descriptor, &cls)) {
#ifdef DEBUG
        std::cerr << "Class not found: " << descriptor << std::endl;
#endif
        return nullptr;
    }
    return cls;
}

inline __attribute__((noinline)) ani_enum ani_find_enum(ani_env *env, char const *descriptor)
{
    ani_enum enm;
    if (ANI_OK != env->FindEnum(descriptor, &enm)) {
#ifdef DEBUG
        std::cerr << "Enum not found: " << descriptor << std::endl;
#endif
        return nullptr;
    }
    return enm;
}

inline __attribute__((noinline)) ani_function ani_find_module_function(ani_env *env, ani_module mod, char const *name,
                                                                       char const *signature)
{
    ani_function fn;
    if (mod == nullptr) {
        return nullptr;
    }
    if (ANI_OK != env->Module_FindFunction(mod, name, signature, &fn)) {
#ifdef DEBUG
        std::cerr << "Function not found: " << name << " with signature: " << (signature ? signature : "<nullptr>")
                  << std::endl;
#endif
        return nullptr;
    }
    return fn;
}

inline __attribute__((noinline)) ani_function ani_find_namespace_function(ani_env *env, ani_namespace ns,
                                                                          char const *name, char const *signature)
{
    ani_function fn;
    if (ns == nullptr) {
        return nullptr;
    }
    if (ANI_OK != env->Namespace_FindFunction(ns, name, signature, &fn)) {
#ifdef DEBUG
        std::cerr << "Function not found: " << name << " with signature: " << (signature ? signature : "<nullptr>")
                  << std::endl;
#endif
        return nullptr;
    }
    return fn;
}

inline __attribute__((noinline)) ani_method ani_find_class_method(ani_env *env, ani_class cls, char const *name,
                                                                  char const *signature)
{
    ani_method mtd;
    if (cls == nullptr) {
        return nullptr;
    }
    if (ANI_OK != env->Class_FindMethod(cls, name, signature, &mtd)) {
#ifdef DEBUG
        std::cerr << "Method not found: " << name << " with signature: " << (signature ? signature : "<nullptr>")
                  << std::endl;
#endif
        return nullptr;
    }
    return mtd;
}

inline __attribute__((noinline)) ani_static_method ani_find_class_static_method(ani_env *env, ani_class cls,
                                                                                char const *name, char const *signature)
{
    ani_static_method mtd;
    if (cls == nullptr) {
        return nullptr;
    }
    if (ANI_OK != env->Class_FindStaticMethod(cls, name, signature, &mtd)) {
#ifdef DEBUG
        std::cerr << "Static method not found: " << name << " with signature: " << (signature ? signature : "<nullptr>")
                  << std::endl;
#endif
        return nullptr;
    }
    return mtd;
}

inline __attribute__((noinline)) ani_variable ani_find_module_variable(ani_env *env, ani_module mod, char const *name)
{
    ani_variable var;
    if (ANI_OK != env->Module_FindVariable(mod, name, &var)) {
#ifdef DEBUG
        std::cerr << "Variable not found: " << name << std::endl;
#endif
        return nullptr;
    }
    return var;
}

inline __attribute__((noinline)) ani_variable ani_find_namespace_variable(ani_env *env, ani_namespace ns,
                                                                          char const *name)
{
    ani_variable var;
    if (ns == nullptr) {
        return nullptr;
    }
    if (ANI_OK != env->Namespace_FindVariable(ns, name, &var)) {
#ifdef DEBUG
        std::cerr << "Variable not found: " << name << std::endl;
#endif
        return nullptr;
    }
    return var;
}

inline __attribute__((noinline)) ani_field ani_find_class_field(ani_env *env, ani_class cls, char const *name)
{
    ani_field fld;
    if (cls == nullptr) {
        return nullptr;
    }
    if (ANI_OK != env->Class_FindField(cls, name, &fld)) {
#ifdef DEBUG
        std::cerr << "Field not found: " << name << std::endl;
#endif
        return nullptr;
    }
    return fld;
}

inline __attribute__((noinline)) ani_static_field ani_find_class_static_field(ani_env *env, ani_class cls,
                                                                              char const *name)
{
    ani_static_field fld;
    if (cls == nullptr) {
        return nullptr;
    }
    if (ANI_OK != env->Class_FindStaticField(cls, name, &fld)) {
#ifdef DEBUG
        std::cerr << "Static field not found: " << name << std::endl;
#endif
        return nullptr;
    }
    return fld;
}
}  // namespace taihe

#if __cplusplus >= 202002L
namespace taihe {
template <std::size_t N = 0>
struct nullable_fixed_string {
    bool is_null;
    char value[N];

    constexpr nullable_fixed_string(std::nullptr_t) : is_null {true}, value {} {}

    constexpr nullable_fixed_string(char const (&sv)[N]) : is_null {false}
    {
        for (std::size_t i = 0; i < N; ++i) {
            value[i] = sv[i];
        }
    }

    constexpr char const *c_str() const
    {
        return is_null ? nullptr : value;
    }
};

template <nullable_fixed_string descriptor_t>
inline ani_module ani_cache_module(ani_env *env)
{
    static sref_guard guard(env, ani_find_module(env, descriptor_t.c_str()));
    return static_cast<ani_module>(guard.get_ref());
}

template <nullable_fixed_string descriptor_t>
inline ani_namespace ani_cache_namespace(ani_env *env)
{
    static sref_guard guard(env, ani_find_namespace(env, descriptor_t.c_str()));
    return static_cast<ani_namespace>(guard.get_ref());
}

template <nullable_fixed_string descriptor_t>
inline ani_class ani_cache_class(ani_env *env)
{
    static sref_guard guard(env, ani_find_class(env, descriptor_t.c_str()));
    return static_cast<ani_class>(guard.get_ref());
}

template <nullable_fixed_string descriptor_t>
inline ani_enum ani_cache_enum(ani_env *env)
{
    static sref_guard guard(env, ani_find_enum(env, descriptor_t.c_str()));
    return static_cast<ani_enum>(guard.get_ref());
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_function ani_cache_module_function(ani_env *env)
{
    static ani_function function =
        ani_find_module_function(env, ani_cache_module<descriptor_t>(env), name_t.c_str(), signature_t.c_str());
    return function;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_function ani_cache_namespace_function(ani_env *env)
{
    static ani_function function =
        ani_find_namespace_function(env, ani_cache_namespace<descriptor_t>(env), name_t.c_str(), signature_t.c_str());
    return function;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_method ani_cache_class_method(ani_env *env)
{
    static ani_method method =
        ani_find_class_method(env, ani_cache_class<descriptor_t>(env), name_t.c_str(), signature_t.c_str());
    return method;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_static_method ani_cache_class_static_method(ani_env *env)
{
    static ani_static_method method =
        ani_find_class_static_method(env, ani_cache_class<descriptor_t>(env), name_t.c_str(), signature_t.c_str());
    return method;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_variable ani_cache_module_variable(ani_env *env)
{
    static ani_variable variable = ani_find_module_variable(env, ani_cache_module<descriptor_t>(env), name_t.c_str());
    return variable;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_variable ani_cache_namespace_variable(ani_env *env)
{
    static ani_variable variable =
        ani_find_namespace_variable(env, ani_cache_namespace<descriptor_t>(env), name_t.c_str());
    return variable;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_field ani_cache_class_field(ani_env *env)
{
    static ani_field field = ani_find_class_field(env, ani_cache_class<descriptor_t>(env), name_t.c_str());
    return field;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_static_field ani_cache_class_static_field(ani_env *env)
{
    static ani_static_field field =
        ani_find_class_static_field(env, ani_cache_class<descriptor_t>(env), name_t.c_str());
    return field;
}
}  // namespace taihe

#define TH_ANI_FIND_MODULE(env, descriptor) ::taihe::ani_cache_module<descriptor>(env)
#define TH_ANI_FIND_NAMESPACE(env, descriptor) ::taihe::ani_cache_namespace<descriptor>(env)
#define TH_ANI_FIND_CLASS(env, descriptor) ::taihe::ani_cache_class<descriptor>(env)
#define TH_ANI_FIND_ENUM(env, descriptor) ::taihe::ani_cache_enum<descriptor>(env)

#define TH_ANI_FIND_MODULE_FUNCTION(env, descriptor, name, signature) \
    ::taihe::ani_cache_module_function<descriptor, name, signature>(env)
#define TH_ANI_FIND_NAMESPACE_FUNCTION(env, descriptor, name, signature) \
    ::taihe::ani_cache_namespace_function<descriptor, name, signature>(env)
#define TH_ANI_FIND_CLASS_METHOD(env, descriptor, name, signature) \
    ::taihe::ani_cache_class_method<descriptor, name, signature>(env)
#define TH_ANI_FIND_CLASS_STATIC_METHOD(env, descriptor, name, signature) \
    ::taihe::ani_cache_class_static_method<descriptor, name, signature>(env)

#define TH_ANI_FIND_MODULE_VARIABLE(env, descriptor, name) ::taihe::ani_cache_module_variable<descriptor, name>(env)
#define TH_ANI_FIND_NAMESPACE_VARIABLE(env, descriptor, name) \
    ::taihe::ani_cache_namespace_variable<descriptor, name>(env)
#define TH_ANI_FIND_CLASS_FIELD(env, descriptor, name) ::taihe::ani_cache_class_field<descriptor, name>(env)
#define TH_ANI_FIND_CLASS_STATIC_FIELD(env, descriptor, name) \
    ::taihe::ani_cache_class_static_field<descriptor, name>(env)
#else  // __cplusplus >= 202002L
#define TH_ANI_FIND_MODULE(penv, descriptor)                                                \
    ([env = (penv)] {                                                                       \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_module(env, descriptor)); \
        return static_cast<ani_module>(__guard.get_ref());                                  \
    }())

#define TH_ANI_FIND_NAMESPACE(penv, descriptor)                                                \
    ([env = (penv)] {                                                                          \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_namespace(env, descriptor)); \
        return static_cast<ani_namespace>(__guard.get_ref());                                  \
    }())

#define TH_ANI_FIND_CLASS(penv, descriptor)                                                \
    ([env = (penv)] {                                                                      \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_class(env, descriptor)); \
        return static_cast<ani_class>(__guard.get_ref());                                  \
    }())

#define TH_ANI_FIND_ENUM(penv, descriptor)                                                \
    ([env = (penv)] {                                                                     \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_enum(env, descriptor)); \
        return static_cast<ani_enum>(__guard.get_ref());                                  \
    }())

#define TH_ANI_FIND_MODULE_FUNCTION(penv, descriptor, name, signature)                                           \
    ([env = (penv)] {                                                                                            \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_module(env, descriptor));                      \
        static ani_function __function =                                                                         \
            ::taihe::ani_find_module_function(env, static_cast<ani_module>(__guard.get_ref()), name, signature); \
        return __function;                                                                                       \
    }())

#define TH_ANI_FIND_NAMESPACE_FUNCTION(penv, descriptor, name, signature)                                              \
    ([env = (penv)] {                                                                                                  \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_namespace(env, descriptor));                         \
        static ani_function __function =                                                                               \
            ::taihe::ani_find_namespace_function(env, static_cast<ani_namespace>(__guard.get_ref()), name, signature); \
        return __function;                                                                                             \
    }())

#define TH_ANI_FIND_CLASS_METHOD(penv, descriptor, name, signature)                                          \
    ([env = (penv)] {                                                                                        \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_class(env, descriptor));                   \
        static ani_method __method =                                                                         \
            ::taihe::ani_find_class_method(env, static_cast<ani_class>(__guard.get_ref()), name, signature); \
        return __method;                                                                                     \
    }())

#define TH_ANI_FIND_CLASS_STATIC_METHOD(penv, descriptor, name, signature)                                          \
    ([env = (penv)] {                                                                                               \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_class(env, descriptor));                          \
        static ani_static_method __method =                                                                         \
            ::taihe::ani_find_class_static_method(env, static_cast<ani_class>(__guard.get_ref()), name, signature); \
        return __method;                                                                                            \
    }())

#define TH_ANI_FIND_MODULE_VARIABLE(penv, descriptor, name)                                           \
    ([env = (penv)] {                                                                                 \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_module(env, descriptor));           \
        static ani_variable __variable =                                                              \
            ::taihe::ani_find_module_variable(env, static_cast<ani_module>(__guard.get_ref()), name); \
        return __variable;                                                                            \
    }())

#define TH_ANI_FIND_NAMESPACE_VARIABLE(penv, descriptor, name)                                              \
    ([env = (penv)] {                                                                                       \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_namespace(env, descriptor));              \
        static ani_variable __variable =                                                                    \
            ::taihe::ani_find_namespace_variable(env, static_cast<ani_namespace>(__guard.get_ref()), name); \
        return __variable;                                                                                  \
    }())

#define TH_ANI_FIND_CLASS_FIELD(penv, descriptor, name)                                          \
    ([env = (penv)] {                                                                            \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_class(env, descriptor));       \
        static ani_field __field =                                                               \
            ::taihe::ani_find_class_field(env, static_cast<ani_class>(__guard.get_ref()), name); \
        return __field;                                                                          \
    }())

#define TH_ANI_FIND_CLASS_STATIC_FIELD(penv, descriptor, name)                                          \
    ([env = (penv)] {                                                                                   \
        static ::taihe::sref_guard __guard(env, ::taihe::ani_find_class(env, descriptor));              \
        static ani_static_field __field =                                                               \
            ::taihe::ani_find_class_static_field(env, static_cast<ani_class>(__guard.get_ref()), name); \
        return __field;                                                                                 \
    }())
#endif  // __cplusplus >= 202002L
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_PLATFORM_ANI_HPP_