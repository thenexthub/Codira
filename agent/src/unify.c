#include "unify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNIFY_BINDER_PATH_MAX 64

static void *unify_reallocarray(void *ptr, size_t count, size_t size) {
  if (size != 0 && count > ((size_t)-1) / size) abort();
  void *next = realloc(ptr, count * size);
  if (!next) abort();
  return next;
}

static char *unify_strdup(const char *text) {
  size_t len = strlen(text ? text : "");
  char *out = malloc(len + 1);
  if (!out) abort();
  memcpy(out, text ? text : "", len);
  out[len] = 0;
  return out;
}

static bool unify_fail(ZUnifyTrace *trace, const char *message) {
  if (trace && !trace->message[0]) snprintf(trace->message, sizeof(trace->message), "%s", message ? message : "types do not unify");
  return false;
}

static void unify_binding_free(ZUnifyBinding *binding) {
  if (!binding) return;
  free(binding->name);
  if (binding->kind == Z_UNIFY_BINDING_STATIC) z_static_value_free(&binding->static_value);
  *binding = (ZUnifyBinding){0};
}

static void unify_trace_truncate(ZUnifyTrace *trace, size_t len) {
  if (!trace || len > trace->len) return;
  for (size_t i = len; i < trace->len; i++) unify_binding_free(&trace->items[i]);
  trace->len = len;
}

void z_unify_trace_init(ZUnifyTrace *trace) {
  if (trace) *trace = (ZUnifyTrace){0};
}

void z_unify_trace_free(ZUnifyTrace *trace) {
  if (!trace) return;
  unify_trace_truncate(trace, 0);
  free(trace->items);
  *trace = (ZUnifyTrace){0};
}

const ZUnifyBinding *z_unify_trace_lookup(const ZUnifyTrace *trace, ZTypeBinderId binder, ZUnifyBindingKind kind) {
  if (!trace || binder == Z_TYPE_BINDER_ID_INVALID) return NULL;
  for (size_t i = 0; i < trace->len; i++) {
    if (trace->items[i].binder == binder && trace->items[i].kind == kind) return &trace->items[i];
  }
  return NULL;
}

static ZUnifyBinding *unify_trace_lookup_mut(ZUnifyTrace *trace, ZTypeBinderId binder, ZUnifyBindingKind kind) {
  if (!trace || binder == Z_TYPE_BINDER_ID_INVALID) return NULL;
  for (size_t i = 0; i < trace->len; i++) {
    if (trace->items[i].binder == binder && trace->items[i].kind == kind) return &trace->items[i];
  }
  return NULL;
}

static ZUnifyBinding *unify_trace_push(ZUnifyTrace *trace) {
  if (!trace) return NULL;
  if (trace->len + 1 > trace->cap) {
    size_t next = trace->cap ? trace->cap * 2 : 8;
    trace->items = unify_reallocarray(trace->items, next, sizeof(ZUnifyBinding));
    trace->cap = next;
  }
  trace->items[trace->len] = (ZUnifyBinding){0};
  return &trace->items[trace->len++];
}

static bool static_value_occurs(const ZStaticValue *value, ZTypeBinderId binder) {
  return value && value->kind == Z_STATIC_VALUE_BINDER && value->binder == binder;
}

static bool type_arg_occurs(const ZTypeArena *arena, const ZTypeArg *arg, ZTypeBinderId binder) {
  if (!arg) return false;
  if (arg->kind == Z_TYPE_ARG_STATIC) return static_value_occurs(&arg->as.static_value, binder);
  return z_type_occurs(arena, arg->as.type, binder);
}

bool z_type_occurs(const ZTypeArena *arena, ZTypeId type, ZTypeBinderId binder) {
  if (binder == Z_TYPE_BINDER_ID_INVALID) return false;
  ZTypeNodeKind kind = z_type_kind(arena, type);
  if (kind == Z_TYPE_NODE_BINDER) return z_type_binder(arena, type) == binder;
  if (kind == Z_TYPE_NODE_CONST) return z_type_occurs(arena, z_type_const_inner(arena, type), binder);
  if (kind == Z_TYPE_NODE_ARRAY) {
    return static_value_occurs(z_type_array_length(arena, type), binder) ||
           z_type_occurs(arena, z_type_array_element(arena, type), binder);
  }
  if (kind == Z_TYPE_NODE_APPLY) {
    for (size_t i = 0; i < z_type_apply_arg_len(arena, type); i++) {
      if (type_arg_occurs(arena, z_type_apply_arg(arena, type, i), binder)) return true;
    }
  }
  return false;
}

static bool binder_path_contains(const ZTypeBinderId *path, size_t len, ZTypeBinderId binder) {
  for (size_t i = 0; i < len; i++) {
    if (path[i] == binder) return true;
  }
  return false;
}

static bool binder_path_push(const ZTypeBinderId *path, size_t path_len, ZTypeBinderId binder, ZTypeBinderId out[UNIFY_BINDER_PATH_MAX], size_t *out_len) {
  if (path_len >= UNIFY_BINDER_PATH_MAX) return false;
  for (size_t i = 0; i < path_len; i++) out[i] = path[i];
  out[path_len] = binder;
  if (out_len) *out_len = path_len + 1;
  return true;
}

static bool binder_alias_resolves_to(const ZTypeArena *arena, const ZUnifyTrace *trace, ZTypeId type, ZTypeBinderId target, const ZTypeBinderId *path, size_t path_len) {
  if (target == Z_TYPE_BINDER_ID_INVALID || z_type_kind(arena, type) != Z_TYPE_NODE_BINDER) return false;
  ZTypeBinderId binder = z_type_binder(arena, type);
  if (binder == target) return true;
  if (binder_path_contains(path, path_len, binder)) return false;
  const ZUnifyBinding *binding = z_unify_trace_lookup(trace, binder, Z_UNIFY_BINDING_TYPE);
  if (!binding || z_type_kind(arena, binding->type) != Z_TYPE_NODE_BINDER) return false;
  ZTypeBinderId next_path[UNIFY_BINDER_PATH_MAX];
  size_t next_len = 0;
  if (!binder_path_push(path, path_len, binder, next_path, &next_len)) return false;
  return binder_alias_resolves_to(arena, trace, binding->type, target, next_path, next_len);
}

static bool type_binding_occurs_resolved(const ZTypeArena *arena, const ZUnifyTrace *trace, ZTypeId type, ZTypeBinderId binder, const ZTypeBinderId *path, size_t path_len) {
  if (binder == Z_TYPE_BINDER_ID_INVALID) return false;
  ZTypeNodeKind kind = z_type_kind(arena, type);
  if (kind == Z_TYPE_NODE_BINDER) {
    ZTypeBinderId found = z_type_binder(arena, type);
    if (found == binder) return true;
    if (binder_path_contains(path, path_len, found)) return false;
    const ZUnifyBinding *binding = z_unify_trace_lookup(trace, found, Z_UNIFY_BINDING_TYPE);
    if (!binding) return false;
    ZTypeBinderId next_path[UNIFY_BINDER_PATH_MAX];
    size_t next_len = 0;
    if (!binder_path_push(path, path_len, found, next_path, &next_len)) return true;
    return type_binding_occurs_resolved(arena, trace, binding->type, binder, next_path, next_len);
  }
  if (kind == Z_TYPE_NODE_CONST) return type_binding_occurs_resolved(arena, trace, z_type_const_inner(arena, type), binder, path, path_len);
  if (kind == Z_TYPE_NODE_ARRAY) return type_binding_occurs_resolved(arena, trace, z_type_array_element(arena, type), binder, path, path_len);
  if (kind == Z_TYPE_NODE_APPLY) {
    for (size_t i = 0; i < z_type_apply_arg_len(arena, type); i++) {
      const ZTypeArg *arg = z_type_apply_arg(arena, type, i);
      if (arg && arg->kind == Z_TYPE_ARG_TYPE && type_binding_occurs_resolved(arena, trace, arg->as.type, binder, path, path_len)) return true;
    }
  }
  return false;
}

static bool type_unify_inner(ZTypeArena *arena, ZTypeId pattern, ZTypeId actual, ZUnifyTrace *trace);

static bool bind_type(ZTypeArena *arena, ZUnifyTrace *trace, const char *name, ZTypeBinderId binder, ZTypeId type) {
  if (binder == Z_TYPE_BINDER_ID_INVALID || type == Z_TYPE_ID_INVALID) return unify_fail(trace, "invalid type binding");
  if (z_type_kind(arena, type) == Z_TYPE_NODE_BINDER && z_type_binder(arena, type) == binder) return true;
  ZUnifyBinding *existing = unify_trace_lookup_mut(trace, binder, Z_UNIFY_BINDING_TYPE);
  if (existing) return type_unify_inner(arena, existing->type, type, trace);
  if (binder_alias_resolves_to(arena, trace, type, binder, NULL, 0)) return true;
  if (type_binding_occurs_resolved(arena, trace, type, binder, NULL, 0)) return unify_fail(trace, "recursive type binding rejected by occurs check");
  ZUnifyBinding *binding = unify_trace_push(trace);
  if (!binding) return unify_fail(trace, "unification trace is required");
  binding->binder = binder;
  binding->kind = Z_UNIFY_BINDING_TYPE;
  binding->name = unify_strdup(name ? name : "");
  binding->type = type;
  return true;
}

static bool static_binder_alias_resolves_to(const ZUnifyTrace *trace, const ZStaticValue *value, ZTypeBinderId target, const ZTypeBinderId *path, size_t path_len) {
  if (target == Z_TYPE_BINDER_ID_INVALID || !value || value->kind != Z_STATIC_VALUE_BINDER) return false;
  if (value->binder == target) return true;
  if (binder_path_contains(path, path_len, value->binder)) return false;
  const ZUnifyBinding *binding = z_unify_trace_lookup(trace, value->binder, Z_UNIFY_BINDING_STATIC);
  if (!binding || binding->static_value.kind != Z_STATIC_VALUE_BINDER) return false;
  ZTypeBinderId next_path[UNIFY_BINDER_PATH_MAX];
  size_t next_len = 0;
  if (!binder_path_push(path, path_len, value->binder, next_path, &next_len)) return false;
  return static_binder_alias_resolves_to(trace, &binding->static_value, target, next_path, next_len);
}

static const char *static_type_or_default(const char *type) {
  return type && type[0] ? type : "usize";
}

static const char *static_value_binder_type(const ZStaticValue *value) {
  return static_type_or_default(value ? value->static_type : NULL);
}

static bool static_type_is_integer(const char *type) {
  type = static_type_or_default(type);
  return strcmp(type, "usize") == 0 || strcmp(type, "isize") == 0 ||
         strcmp(type, "u8") == 0 || strcmp(type, "u16") == 0 || strcmp(type, "u32") == 0 || strcmp(type, "u64") == 0 ||
         strcmp(type, "i8") == 0 || strcmp(type, "i16") == 0 || strcmp(type, "i32") == 0 || strcmp(type, "i64") == 0;
}

static bool static_type_compatible(const char *left, const char *right) {
  left = static_type_or_default(left);
  right = static_type_or_default(right);
  if (static_type_is_integer(left) && static_type_is_integer(right)) return true;
  return strcmp(left, right) == 0;
}

static bool static_symbol_matches_type(const ZStaticValue *value, const char *type) {
  if (!value || value->kind != Z_STATIC_VALUE_SYMBOL || !value->text) return false;
  type = static_type_or_default(type);
  size_t type_len = strlen(type);
  return strncmp(value->text, type, type_len) == 0 && value->text[type_len] == '.' && value->text[type_len + 1] != 0;
}

static bool static_value_matches_type(const ZStaticValue *value, const char *type) {
  if (!value) return false;
  type = static_type_or_default(type);
  if (value->kind == Z_STATIC_VALUE_BINDER) return static_type_compatible(type, static_value_binder_type(value));
  if (static_type_is_integer(type)) return value->kind == Z_STATIC_VALUE_NUMBER;
  if (strcmp(type, "Bool") == 0) return value->kind == Z_STATIC_VALUE_BOOL;
  return static_symbol_matches_type(value, type);
}

static bool static_value_unify(const ZStaticValue *pattern, const ZStaticValue *actual, ZUnifyTrace *trace);

static bool bind_static(ZUnifyTrace *trace, const char *name, ZTypeBinderId binder, const char *static_type, const ZStaticValue *value) {
  if (binder == Z_TYPE_BINDER_ID_INVALID || !value) return unify_fail(trace, "invalid static binding");
  if (value->kind == Z_STATIC_VALUE_BINDER && value->binder == binder) return true;
  if (!static_value_matches_type(value, static_type)) return unify_fail(trace, "static value does not match binder type");
  ZUnifyBinding *existing = unify_trace_lookup_mut(trace, binder, Z_UNIFY_BINDING_STATIC);
  if (existing) return static_value_unify(&existing->static_value, value, trace);
  if (static_binder_alias_resolves_to(trace, value, binder, NULL, 0)) return true;
  ZUnifyBinding *binding = unify_trace_push(trace);
  if (!binding) return unify_fail(trace, "unification trace is required");
  binding->binder = binder;
  binding->kind = Z_UNIFY_BINDING_STATIC;
  binding->name = unify_strdup(name ? name : "");
  if (!z_static_value_clone(value, &binding->static_value)) return unify_fail(trace, "could not copy static binding");
  return true;
}

static bool static_value_unify(const ZStaticValue *pattern, const ZStaticValue *actual, ZUnifyTrace *trace) {
  if (!pattern || !actual) return unify_fail(trace, "missing static value");
  if (pattern->kind == Z_STATIC_VALUE_BINDER) return bind_static(trace, pattern->text, pattern->binder, pattern->static_type, actual);
  if (actual->kind == Z_STATIC_VALUE_BINDER) return bind_static(trace, actual->text, actual->binder, actual->static_type, pattern);
  if (z_static_value_equal(pattern, actual)) return true;
  return unify_fail(trace, "static values do not unify");
}

static bool substitute_static_value_inner(const ZStaticValue *source, const ZUnifyTrace *trace, ZStaticValue *out, const ZTypeBinderId *path, size_t path_len) {
  if (!source || !out) return false;
  if (source->kind == Z_STATIC_VALUE_BINDER) {
    if (binder_path_contains(path, path_len, source->binder)) return false;
    const ZUnifyBinding *binding = z_unify_trace_lookup(trace, source->binder, Z_UNIFY_BINDING_STATIC);
    if (binding) {
      ZTypeBinderId next_path[UNIFY_BINDER_PATH_MAX];
      size_t next_len = 0;
      if (!binder_path_push(path, path_len, source->binder, next_path, &next_len)) return false;
      return substitute_static_value_inner(&binding->static_value, trace, out, next_path, next_len);
    }
  }
  return z_static_value_clone(source, out);
}

static bool substitute_static_value(const ZStaticValue *source, const ZUnifyTrace *trace, ZStaticValue *out) {
  return substitute_static_value_inner(source, trace, out, NULL, 0);
}

static void free_type_arg_copy(ZTypeArg *args, size_t len) {
  if (!args) return;
  for (size_t i = 0; i < len; i++) {
    if (args[i].kind == Z_TYPE_ARG_STATIC) z_static_value_free(&args[i].as.static_value);
  }
  free(args);
}

static bool type_substitute_inner(ZTypeArena *arena, ZTypeId source, const ZUnifyTrace *trace, ZTypeId *out, const ZTypeBinderId *path, size_t path_len) {
  if (out) *out = Z_TYPE_ID_INVALID;
  ZTypeNodeKind kind = z_type_kind(arena, source);
  if (kind == Z_TYPE_NODE_INVALID) return false;
  if (kind == Z_TYPE_NODE_BINDER) {
    ZTypeBinderId binder = z_type_binder(arena, source);
    if (binder_path_contains(path, path_len, binder)) return false;
    const ZUnifyBinding *binding = z_unify_trace_lookup(trace, binder, Z_UNIFY_BINDING_TYPE);
    if (binding) {
      ZTypeBinderId next_path[UNIFY_BINDER_PATH_MAX];
      size_t next_len = 0;
      if (!binder_path_push(path, path_len, binder, next_path, &next_len)) return false;
      return type_substitute_inner(arena, binding->type, trace, out, next_path, next_len);
    }
    ZTypeId clone = z_type_make_binder(arena, z_type_name(arena, source), binder);
    if (out) *out = clone;
    return clone != Z_TYPE_ID_INVALID;
  }
  if (kind == Z_TYPE_NODE_NAME) {
    ZTypeId clone = z_type_make_name(arena, z_type_name(arena, source));
    if (out) *out = clone;
    return clone != Z_TYPE_ID_INVALID;
  }
  if (kind == Z_TYPE_NODE_CONST) {
    ZTypeId inner = Z_TYPE_ID_INVALID;
    if (!type_substitute_inner(arena, z_type_const_inner(arena, source), trace, &inner, path, path_len)) return false;
    ZTypeId clone = z_type_make_const(arena, inner);
    if (out) *out = clone;
    return clone != Z_TYPE_ID_INVALID;
  }
  if (kind == Z_TYPE_NODE_ARRAY) {
    ZStaticValue length = {0};
    ZTypeId element = Z_TYPE_ID_INVALID;
    if (!substitute_static_value(z_type_array_length(arena, source), trace, &length)) return false;
    bool ok = type_substitute_inner(arena, z_type_array_element(arena, source), trace, &element, path, path_len);
    if (!ok) {
      z_static_value_free(&length);
      return false;
    }
    ZTypeId clone = z_type_make_array(arena, &length, element);
    z_static_value_free(&length);
    if (out) *out = clone;
    return clone != Z_TYPE_ID_INVALID;
  }
  if (kind == Z_TYPE_NODE_APPLY) {
    size_t arg_len = z_type_apply_arg_len(arena, source);
    ZTypeArg *args = arg_len ? unify_reallocarray(NULL, arg_len, sizeof(ZTypeArg)) : NULL;
    if (args) memset(args, 0, arg_len * sizeof(ZTypeArg));
    bool ok = true;
    for (size_t i = 0; i < arg_len && ok; i++) {
      const ZTypeArg *arg = z_type_apply_arg(arena, source, i);
      args[i] = *arg;
      if (arg->kind == Z_TYPE_ARG_TYPE) {
        args[i].as.type = Z_TYPE_ID_INVALID;
        ok = type_substitute_inner(arena, arg->as.type, trace, &args[i].as.type, path, path_len);
      } else {
        args[i].as.static_value = (ZStaticValue){0};
        ok = substitute_static_value(&arg->as.static_value, trace, &args[i].as.static_value);
      }
    }
    ZTypeId clone = ok ? z_type_make_apply(arena, z_type_name(arena, source), args, arg_len) : Z_TYPE_ID_INVALID;
    free_type_arg_copy(args, arg_len);
    if (out) *out = clone;
    return clone != Z_TYPE_ID_INVALID;
  }
  return false;
}

bool z_type_substitute(ZTypeArena *arena, ZTypeId source, const ZUnifyTrace *trace, ZTypeId *out) {
  if (out) *out = Z_TYPE_ID_INVALID;
  size_t start_len = arena ? arena->len : 0;
  ZTypeId substituted = Z_TYPE_ID_INVALID;
  if (type_substitute_inner(arena, source, trace, &substituted, NULL, 0)) {
    if (out) *out = substituted;
    return true;
  }
  z_type_arena_truncate(arena, start_len);
  return false;
}

static bool type_unify_inner(ZTypeArena *arena, ZTypeId pattern, ZTypeId actual, ZUnifyTrace *trace) {
  ZTypeNodeKind pattern_kind = z_type_kind(arena, pattern);
  ZTypeNodeKind actual_kind = z_type_kind(arena, actual);
  if (pattern_kind == Z_TYPE_NODE_INVALID || actual_kind == Z_TYPE_NODE_INVALID) return unify_fail(trace, "invalid type id");
  if (pattern_kind == Z_TYPE_NODE_BINDER) return bind_type(arena, trace, z_type_name(arena, pattern), z_type_binder(arena, pattern), actual);
  if (actual_kind == Z_TYPE_NODE_BINDER) return bind_type(arena, trace, z_type_name(arena, actual), z_type_binder(arena, actual), pattern);
  if (pattern_kind != actual_kind) return unify_fail(trace, "type shapes do not match");
  if (pattern_kind == Z_TYPE_NODE_NAME) {
    return strcmp(z_type_name(arena, pattern), z_type_name(arena, actual)) == 0 ? true : unify_fail(trace, "type names do not match");
  }
  if (pattern_kind == Z_TYPE_NODE_CONST) {
    return type_unify_inner(arena, z_type_const_inner(arena, pattern), z_type_const_inner(arena, actual), trace);
  }
  if (pattern_kind == Z_TYPE_NODE_ARRAY) {
    return static_value_unify(z_type_array_length(arena, pattern), z_type_array_length(arena, actual), trace) &&
           type_unify_inner(arena, z_type_array_element(arena, pattern), z_type_array_element(arena, actual), trace);
  }
  if (pattern_kind == Z_TYPE_NODE_APPLY) {
    if (strcmp(z_type_name(arena, pattern), z_type_name(arena, actual)) != 0 ||
        z_type_apply_arg_len(arena, pattern) != z_type_apply_arg_len(arena, actual)) {
      return unify_fail(trace, "generic type heads do not match");
    }
    for (size_t i = 0; i < z_type_apply_arg_len(arena, pattern); i++) {
      const ZTypeArg *pattern_arg = z_type_apply_arg(arena, pattern, i);
      const ZTypeArg *actual_arg = z_type_apply_arg(arena, actual, i);
      if (!pattern_arg || !actual_arg || pattern_arg->kind != actual_arg->kind) return unify_fail(trace, "generic type argument kinds do not match");
      if (pattern_arg->kind == Z_TYPE_ARG_TYPE) {
        if (!type_unify_inner(arena, pattern_arg->as.type, actual_arg->as.type, trace)) return false;
      } else if (!static_value_unify(&pattern_arg->as.static_value, &actual_arg->as.static_value, trace)) {
        return false;
      }
    }
    return true;
  }
  return unify_fail(trace, "unsupported type node");
}

bool z_type_unify(ZTypeArena *arena, ZTypeId pattern, ZTypeId actual, ZUnifyTrace *trace) {
  size_t start_len = trace ? trace->len : 0;
  if (trace) trace->message[0] = 0;

  bool ok = type_unify_inner(arena, pattern, actual, trace);
  if (ok) {
    if (trace) trace->message[0] = 0;
    return true;
  }

  if (trace) {
    char failure_message[sizeof(trace->message)];
    snprintf(failure_message, sizeof(failure_message), "%s", trace->message[0] ? trace->message : "types do not unify");
    unify_trace_truncate(trace, start_len);
    snprintf(trace->message, sizeof(trace->message), "%s", failure_message);
  }
  return false;
}
