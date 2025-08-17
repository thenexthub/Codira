//===-- flang/Parser/openmp-utils.h ---------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Common OpenMP utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_PARSER_OPENMP_UTILS_H
#define LANGUAGE_COMPABILITY_PARSER_OPENMP_UTILS_H

#include "language/Compability/Common/indirection.h"
#include "language/Compability/Parser/parse-tree.h"
#include "toolchain/Frontend/OpenMP/OMP.h"

#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace language::Compability::parser::omp {

namespace detail {
using D = toolchain::omp::Directive;

template <typename Construct> //
struct ConstructId {
  static constexpr toolchain::omp::Directive id{D::OMPD_unknown};
};

#define MAKE_CONSTR_ID(Construct, Id) \
  template <> struct ConstructId<Construct> { \
    static constexpr toolchain::omp::Directive id{Id}; \
  }

MAKE_CONSTR_ID(OmpAssumeDirective, D::OMPD_assume);
MAKE_CONSTR_ID(OmpCriticalDirective, D::OMPD_critical);
MAKE_CONSTR_ID(OmpDeclareVariantDirective, D::OMPD_declare_variant);
MAKE_CONSTR_ID(OmpErrorDirective, D::OMPD_error);
MAKE_CONSTR_ID(OmpMetadirectiveDirective, D::OMPD_metadirective);
MAKE_CONSTR_ID(OpenMPDeclarativeAllocate, D::OMPD_allocate);
MAKE_CONSTR_ID(OpenMPDeclarativeAssumes, D::OMPD_assumes);
MAKE_CONSTR_ID(OpenMPDeclareMapperConstruct, D::OMPD_declare_mapper);
MAKE_CONSTR_ID(OpenMPDeclareReductionConstruct, D::OMPD_declare_reduction);
MAKE_CONSTR_ID(OpenMPDeclareSimdConstruct, D::OMPD_declare_simd);
MAKE_CONSTR_ID(OpenMPDeclareTargetConstruct, D::OMPD_declare_target);
MAKE_CONSTR_ID(OpenMPExecutableAllocate, D::OMPD_allocate);
MAKE_CONSTR_ID(OpenMPRequiresConstruct, D::OMPD_requires);
MAKE_CONSTR_ID(OpenMPThreadprivate, D::OMPD_threadprivate);

#undef MAKE_CONSTR_ID

struct DirectiveNameScope {
  static OmpDirectiveName MakeName(CharBlock source = {},
      toolchain::omp::Directive id = toolchain::omp::Directive::OMPD_unknown) {
    OmpDirectiveName name;
    name.source = source;
    name.v = id;
    return name;
  }

  static OmpDirectiveName GetOmpDirectiveName(const OmpNothingDirective &x) {
    return MakeName(x.source, toolchain::omp::Directive::OMPD_nothing);
  }

  static OmpDirectiveName GetOmpDirectiveName(const OmpBeginLoopDirective &x) {
    auto &dir{std::get<OmpLoopDirective>(x.t)};
    return MakeName(dir.source, dir.v);
  }

  static OmpDirectiveName GetOmpDirectiveName(const OpenMPSectionConstruct &x) {
    if (auto &spec{std::get<std::optional<OmpDirectiveSpecification>>(x.t)}) {
      return spec->DirName();
    } else {
      return MakeName({}, toolchain::omp::Directive::OMPD_section);
    }
  }

  static OmpDirectiveName GetOmpDirectiveName(
      const OmpBeginSectionsDirective &x) {
    auto &dir{std::get<OmpSectionsDirective>(x.t)};
    return MakeName(dir.source, dir.v);
  }

  template <typename T>
  static OmpDirectiveName GetOmpDirectiveName(const T &x) {
    if constexpr (WrapperTrait<T>) {
      if constexpr (std::is_same_v<T, OpenMPCancelConstruct> ||
          std::is_same_v<T, OpenMPCancellationPointConstruct> ||
          std::is_same_v<T, OpenMPDepobjConstruct> ||
          std::is_same_v<T, OpenMPFlushConstruct> ||
          std::is_same_v<T, OpenMPInteropConstruct> ||
          std::is_same_v<T, OpenMPSimpleStandaloneConstruct>) {
        return x.v.DirName();
      } else {
        return GetOmpDirectiveName(x.v);
      }
    } else if constexpr (TupleTrait<T>) {
      if constexpr (std::is_base_of_v<OmpBlockConstruct, T>) {
        return std::get<OmpBeginDirective>(x.t).DirName();
      } else if constexpr (std::is_same_v<T, OmpAssumeDirective> ||
          std::is_same_v<T, OmpCriticalDirective> ||
          std::is_same_v<T, OmpDeclareVariantDirective> ||
          std::is_same_v<T, OmpErrorDirective> ||
          std::is_same_v<T, OmpMetadirectiveDirective> ||
          std::is_same_v<T, OpenMPDeclarativeAllocate> ||
          std::is_same_v<T, OpenMPDeclarativeAssumes> ||
          std::is_same_v<T, OpenMPDeclareMapperConstruct> ||
          std::is_same_v<T, OpenMPDeclareReductionConstruct> ||
          std::is_same_v<T, OpenMPDeclareSimdConstruct> ||
          std::is_same_v<T, OpenMPDeclareTargetConstruct> ||
          std::is_same_v<T, OpenMPExecutableAllocate> ||
          std::is_same_v<T, OpenMPRequiresConstruct> ||
          std::is_same_v<T, OpenMPThreadprivate>) {
        return MakeName(std::get<Verbatim>(x.t).source, ConstructId<T>::id);
      } else {
        return GetFromTuple(
            x.t, std::make_index_sequence<std::tuple_size_v<decltype(x.t)>>{});
      }
    } else if constexpr (UnionTrait<T>) {
      return common::visit(
          [](auto &&s) { return GetOmpDirectiveName(s); }, x.u);
    } else {
      return MakeName();
    }
  }

  template <typename... Ts, size_t... Is>
  static OmpDirectiveName GetFromTuple(
      const std::tuple<Ts...> &t, std::index_sequence<Is...>) {
    OmpDirectiveName name = MakeName();
    auto accumulate = [&](const OmpDirectiveName &n) {
      if (name.v == toolchain::omp::Directive::OMPD_unknown) {
        name = n;
      } else {
        assert(
            n.v == toolchain::omp::Directive::OMPD_unknown && "Conflicting names");
      }
    };
    (accumulate(GetOmpDirectiveName(std::get<Is>(t))), ...);
    return name;
  }

  template <typename T>
  static OmpDirectiveName GetOmpDirectiveName(const common::Indirection<T> &x) {
    return GetOmpDirectiveName(x.value());
  }
};
} // namespace detail

template <typename T> OmpDirectiveName GetOmpDirectiveName(const T &x) {
  return detail::DirectiveNameScope::GetOmpDirectiveName(x);
}

} // namespace language::Compability::parser::omp

#endif // FORTRAN_PARSER_OPENMP_UTILS_H
