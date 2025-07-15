//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_C_CODE_COMPLETION_H
#define LANGUAGE_C_CODE_COMPLETION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// The version constants for the CodiraCodeCompletion C API.
/// LANGUAGEIDE_VERSION_MINOR should increase when there are API additions.
/// LANGUAGEIDE_VERSION_MAJOR is intended for "major" source/ABI breaking changes.
#define LANGUAGEIDE_VERSION_MAJOR 0
#define LANGUAGEIDE_VERSION_MINOR 1

#define LANGUAGEIDE_VERSION_ENCODE(major, minor) (((major)*10000) + ((minor)*1))

#define LANGUAGEIDE_VERSION                                                       \
  LANGUAGEIDE_VERSION_ENCODE(LANGUAGEIDE_VERSION_MAJOR, LANGUAGEIDE_VERSION_MINOR)

#define LANGUAGEIDE_VERSION_STRINGIZE_(major, minor) #major "." #minor
#define LANGUAGEIDE_VERSION_STRINGIZE(major, minor)                               \
  LANGUAGEIDE_VERSION_STRINGIZE_(major, minor)

#define LANGUAGEIDE_VERSION_STRING                                                \
  LANGUAGEIDE_VERSION_STRINGIZE(LANGUAGEIDE_VERSION_MAJOR, LANGUAGEIDE_VERSION_MINOR)

#ifdef __cplusplus
#define LANGUAGEIDE_BEGIN_DECLS extern "C" {
#define LANGUAGEIDE_END_DECLS }
#else
#define LANGUAGEIDE_BEGIN_DECLS
#define LANGUAGEIDE_END_DECLS
#endif

#if defined(_WIN32)
// NOTE: static builds are currently unsupported.
# if defined(sourcekitd_EXPORTS)
#   define LANGUAGEIDE_PUBLIC __declspec(dllexport)
# else
#   define LANGUAGEIDE_PUBLIC __declspec(dllimport)
# endif
#else
# define LANGUAGEIDE_PUBLIC /**/
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if !__has_feature(blocks)
#error -fblocks is a requirement to use this library
#endif

#if defined(__clang__) || defined(__GNUC__)
#define LANGUAGEIDE_DEPRECATED(m) __attribute__((deprecated(m)))
#endif

LANGUAGEIDE_BEGIN_DECLS

//=== Types ---------------------------------------------------------------===//

/// Global state across completions including compiler instance caching.
typedef void *languageide_connection_t;

/// Opaque completion item handle, used to retrieve additional information that
/// may be more expensive to compute.
typedef void *languageide_completion_item_t;

typedef enum languageide_completion_kind_t: uint32_t {
  LANGUAGEIDE_COMPLETION_KIND_NONE = 0,
  LANGUAGEIDE_COMPLETION_KIND_IMPORT = 1,
  LANGUAGEIDE_COMPLETION_KIND_UNRESOLVEDMEMBER = 2,
  LANGUAGEIDE_COMPLETION_KIND_DOTEXPR = 3,
  LANGUAGEIDE_COMPLETION_KIND_STMTOREXPR = 4,
  LANGUAGEIDE_COMPLETION_KIND_POSTFIXEXPRBEGINNING = 5,
  LANGUAGEIDE_COMPLETION_KIND_POSTFIXEXPR = 6,
  /* obsoleted */ LANGUAGEIDE_COMPLETION_KIND_POSTFIXEXPRPAREN = 7,
  LANGUAGEIDE_COMPLETION_KIND_KEYPATHEXPROBJC = 8,
  LANGUAGEIDE_COMPLETION_KIND_KEYPATHEXPRLANGUAGE = 9,
  LANGUAGEIDE_COMPLETION_KIND_TYPEDECLRESULTBEGINNING = 10,
  LANGUAGEIDE_COMPLETION_KIND_TYPESIMPLEBEGINNING = 11,
  LANGUAGEIDE_COMPLETION_KIND_TYPEIDENTIFIERWITHDOT = 12,
  LANGUAGEIDE_COMPLETION_KIND_TYPEIDENTIFIERWITHOUTDOT = 13,
  LANGUAGEIDE_COMPLETION_KIND_CASESTMTKEYWORD = 14,
  LANGUAGEIDE_COMPLETION_KIND_CASESTMTBEGINNING = 15,
  LANGUAGEIDE_COMPLETION_KIND_NOMINALMEMBERBEGINNING = 16,
  LANGUAGEIDE_COMPLETION_KIND_ACCESSORBEGINNING = 17,
  LANGUAGEIDE_COMPLETION_KIND_ATTRIBUTEBEGIN = 18,
  LANGUAGEIDE_COMPLETION_KIND_ATTRIBUTEDECLPAREN = 19,
  LANGUAGEIDE_COMPLETION_KIND_POUNDAVAILABLEPLATFORM = 20,
  LANGUAGEIDE_COMPLETION_KIND_CALLARG = 21,
  LANGUAGEIDE_COMPLETION_KIND_LABELEDTRAILINGCLOSURE = 22,
  LANGUAGEIDE_COMPLETION_KIND_RETURNSTMTEXPR = 23,
  LANGUAGEIDE_COMPLETION_KIND_YIELDSTMTEXPR = 24,
  LANGUAGEIDE_COMPLETION_KIND_FOREACHSEQUENCE = 25,
  LANGUAGEIDE_COMPLETION_KIND_AFTERPOUNDEXPR = 26,
  LANGUAGEIDE_COMPLETION_KIND_AFTERPOUNDDIRECTIVE = 27,
  LANGUAGEIDE_COMPLETION_KIND_PLATFORMCONDITON = 28,
  LANGUAGEIDE_COMPLETION_KIND_AFTERIFSTMTELSE = 29,
  LANGUAGEIDE_COMPLETION_KIND_GENERICREQUIREMENT = 30,
  LANGUAGEIDE_COMPLETION_KIND_PRECEDENCEGROUP = 31,
  LANGUAGEIDE_COMPLETION_KIND_STMTLABEL = 32,
  LANGUAGEIDE_COMPLETION_KIND_EFFECTSSPECIFIER = 33,
  LANGUAGEIDE_COMPLETION_KIND_FOREACHPATTERNBEGINNING = 34,
  LANGUAGEIDE_COMPLETION_KIND_TYPEATTRBEGINNING = 35,
  LANGUAGEIDE_COMPLETION_KIND_OPTIONALBINDING = 36,
  LANGUAGEIDE_COMPLETION_KIND_FOREACHKWIN = 37,
  /*obsoleted*/ LANGUAGEIDE_COMPLETION_KIND_WITHOUTCONSTRAINTTYPE = 38,
  LANGUAGEIDE_COMPLETION_KIND_THENSTMTEXPR = 39,
  LANGUAGEIDE_COMPLETION_KIND_TYPEBEGINNING = 40,
  LANGUAGEIDE_COMPLETION_KIND_TYPESIMPLEORCOMPOSITION = 41,
  LANGUAGEIDE_COMPLETION_KIND_TYPEPOSSIBLEFUNCTIONPARAMBEGINNING = 42,
  LANGUAGEIDE_COMPLETION_KIND_TYPEATTRINHERITANCEBEGINNING = 43,
  LANGUAGEIDE_COMPLETION_KIND_TYPESIMPLEINVERTED = 44,
  LANGUAGEIDE_COMPLETION_KIND_USING = 45,
} languageide_completion_kind_t;

typedef enum languageide_completion_item_kind_t: uint32_t {
  LANGUAGEIDE_COMPLETION_ITEM_KIND_DECLARATION = 0,
  LANGUAGEIDE_COMPLETION_ITEM_KIND_KEYWORD = 1,
  LANGUAGEIDE_COMPLETION_ITEM_KIND_PATTERN = 2,
  LANGUAGEIDE_COMPLETION_ITEM_KIND_LITERAL = 3,
  LANGUAGEIDE_COMPLETION_ITEM_KIND_BUILTINOPERATOR = 4,
} languageide_completion_item_kind_t;

typedef enum languageide_completion_item_decl_kind_t: uint32_t {
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_MODULE = 0,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_CLASS = 1,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_STRUCT = 2,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_ENUM = 3,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_ENUMELEMENT = 4,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_PROTOCOL = 5,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_ASSOCIATEDTYPE = 6,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_TYPEALIAS = 7,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_GENERICTYPEPARAM = 8,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_CONSTRUCTOR = 9,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_DESTRUCTOR = 10,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_SUBSCRIPT = 11,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_STATICMETHOD = 12,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_INSTANCEMETHOD = 13,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_PREFIXOPERATORFUNCTION = 14,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_POSTFIXOPERATORFUNCTION = 15,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_INFIXOPERATORFUNCTION = 16,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_FREEFUNCTION = 17,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_STATICVAR = 18,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_INSTANCEVAR = 19,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_LOCALVAR = 20,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_GLOBALVAR = 21,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_PRECEDENCEGROUP = 22,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_ACTOR = 23,
  LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_MACRO = 24,
} languageide_completion_item_decl_kind_t;

typedef enum languageide_completion_type_relation_t: uint32_t {
  LANGUAGEIDE_COMPLETION_TYPE_RELATION_NOTAPPLICABLE = 0,
  LANGUAGEIDE_COMPLETION_TYPE_RELATION_UNKNOWN = 1,
  LANGUAGEIDE_COMPLETION_TYPE_RELATION_UNRELATED = 2,
  LANGUAGEIDE_COMPLETION_TYPE_RELATION_INVALID = 3,
  LANGUAGEIDE_COMPLETION_TYPE_RELATION_CONVERTIBLE = 4,
  LANGUAGEIDE_COMPLETION_TYPE_RELATION_IDENTICAL = 5,
} languageide_completion_type_relation_t;

typedef enum languageide_completion_semantic_context_t: uint32_t {
  LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_NONE = 0,
  /* obsoleted */ LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_EXPRESSIONSPECIFIC = 1,
  LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_LOCAL = 2,
  LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_CURRENTNOMINAL = 3,
  LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_SUPER = 4,
  LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_OUTSIDENOMINAL = 5,
  LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_CURRENTMODULE = 6,
  LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_OTHERMODULE = 7,
} languageide_completion_semantic_context_t;

typedef enum languageide_completion_flair_t: uint32_t {
  LANGUAGEIDE_COMPLETION_FLAIR_EXPRESSIONSPECIFIC = 1 << 0,
  LANGUAGEIDE_COMPLETION_FLAIR_SUPERCHAIN = 1 << 1,
  LANGUAGEIDE_COMPLETION_FLAIR_ARGUMENTLABELS = 1 << 2,
  LANGUAGEIDE_COMPLETION_FLAIR_COMMONKEYWORDATCURRENTPOSITION = 1 << 3,
  LANGUAGEIDE_COMPLETION_FLAIR_RAREKEYWORDATCURRENTPOSITION = 1 << 4,
  LANGUAGEIDE_COMPLETION_FLAIR_RARETYPEATCURRENTPOSITION = 1 << 5,
  LANGUAGEIDE_COMPLETION_FLAIR_EXPRESSIONATNONSCRIPTORMAINFILESCOPE = 1 << 6,
} languageide_completion_flair_t;

typedef enum languageide_completion_not_recommended_reason_t: uint32_t {
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_NONE = 0,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_REDUNDANT_IMPORT = 1,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_DEPRECATED = 2,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_INVALID_ASYNC_CONTEXT = 3,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_CROSS_ACTOR_REFERENCE = 4,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_VARIABLE_USED_IN_OWN_DEFINITION = 5,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_REDUNDANT_IMPORT_INDIRECT = 6,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_SOFTDEPRECATED = 7,
  LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_NON_ASYNC_ALTERNATIVE_USED_IN_ASYNC_CONTEXT =
      8,
} languageide_completion_not_recommended_reason_t;

typedef enum languageide_completion_diagnostic_severity_t: uint32_t {
  LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_NONE = 0,
  LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_ERROR = 1,
  LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_WARNING = 2,
  LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_REMARK = 3,
  LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_NOTE = 4,
} languageide_completion_diagnostic_severity_t;

typedef void *languageide_completion_request_t;

typedef void *languageide_completion_response_t;

typedef void *languageide_fuzzy_match_pattern_t;

typedef void *languageide_cache_invalidation_options_t;

/// languageide equivalent of sourcekitd_request_handle_t
typedef const void *languageide_request_handle_t;

//=== Functions -----------------------------------------------------------===//

LANGUAGEIDE_DEPRECATED(
    "Use languageide_connection_create_with_inspection_instance instead")
LANGUAGEIDE_PUBLIC languageide_connection_t languageide_connection_create(void);

LANGUAGEIDE_PUBLIC languageide_connection_t
languageide_connection_create_with_inspection_instance(
    void *opqueCodiraIDEInspectionInstance);

LANGUAGEIDE_PUBLIC void languageide_connection_dispose(languageide_connection_t);

LANGUAGEIDE_PUBLIC void
    languageide_connection_mark_cached_compiler_instance_should_be_invalidated(
        languageide_connection_t, languageide_cache_invalidation_options_t);

/// Override the contents of the file \p path with \p contents. If \p contents
/// is NULL, go back to using the real the file system.
LANGUAGEIDE_PUBLIC void
languageide_set_file_contents(languageide_connection_t connection, const char *path,
                           const char *contents);

/// Cancel the request with \p handle.
LANGUAGEIDE_PUBLIC void languageide_cancel_request(languageide_connection_t _conn,
                                             languageide_request_handle_t handle);

LANGUAGEIDE_PUBLIC languageide_completion_request_t
languageide_completion_request_create(const char *path, uint32_t offset,
                                   char *const *const compiler_args,
                                   uint32_t num_compiler_args);

LANGUAGEIDE_PUBLIC void
    languageide_completion_request_dispose(languageide_completion_request_t);

LANGUAGEIDE_PUBLIC void
languageide_completion_request_set_annotate_result(languageide_completion_request_t,
                                                bool);

LANGUAGEIDE_PUBLIC void languageide_completion_request_set_include_objectliterals(
    languageide_completion_request_t, bool);

LANGUAGEIDE_PUBLIC void languageide_completion_request_set_add_inits_to_top_level(
    languageide_completion_request_t, bool);

LANGUAGEIDE_PUBLIC void
languageide_completion_request_set_add_call_with_no_default_args(
    languageide_completion_request_t, bool);

/// Same as languageide_complete but supports cancellation.
/// This request is identified by \p handle. Calling languageide_cancel_request
/// with that handle cancels the request.
/// Note that the caller is responsible for creating a unique request handle.
/// This differs from the sourcekitd functions in which SourceKit creates a
/// unique handle and passes it to the client via an out parameter.
LANGUAGEIDE_PUBLIC languageide_completion_response_t languageide_complete_cancellable(
    languageide_connection_t _conn, languageide_completion_request_t _req,
    languageide_request_handle_t handle);

LANGUAGEIDE_DEPRECATED("Use languageide_complete_cancellable instead")
LANGUAGEIDE_PUBLIC languageide_completion_response_t languageide_complete(
    languageide_connection_t provider, languageide_completion_request_t);

LANGUAGEIDE_PUBLIC void
    languageide_completion_result_dispose(languageide_completion_response_t);

LANGUAGEIDE_PUBLIC bool
    languageide_completion_result_is_error(languageide_completion_response_t);

/// Result has the same lifetime as the result.
LANGUAGEIDE_PUBLIC const char *languageide_completion_result_get_error_description(
    languageide_completion_response_t);

LANGUAGEIDE_PUBLIC bool
    languageide_completion_result_is_cancelled(languageide_completion_response_t);

/// Copies a string representation of the completion result. This string should
/// be disposed of with \c free when done.
LANGUAGEIDE_PUBLIC const char *
    languageide_completion_result_description_copy(languageide_completion_response_t);

LANGUAGEIDE_PUBLIC void languageide_completion_result_get_completions(
    languageide_completion_response_t,
    void (^completions_handler)(const languageide_completion_item_t *completions,
                                const char **filter_names,
                                uint64_t num_completions));

LANGUAGEIDE_PUBLIC languageide_completion_item_t
languageide_completion_result_get_completion_at_index(
    languageide_completion_response_t, uint64_t index);

LANGUAGEIDE_PUBLIC languageide_completion_kind_t
    languageide_completion_result_get_kind(languageide_completion_response_t);

LANGUAGEIDE_PUBLIC void languageide_completion_result_foreach_baseexpr_typename(
    languageide_completion_response_t, bool (^handler)(const char *));

LANGUAGEIDE_PUBLIC bool languageide_completion_result_is_reusing_astcontext(
    languageide_completion_response_t);

/// Copies a string representation of the completion item. This string should
/// be disposed of with \c free when done.
LANGUAGEIDE_PUBLIC const char *
    languageide_completion_item_description_copy(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC void
languageide_completion_item_get_label(languageide_completion_response_t,
                                   languageide_completion_item_t, bool annotate,
                                   void (^handler)(const char *));

LANGUAGEIDE_PUBLIC void
languageide_completion_item_get_source_text(languageide_completion_response_t,
                                         languageide_completion_item_t,
                                         void (^handler)(const char *));

LANGUAGEIDE_PUBLIC void languageide_completion_item_get_type_name(
    languageide_completion_response_t, languageide_completion_item_t, bool annotate,
    void (^handler)(const char *));

LANGUAGEIDE_PUBLIC void
languageide_completion_item_get_doc_brief(languageide_completion_response_t,
                                       languageide_completion_item_t,
                                       void (^handler)(const char *));

LANGUAGEIDE_PUBLIC void languageide_completion_item_get_associated_usrs(
    languageide_completion_response_t, languageide_completion_item_t,
    void (^handler)(const char **, uint64_t));

LANGUAGEIDE_PUBLIC
    uint32_t languageide_completion_item_get_kind(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC uint32_t
    languageide_completion_item_get_associated_kind(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC uint32_t
    languageide_completion_item_get_semantic_context(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC
    uint32_t languageide_completion_item_get_flair(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC bool
    languageide_completion_item_is_not_recommended(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC uint32_t
    languageide_completion_item_not_recommended_reason(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC bool
languageide_completion_item_has_diagnostic(languageide_completion_item_t _item);

LANGUAGEIDE_PUBLIC void languageide_completion_item_get_diagnostic(
    languageide_completion_response_t, languageide_completion_item_t,
    void (^handler)(languageide_completion_diagnostic_severity_t, const char *));

LANGUAGEIDE_PUBLIC bool
    languageide_completion_item_is_system(languageide_completion_item_t);

/// Call \p handler with the name of the module the code completion item
/// \p _item is defined in. The module may be \c nullptr for e.g. keywords.
LANGUAGEIDE_PUBLIC
void languageide_completion_item_get_module_name(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    void (^handler)(const char *));

LANGUAGEIDE_PUBLIC uint32_t
    languageide_completion_item_get_num_bytes_to_erase(languageide_completion_item_t);

LANGUAGEIDE_PUBLIC uint32_t
    languageide_completion_item_get_type_relation(languageide_completion_item_t);

/// Returns 0 for items not in an external module, and ~0u if the other module
/// is not imported or the depth is otherwise unknown.
LANGUAGEIDE_PUBLIC uint32_t languageide_completion_item_import_depth(
    languageide_completion_response_t, languageide_completion_item_t);

LANGUAGEIDE_PUBLIC languageide_fuzzy_match_pattern_t
languageide_fuzzy_match_pattern_create(const char *pattern);

LANGUAGEIDE_PUBLIC bool languageide_fuzzy_match_pattern_matches_candidate(
    languageide_fuzzy_match_pattern_t pattern, const char *candidate,
    double *outScore);

LANGUAGEIDE_PUBLIC void
    languageide_fuzzy_match_pattern_dispose(languageide_fuzzy_match_pattern_t);

LANGUAGEIDE_END_DECLS

#endif
