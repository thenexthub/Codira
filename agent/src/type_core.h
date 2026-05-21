#ifndef AGENT_TYPE_CORE_H
#define AGENT_TYPE_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t ZTypeId;
typedef uint32_t ZTypeBinderId;

#define Z_TYPE_ID_INVALID ((ZTypeId)0)
#define Z_TYPE_BINDER_ID_INVALID ((ZTypeBinderId)0)

typedef enum {
  Z_TYPE_BINDER_TYPE,
  Z_TYPE_BINDER_STATIC
} ZTypeBinderKind;

typedef enum {
  Z_TYPE_ARG_TYPE,
  Z_TYPE_ARG_STATIC
} ZTypeArgKind;

typedef struct {
  const char *name;
  ZTypeBinderKind kind;
  ZTypeBinderId id;
  const char *static_type;
} ZTypeBinderDecl;

typedef bool (*ZTypeArgKindCallback)(const void *context, const char *type_name, size_t arg_index, ZTypeArgKind *out_kind);

typedef struct {
  const ZTypeBinderDecl *items;
  size_t len;
  ZTypeArgKindCallback arg_kind;
  const void *arg_kind_context;
} ZTypeBinderScope;

typedef enum {
  Z_STATIC_VALUE_INVALID,
  Z_STATIC_VALUE_NUMBER,
  Z_STATIC_VALUE_BOOL,
  Z_STATIC_VALUE_SYMBOL,
  Z_STATIC_VALUE_BINDER
} ZStaticValueKind;

typedef struct {
  ZStaticValueKind kind;
  char *text;
  char *static_type;
  ZTypeBinderId binder;
  unsigned long long number;
  bool boolean;
} ZStaticValue;

typedef enum {
  Z_TYPE_NODE_INVALID,
  Z_TYPE_NODE_NAME,
  Z_TYPE_NODE_BINDER,
  Z_TYPE_NODE_CONST,
  Z_TYPE_NODE_ARRAY,
  Z_TYPE_NODE_APPLY
} ZTypeNodeKind;

typedef struct {
  ZTypeArgKind kind;
  union {
    ZTypeId type;
    ZStaticValue static_value;
  } as;
} ZTypeArg;

typedef struct ZTypeNode ZTypeNode;

typedef struct {
  ZTypeNode *nodes;
  size_t len;
  size_t cap;
} ZTypeArena;

typedef struct {
  size_t offset;
  char message[128];
} ZTypeParseError;

void z_type_arena_init(ZTypeArena *arena);
void z_type_arena_free(ZTypeArena *arena);
void z_type_arena_truncate(ZTypeArena *arena, size_t len);

bool z_static_value_parse(const char *text, ZStaticValue *out, ZTypeParseError *error);
bool z_static_value_parse_with_binders(const char *text, const ZTypeBinderScope *scope, ZStaticValue *out, ZTypeParseError *error);
char *z_static_value_format(const ZStaticValue *value);
bool z_static_value_clone(const ZStaticValue *source, ZStaticValue *out);
bool z_static_value_equal(const ZStaticValue *left, const ZStaticValue *right);
void z_static_value_free(ZStaticValue *value);

bool z_type_parse(ZTypeArena *arena, const char *text, ZTypeId *out, ZTypeParseError *error);
bool z_type_parse_with_binders(ZTypeArena *arena, const char *text, const ZTypeBinderScope *scope, ZTypeId *out, ZTypeParseError *error);
char *z_type_format(const ZTypeArena *arena, ZTypeId type);
bool z_type_equal(const ZTypeArena *arena, ZTypeId left, ZTypeId right);
uint64_t z_type_hash(const ZTypeArena *arena, ZTypeId type);

ZTypeId z_type_make_name(ZTypeArena *arena, const char *name);
ZTypeId z_type_make_binder(ZTypeArena *arena, const char *name, ZTypeBinderId binder);
ZTypeId z_type_make_const(ZTypeArena *arena, ZTypeId inner);
ZTypeId z_type_make_array(ZTypeArena *arena, const ZStaticValue *length, ZTypeId element);
ZTypeId z_type_make_apply(ZTypeArena *arena, const char *name, const ZTypeArg *args, size_t arg_len);

ZTypeNodeKind z_type_kind(const ZTypeArena *arena, ZTypeId type);
const char *z_type_name(const ZTypeArena *arena, ZTypeId type);
ZTypeBinderId z_type_binder(const ZTypeArena *arena, ZTypeId type);
ZTypeId z_type_const_inner(const ZTypeArena *arena, ZTypeId type);
const ZStaticValue *z_type_array_length(const ZTypeArena *arena, ZTypeId type);
ZTypeId z_type_array_element(const ZTypeArena *arena, ZTypeId type);
size_t z_type_apply_arg_len(const ZTypeArena *arena, ZTypeId type);
const ZTypeArg *z_type_apply_arg(const ZTypeArena *arena, ZTypeId type, size_t index);

#endif
