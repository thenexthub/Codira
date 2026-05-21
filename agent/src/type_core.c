#include "type_core.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ZTypeNode {
  ZTypeNodeKind kind;
  union {
    char *name;
    struct {
      char *name;
      ZTypeBinderId binder;
    } binder;
    struct {
      ZTypeId inner;
    } constant;
    struct {
      ZStaticValue length;
      ZTypeId element;
    } array;
    struct {
      char *name;
      ZTypeArg *args;
      size_t arg_len;
    } apply;
  } as;
};

typedef struct {
  char *data;
  size_t len;
  size_t cap;
} TypeBuf;

typedef struct {
  const char *text;
  size_t cursor;
  const ZTypeBinderScope *scope;
  ZTypeParseError *error;
} TypeParser;

typedef struct {
  size_t cursor;
  size_t arena_len;
  ZTypeParseError error;
} TypeParserMark;

static void *type_reallocarray(void *ptr, size_t count, size_t size) {
  if (size != 0 && count > SIZE_MAX / size) abort();
  void *next = realloc(ptr, count * size);
  if (!next) abort();
  return next;
}

static char *type_strndup(const char *text, size_t len) {
  char *out = malloc(len + 1);
  if (!out) abort();
  memcpy(out, text, len);
  out[len] = 0;
  return out;
}

static char *type_strdup(const char *text) {
  return type_strndup(text ? text : "", strlen(text ? text : ""));
}

static void type_buf_append_len(TypeBuf *buf, const char *text, size_t len) {
  if (buf->len + len + 1 > buf->cap) {
    size_t next = buf->cap ? buf->cap : 32;
    while (buf->len + len + 1 > next) next *= 2;
    buf->data = type_reallocarray(buf->data, next, 1);
    buf->cap = next;
  }
  memcpy(buf->data + buf->len, text, len);
  buf->len += len;
  buf->data[buf->len] = 0;
}

static void type_buf_append(TypeBuf *buf, const char *text) {
  type_buf_append_len(buf, text, strlen(text));
}

static void type_buf_append_char(TypeBuf *buf, char ch) {
  type_buf_append_len(buf, &ch, 1);
}

static const ZTypeBinderDecl *type_binder_lookup(const ZTypeBinderScope *scope, const char *name, ZTypeBinderKind kind) {
  if (!scope || !name) return NULL;
  for (size_t i = 0; i < scope->len; i++) {
    const ZTypeBinderDecl *decl = &scope->items[i];
    if (decl->kind == kind && decl->id != Z_TYPE_BINDER_ID_INVALID && decl->name && strcmp(decl->name, name) == 0) return decl;
  }
  return NULL;
}

static bool static_value_from_binder(const ZTypeBinderDecl *decl, ZStaticValue *out) {
  if (!decl || decl->kind != Z_TYPE_BINDER_STATIC || decl->id == Z_TYPE_BINDER_ID_INVALID || !out) return false;
  *out = (ZStaticValue){
      .kind = Z_STATIC_VALUE_BINDER,
      .text = type_strdup(decl->name ? decl->name : ""),
      .static_type = type_strdup(decl->static_type && decl->static_type[0] ? decl->static_type : "usize"),
      .binder = decl->id};
  return true;
}

bool z_static_value_clone(const ZStaticValue *source, ZStaticValue *out) {
  if (!source || !out) return false;
  *out = *source;
  out->text = type_strdup(source->text ? source->text : "");
  out->static_type = source->static_type ? type_strdup(source->static_type) : NULL;
  return true;
}

void z_static_value_free(ZStaticValue *value) {
  if (!value) return;
  free(value->text);
  free(value->static_type);
  *value = (ZStaticValue){0};
}

static void type_arg_free(ZTypeArg *arg) {
  if (!arg) return;
  if (arg->kind == Z_TYPE_ARG_STATIC) z_static_value_free(&arg->as.static_value);
  *arg = (ZTypeArg){0};
}

static void type_node_free(ZTypeNode *node) {
  if (!node) return;
  if (node->kind == Z_TYPE_NODE_NAME) {
    free(node->as.name);
  } else if (node->kind == Z_TYPE_NODE_BINDER) {
    free(node->as.binder.name);
  } else if (node->kind == Z_TYPE_NODE_ARRAY) {
    z_static_value_free(&node->as.array.length);
  } else if (node->kind == Z_TYPE_NODE_APPLY) {
    free(node->as.apply.name);
    for (size_t i = 0; i < node->as.apply.arg_len; i++) type_arg_free(&node->as.apply.args[i]);
    free(node->as.apply.args);
  }
  *node = (ZTypeNode){0};
}

void z_type_arena_init(ZTypeArena *arena) {
  if (arena) *arena = (ZTypeArena){0};
}

void z_type_arena_free(ZTypeArena *arena) {
  if (!arena) return;
  for (size_t i = 0; i < arena->len; i++) type_node_free(&arena->nodes[i]);
  free(arena->nodes);
  *arena = (ZTypeArena){0};
}

static ZTypeNode *type_node(const ZTypeArena *arena, ZTypeId id) {
  if (!arena || id == Z_TYPE_ID_INVALID || (size_t)id > arena->len) return NULL;
  return &arena->nodes[(size_t)id - 1];
}

static ZTypeId type_arena_add(ZTypeArena *arena, ZTypeNode node) {
  if (!arena) return Z_TYPE_ID_INVALID;
  if (arena->len + 1 > arena->cap) {
    size_t next = arena->cap ? arena->cap * 2 : 32;
    arena->nodes = type_reallocarray(arena->nodes, next, sizeof(ZTypeNode));
    arena->cap = next;
  }
  arena->nodes[arena->len] = node;
  arena->len++;
  return (ZTypeId)arena->len;
}

ZTypeId z_type_make_name(ZTypeArena *arena, const char *name) {
  return type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_NAME, .as.name = type_strdup(name ? name : "Unknown")});
}

ZTypeId z_type_make_binder(ZTypeArena *arena, const char *name, ZTypeBinderId binder) {
  if (binder == Z_TYPE_BINDER_ID_INVALID) return Z_TYPE_ID_INVALID;
  return type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_BINDER, .as.binder = {.name = type_strdup(name ? name : ""), .binder = binder}});
}

ZTypeId z_type_make_const(ZTypeArena *arena, ZTypeId inner) {
  if (inner == Z_TYPE_ID_INVALID) return Z_TYPE_ID_INVALID;
  return type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_CONST, .as.constant = {.inner = inner}});
}

ZTypeId z_type_make_array(ZTypeArena *arena, const ZStaticValue *length, ZTypeId element) {
  if (element == Z_TYPE_ID_INVALID) return Z_TYPE_ID_INVALID;
  ZStaticValue length_copy = {0};
  if (!z_static_value_clone(length, &length_copy)) return Z_TYPE_ID_INVALID;
  return type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_ARRAY, .as.array = {.length = length_copy, .element = element}});
}

ZTypeId z_type_make_apply(ZTypeArena *arena, const char *name, const ZTypeArg *args, size_t arg_len) {
  if (!name || !name[0] || (arg_len > 0 && !args)) return Z_TYPE_ID_INVALID;
  ZTypeArg *arg_copy = NULL;
  if (arg_len > 0) {
    arg_copy = type_reallocarray(NULL, arg_len, sizeof(ZTypeArg));
    for (size_t i = 0; i < arg_len; i++) {
      arg_copy[i] = args[i];
      if (args[i].kind == Z_TYPE_ARG_STATIC && !z_static_value_clone(&args[i].as.static_value, &arg_copy[i].as.static_value)) {
        for (size_t j = 0; j < i; j++) type_arg_free(&arg_copy[j]);
        free(arg_copy);
        return Z_TYPE_ID_INVALID;
      }
    }
  }
  return type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_APPLY, .as.apply = {.name = type_strdup(name), .args = arg_copy, .arg_len = arg_len}});
}

void z_type_arena_truncate(ZTypeArena *arena, size_t len) {
  if (!arena || len > arena->len) return;
  for (size_t i = len; i < arena->len; i++) type_node_free(&arena->nodes[i]);
  arena->len = len;
}

static void parser_error(TypeParser *parser, const char *message) {
  if (!parser || !parser->error || parser->error->message[0]) return;
  parser->error->offset = parser->cursor;
  snprintf(parser->error->message, sizeof(parser->error->message), "%s", message ? message : "invalid type syntax");
}

static void parser_skip_ws(TypeParser *parser) {
  while (isspace((unsigned char)parser->text[parser->cursor])) parser->cursor++;
}

static bool ident_start(char ch) {
  return isalpha((unsigned char)ch) || ch == '_';
}

static bool ident_continue(char ch) {
  return isalnum((unsigned char)ch) || ch == '_';
}

static bool parser_keyword(TypeParser *parser, const char *keyword) {
  size_t len = strlen(keyword);
  return strncmp(parser->text + parser->cursor, keyword, len) == 0 &&
         !ident_continue(parser->text[parser->cursor + len]);
}

static char *parser_ident(TypeParser *parser) {
  parser_skip_ws(parser);
  size_t start = parser->cursor;
  if (!ident_start(parser->text[parser->cursor])) {
    parser_error(parser, "expected type name");
    return NULL;
  }
  parser->cursor++;
  while (ident_continue(parser->text[parser->cursor])) parser->cursor++;
  return type_strndup(parser->text + start, parser->cursor - start);
}

static TypeParserMark parser_mark(const TypeParser *parser, const ZTypeArena *arena) {
  return (TypeParserMark){
      .cursor = parser ? parser->cursor : 0,
      .arena_len = arena ? arena->len : 0,
      .error = parser && parser->error ? *parser->error : (ZTypeParseError){0},
  };
}

static void parser_restore(TypeParser *parser, ZTypeArena *arena, TypeParserMark mark) {
  if (parser) {
    parser->cursor = mark.cursor;
    if (parser->error) *parser->error = mark.error;
  }
  z_type_arena_truncate(arena, mark.arena_len);
}

static bool static_symbol_text(const char *text) {
  if (!text || !ident_start(text[0])) return false;
  size_t i = 0;
  bool saw_segment_char = false;
  while (text[i]) {
    if (ident_continue(text[i])) {
      saw_segment_char = true;
      i++;
      continue;
    }
    if (text[i] == '.' && saw_segment_char && ident_start(text[i + 1])) {
      saw_segment_char = false;
      i++;
      continue;
    }
    return false;
  }
  return saw_segment_char;
}

static char *trimmed_copy(const char *text, size_t len) {
  while (len > 0 && isspace((unsigned char)*text)) {
    text++;
    len--;
  }
  while (len > 0 && isspace((unsigned char)text[len - 1])) len--;
  return type_strndup(text, len);
}

static bool integer_suffix_text(const char *text) {
  static const char *suffixes[] = {
    "i8", "i16", "i32", "i64", "isize",
    "u8", "u16", "u32", "u64", "usize",
    NULL,
  };
  for (size_t i = 0; suffixes[i]; i++) {
    if (strcmp(text, suffixes[i]) == 0) return true;
  }
  return false;
}

static bool parse_number_text(const char *text, unsigned long long *out) {
  if (!text || !text[0]) return false;
  size_t body_len = strlen(text);
  const char *last_underscore = strrchr(text, '_');
  if (last_underscore) {
    const char *candidate = last_underscore + 1;
    if (candidate[0] == 0) return false;
    if (integer_suffix_text(candidate)) {
      body_len = (size_t)(last_underscore - text);
    } else if (candidate[0] == 'i' || candidate[0] == 'u') {
      return false;
    }
  }
  if (body_len == 0) return false;
  unsigned base = 10;
  size_t index = 0;
  if (body_len > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
    base = 16;
    index = 2;
  } else if (body_len > 2 && text[0] == '0' && (text[1] == 'b' || text[1] == 'B')) {
    base = 2;
    index = 2;
  } else if (body_len > 2 && text[0] == '0' && (text[1] == 'o' || text[1] == 'O')) {
    base = 8;
    index = 2;
  }
  unsigned long long value = 0;
  bool saw_digit = false;
  bool previous_underscore = false;
  for (; index < body_len; index++) {
    char ch = text[index];
    if (ch == '_') {
      if (!saw_digit || previous_underscore) return false;
      previous_underscore = true;
      continue;
    }
    int digit = -1;
    if (ch >= '0' && ch <= '9') digit = ch - '0';
    else if (ch >= 'a' && ch <= 'f') digit = 10 + ch - 'a';
    else if (ch >= 'A' && ch <= 'F') digit = 10 + ch - 'A';
    if (digit < 0 || (unsigned)digit >= base) return false;
    if (value > (ULLONG_MAX - (unsigned)digit) / base) return false;
    value = value * base + (unsigned)digit;
    saw_digit = true;
    previous_underscore = false;
  }
  if (!saw_digit || previous_underscore) return false;
  if (out) *out = value;
  return true;
}

static bool static_value_parse_inner(const char *text, const ZTypeBinderScope *scope, ZStaticValue *out, ZTypeParseError *error) {
  if (out) *out = (ZStaticValue){0};
  if (error) *error = (ZTypeParseError){0};
  char *trimmed = trimmed_copy(text ? text : "", strlen(text ? text : ""));
  if (!trimmed[0]) {
    if (error) snprintf(error->message, sizeof(error->message), "%s", "expected static value");
    free(trimmed);
    return false;
  }
  unsigned long long number = 0;
  if (parse_number_text(trimmed, &number)) {
    if (out) {
      char canonical[32];
      snprintf(canonical, sizeof(canonical), "%llu", number);
      *out = (ZStaticValue){.kind = Z_STATIC_VALUE_NUMBER, .text = type_strdup(canonical), .number = number};
    }
    free(trimmed);
    return true;
  }
  if (strcmp(trimmed, "true") == 0 || strcmp(trimmed, "false") == 0) {
    if (out) *out = (ZStaticValue){.kind = Z_STATIC_VALUE_BOOL, .text = type_strdup(trimmed), .boolean = strcmp(trimmed, "true") == 0};
    free(trimmed);
    return true;
  }
  if (static_symbol_text(trimmed)) {
    const ZTypeBinderDecl *binder = type_binder_lookup(scope, trimmed, Z_TYPE_BINDER_STATIC);
    if (binder) {
      if (out) static_value_from_binder(binder, out);
    } else if (out) *out = (ZStaticValue){.kind = Z_STATIC_VALUE_SYMBOL, .text = type_strdup(trimmed)};
    free(trimmed);
    return true;
  }
  if (error) snprintf(error->message, sizeof(error->message), "%s", "invalid static value");
  free(trimmed);
  return false;
}

bool z_static_value_parse(const char *text, ZStaticValue *out, ZTypeParseError *error) {
  return static_value_parse_inner(text, NULL, out, error);
}

bool z_static_value_parse_with_binders(const char *text, const ZTypeBinderScope *scope, ZStaticValue *out, ZTypeParseError *error) {
  return static_value_parse_inner(text, scope, out, error);
}

char *z_static_value_format(const ZStaticValue *value) {
  if (!value || value->kind == Z_STATIC_VALUE_INVALID) return NULL;
  return type_strdup(value->text ? value->text : "");
}

static bool parser_parse_static_value(TypeParser *parser, const char *text, size_t start, ZStaticValue *out);

static bool parser_expected_arg_kind(TypeParser *parser, const char *type_name, size_t arg_index, ZTypeArgKind *out_kind) {
  if (!parser || !parser->scope || !parser->scope->arg_kind) return false;
  return parser->scope->arg_kind(parser->scope->arg_kind_context, type_name, arg_index, out_kind);
}

static bool parser_parse_static_arg(TypeParser *parser, size_t start, ZStaticValue *out) {
  while (parser->text[parser->cursor] && parser->text[parser->cursor] != ',' && parser->text[parser->cursor] != '>') parser->cursor++;
  char *text = trimmed_copy(parser->text + start, parser->cursor - start);
  bool ok = parser_parse_static_value(parser, text, start, out);
  free(text);
  return ok;
}

static bool parser_static_until(TypeParser *parser, char delimiter, ZStaticValue *out) {
  parser_skip_ws(parser);
  size_t start = parser->cursor;
  int depth = 0;
  while (parser->text[parser->cursor]) {
    char ch = parser->text[parser->cursor];
    if (ch == '<' || ch == '[' || ch == '(') depth++;
    else if ((ch == '>' || ch == ']' || ch == ')') && depth > 0) depth--;
    if (depth == 0 && ch == delimiter) break;
    parser->cursor++;
  }
  if (parser->text[parser->cursor] != delimiter) {
    parser_error(parser, "unterminated static value");
    return false;
  }
  char *text = trimmed_copy(parser->text + start, parser->cursor - start);
  bool ok = parser_parse_static_value(parser, text, start, out);
  free(text);
  return ok;
}

static bool parse_type(TypeParser *parser, ZTypeArena *arena, ZTypeId *out);

static bool parser_parse_static_value(TypeParser *parser, const char *text, size_t start, ZStaticValue *out) {
  ZTypeParseError static_error = {0};
  bool ok = z_static_value_parse_with_binders(text, parser ? parser->scope : NULL, out, &static_error);
  if (!ok && parser && parser->error) {
    *parser->error = static_error;
    parser->error->offset = start + static_error.offset;
  }
  return ok;
}

static bool parse_type_arg(TypeParser *parser, ZTypeArena *arena, const char *type_name, size_t arg_index, ZTypeArg *out) {
  parser_skip_ws(parser);
  size_t start = parser->cursor;
  ZTypeArgKind expected_kind = Z_TYPE_ARG_TYPE;
  bool has_expected_kind = parser_expected_arg_kind(parser, type_name, arg_index, &expected_kind);
  if (has_expected_kind && expected_kind == Z_TYPE_ARG_STATIC) {
    ZStaticValue value = {0};
    if (!parser_parse_static_arg(parser, start, &value)) return false;
    *out = (ZTypeArg){.kind = Z_TYPE_ARG_STATIC, .as.static_value = value};
    return true;
  }
  if (!has_expected_kind && ident_start(parser->text[parser->cursor])) {
    size_t ident_end = parser->cursor + 1;
    while (ident_continue(parser->text[ident_end])) ident_end++;
    char *name = type_strndup(parser->text + parser->cursor, ident_end - parser->cursor);
    const ZTypeBinderDecl *static_binder = type_binder_lookup(parser->scope, name, Z_TYPE_BINDER_STATIC);
    size_t after_name = ident_end;
    while (isspace((unsigned char)parser->text[after_name])) after_name++;
    if (static_binder && (parser->text[after_name] == ',' || parser->text[after_name] == '>')) {
      parser->cursor = after_name;
      ZStaticValue value = {0};
      bool ok = static_value_from_binder(static_binder, &value);
      free(name);
      if (!ok) return false;
      *out = (ZTypeArg){.kind = Z_TYPE_ARG_STATIC, .as.static_value = value};
      return true;
    }
    free(name);
  }
  if (!has_expected_kind && (isdigit((unsigned char)parser->text[parser->cursor]) ||
      parser_keyword(parser, "true") ||
      parser_keyword(parser, "false"))) {
    ZStaticValue value = {0};
    if (!parser_parse_static_arg(parser, start, &value)) return false;
    *out = (ZTypeArg){.kind = Z_TYPE_ARG_STATIC, .as.static_value = value};
    return true;
  }
  ZTypeId type = Z_TYPE_ID_INVALID;
  TypeParserMark type_mark = parser_mark(parser, arena);
  if (parse_type(parser, arena, &type)) {
    parser_skip_ws(parser);
    if (parser->text[parser->cursor] == ',' || parser->text[parser->cursor] == '>') {
      *out = (ZTypeArg){.kind = Z_TYPE_ARG_TYPE, .as.type = type};
      return true;
    }
  }
  if (has_expected_kind && expected_kind == Z_TYPE_ARG_TYPE) {
    parser_restore(parser, arena, type_mark);
    parser_error(parser, "expected type argument");
    return false;
  }
  parser_restore(parser, arena, type_mark);
  while (parser->text[parser->cursor] && parser->text[parser->cursor] != ',' && parser->text[parser->cursor] != '>') parser->cursor++;
  char *text = trimmed_copy(parser->text + start, parser->cursor - start);
  ZStaticValue value = {0};
  bool ok = parser_parse_static_value(parser, text, start, &value);
  free(text);
  if (!ok) return false;
  *out = (ZTypeArg){.kind = Z_TYPE_ARG_STATIC, .as.static_value = value};
  return true;
}

static bool parse_type(TypeParser *parser, ZTypeArena *arena, ZTypeId *out) {
  parser_skip_ws(parser);
  if (parser_keyword(parser, "const")) {
    parser->cursor += 5;
    ZTypeId inner = Z_TYPE_ID_INVALID;
    if (!parse_type(parser, arena, &inner)) return false;
    *out = z_type_make_const(arena, inner);
    return *out != Z_TYPE_ID_INVALID;
  }
  if (parser->text[parser->cursor] == '[') {
    parser->cursor++;
    ZStaticValue length = {0};
    if (!parser_static_until(parser, ']', &length)) return false;
    parser->cursor++;
    ZTypeId element = Z_TYPE_ID_INVALID;
    if (!parse_type(parser, arena, &element)) {
      z_static_value_free(&length);
      return false;
    }
    *out = type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_ARRAY, .as.array = {.length = length, .element = element}});
    return *out != Z_TYPE_ID_INVALID;
  }
  char *name = parser_ident(parser);
  if (!name) return false;
  parser_skip_ws(parser);
  if (parser->text[parser->cursor] != '<') {
    const ZTypeBinderDecl *binder = type_binder_lookup(parser->scope, name, Z_TYPE_BINDER_TYPE);
    if (binder) {
      *out = type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_BINDER, .as.binder = {.name = name, .binder = binder->id}});
    } else {
      *out = type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_NAME, .as.name = name});
    }
    return *out != Z_TYPE_ID_INVALID;
  }
  parser->cursor++;
  ZTypeArg *args = NULL;
  size_t arg_len = 0;
  size_t arg_cap = 0;
  parser_skip_ws(parser);
  if (parser->text[parser->cursor] == '>') {
    free(name);
    parser_error(parser, "generic type requires at least one argument");
    return false;
  }
  while (parser->text[parser->cursor]) {
    if (arg_len + 1 > arg_cap) {
      arg_cap = arg_cap ? arg_cap * 2 : 4;
      args = type_reallocarray(args, arg_cap, sizeof(ZTypeArg));
    }
    args[arg_len] = (ZTypeArg){0};
    if (!parse_type_arg(parser, arena, name, arg_len, &args[arg_len])) {
      for (size_t i = 0; i < arg_len; i++) type_arg_free(&args[i]);
      free(args);
      free(name);
      return false;
    }
    arg_len++;
    parser_skip_ws(parser);
    if (parser->text[parser->cursor] == '>') {
      parser->cursor++;
      *out = type_arena_add(arena, (ZTypeNode){.kind = Z_TYPE_NODE_APPLY, .as.apply = {.name = name, .args = args, .arg_len = arg_len}});
      return *out != Z_TYPE_ID_INVALID;
    }
    if (parser->text[parser->cursor] != ',') {
      for (size_t i = 0; i < arg_len; i++) type_arg_free(&args[i]);
      free(args);
      free(name);
      parser_error(parser, "expected ',' or '>' in generic type");
      return false;
    }
    parser->cursor++;
    parser_skip_ws(parser);
  }
  for (size_t i = 0; i < arg_len; i++) type_arg_free(&args[i]);
  free(args);
  free(name);
  parser_error(parser, "unterminated generic type");
  return false;
}

static bool type_parse_inner(ZTypeArena *arena, const char *text, const ZTypeBinderScope *scope, ZTypeId *out, ZTypeParseError *error) {
  if (out) *out = Z_TYPE_ID_INVALID;
  if (error) *error = (ZTypeParseError){0};
  if (!arena) {
    if (error) snprintf(error->message, sizeof(error->message), "%s", "type arena is required");
    return false;
  }
  TypeParser parser = {.text = text ? text : "", .scope = scope, .error = error};
  ZTypeId type = Z_TYPE_ID_INVALID;
  size_t start_len = arena->len;
  if (!parse_type(&parser, arena, &type)) {
    z_type_arena_truncate(arena, start_len);
    return false;
  }
  parser_skip_ws(&parser);
  if (parser.text[parser.cursor] != 0) {
    parser_error(&parser, "unexpected trailing type syntax");
    z_type_arena_truncate(arena, start_len);
    return false;
  }
  if (out) *out = type;
  return true;
}

bool z_type_parse(ZTypeArena *arena, const char *text, ZTypeId *out, ZTypeParseError *error) {
  return type_parse_inner(arena, text, NULL, out, error);
}

bool z_type_parse_with_binders(ZTypeArena *arena, const char *text, const ZTypeBinderScope *scope, ZTypeId *out, ZTypeParseError *error) {
  return type_parse_inner(arena, text, scope, out, error);
}

static void type_format_into(const ZTypeArena *arena, ZTypeId type, TypeBuf *buf);

static void static_value_format_into(const ZStaticValue *value, TypeBuf *buf) {
  if (value && value->text) type_buf_append(buf, value->text);
}

static void type_arg_format_into(const ZTypeArena *arena, const ZTypeArg *arg, TypeBuf *buf) {
  if (!arg) return;
  if (arg->kind == Z_TYPE_ARG_TYPE) type_format_into(arena, arg->as.type, buf);
  else static_value_format_into(&arg->as.static_value, buf);
}

static void type_format_into(const ZTypeArena *arena, ZTypeId type, TypeBuf *buf) {
  const ZTypeNode *node = type_node(arena, type);
  if (!node) {
    type_buf_append(buf, "Unknown");
    return;
  }
  if (node->kind == Z_TYPE_NODE_NAME) {
    type_buf_append(buf, node->as.name);
  } else if (node->kind == Z_TYPE_NODE_BINDER) {
    type_buf_append(buf, node->as.binder.name);
  } else if (node->kind == Z_TYPE_NODE_CONST) {
    type_buf_append(buf, "const ");
    type_format_into(arena, node->as.constant.inner, buf);
  } else if (node->kind == Z_TYPE_NODE_ARRAY) {
    type_buf_append_char(buf, '[');
    static_value_format_into(&node->as.array.length, buf);
    type_buf_append_char(buf, ']');
    type_format_into(arena, node->as.array.element, buf);
  } else if (node->kind == Z_TYPE_NODE_APPLY) {
    type_buf_append(buf, node->as.apply.name);
    type_buf_append_char(buf, '<');
    for (size_t i = 0; i < node->as.apply.arg_len; i++) {
      if (i > 0) type_buf_append_char(buf, ',');
      type_arg_format_into(arena, &node->as.apply.args[i], buf);
    }
    type_buf_append_char(buf, '>');
  }
}

char *z_type_format(const ZTypeArena *arena, ZTypeId type) {
  TypeBuf buf = {0};
  type_format_into(arena, type, &buf);
  return buf.data ? buf.data : type_strdup("");
}

bool z_static_value_equal(const ZStaticValue *left, const ZStaticValue *right) {
  if (!left || !right || left->kind != right->kind) return false;
  if (left->kind == Z_STATIC_VALUE_NUMBER) return left->number == right->number;
  if (left->kind == Z_STATIC_VALUE_BOOL) return left->boolean == right->boolean;
  if (left->kind == Z_STATIC_VALUE_BINDER) return left->binder == right->binder;
  return strcmp(left->text ? left->text : "", right->text ? right->text : "") == 0;
}

static bool type_arg_equal(const ZTypeArena *arena, const ZTypeArg *left, const ZTypeArg *right) {
  if (!left || !right || left->kind != right->kind) return false;
  if (left->kind == Z_TYPE_ARG_TYPE) return z_type_equal(arena, left->as.type, right->as.type);
  return z_static_value_equal(&left->as.static_value, &right->as.static_value);
}

bool z_type_equal(const ZTypeArena *arena, ZTypeId left, ZTypeId right) {
  if (left == right) return true;
  const ZTypeNode *left_node = type_node(arena, left);
  const ZTypeNode *right_node = type_node(arena, right);
  if (!left_node || !right_node || left_node->kind != right_node->kind) return false;
  if (left_node->kind == Z_TYPE_NODE_NAME) return strcmp(left_node->as.name, right_node->as.name) == 0;
  if (left_node->kind == Z_TYPE_NODE_BINDER) return left_node->as.binder.binder == right_node->as.binder.binder;
  if (left_node->kind == Z_TYPE_NODE_CONST) return z_type_equal(arena, left_node->as.constant.inner, right_node->as.constant.inner);
  if (left_node->kind == Z_TYPE_NODE_ARRAY) {
    return z_static_value_equal(&left_node->as.array.length, &right_node->as.array.length) &&
           z_type_equal(arena, left_node->as.array.element, right_node->as.array.element);
  }
  if (left_node->kind == Z_TYPE_NODE_APPLY) {
    if (strcmp(left_node->as.apply.name, right_node->as.apply.name) != 0 ||
        left_node->as.apply.arg_len != right_node->as.apply.arg_len) return false;
    for (size_t i = 0; i < left_node->as.apply.arg_len; i++) {
      if (!type_arg_equal(arena, &left_node->as.apply.args[i], &right_node->as.apply.args[i])) return false;
    }
    return true;
  }
  return false;
}

static uint64_t hash_bytes(uint64_t hash, const char *text) {
  const unsigned char *cursor = (const unsigned char *)(text ? text : "");
  while (*cursor) {
    hash ^= (uint64_t)*cursor++;
    hash *= 1099511628211ull;
  }
  return hash;
}

static uint64_t hash_u64(uint64_t hash, uint64_t value) {
  for (size_t i = 0; i < 8; i++) {
    hash ^= (value >> (i * 8)) & 0xffu;
    hash *= 1099511628211ull;
  }
  return hash;
}

static uint64_t static_value_hash(const ZStaticValue *value) {
  uint64_t hash = 1469598103934665603ull;
  if (!value) return hash;
  hash = hash_u64(hash, (uint64_t)value->kind);
  if (value->kind == Z_STATIC_VALUE_NUMBER) return hash_u64(hash, value->number);
  if (value->kind == Z_STATIC_VALUE_BOOL) return hash_u64(hash, value->boolean ? 1 : 0);
  if (value->kind == Z_STATIC_VALUE_BINDER) return hash_u64(hash, value->binder);
  return hash_bytes(hash, value->text);
}

static uint64_t type_arg_hash(const ZTypeArena *arena, const ZTypeArg *arg) {
  uint64_t hash = 1469598103934665603ull;
  if (!arg) return hash;
  hash = hash_u64(hash, (uint64_t)arg->kind);
  if (arg->kind == Z_TYPE_ARG_TYPE) return hash_u64(hash, z_type_hash(arena, arg->as.type));
  return hash_u64(hash, static_value_hash(&arg->as.static_value));
}

uint64_t z_type_hash(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  uint64_t hash = 1469598103934665603ull;
  if (!node) return hash;
  hash = hash_u64(hash, (uint64_t)node->kind);
  if (node->kind == Z_TYPE_NODE_NAME) return hash_bytes(hash, node->as.name);
  if (node->kind == Z_TYPE_NODE_BINDER) return hash_u64(hash, node->as.binder.binder);
  if (node->kind == Z_TYPE_NODE_CONST) return hash_u64(hash, z_type_hash(arena, node->as.constant.inner));
  if (node->kind == Z_TYPE_NODE_ARRAY) {
    hash = hash_u64(hash, static_value_hash(&node->as.array.length));
    return hash_u64(hash, z_type_hash(arena, node->as.array.element));
  }
  if (node->kind == Z_TYPE_NODE_APPLY) {
    hash = hash_bytes(hash, node->as.apply.name);
    hash = hash_u64(hash, node->as.apply.arg_len);
    for (size_t i = 0; i < node->as.apply.arg_len; i++) hash = hash_u64(hash, type_arg_hash(arena, &node->as.apply.args[i]));
    return hash;
  }
  return hash;
}

ZTypeNodeKind z_type_kind(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  return node ? node->kind : Z_TYPE_NODE_INVALID;
}

const char *z_type_name(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  if (!node) return NULL;
  if (node->kind == Z_TYPE_NODE_NAME) return node->as.name;
  if (node->kind == Z_TYPE_NODE_BINDER) return node->as.binder.name;
  if (node->kind == Z_TYPE_NODE_APPLY) return node->as.apply.name;
  return NULL;
}

ZTypeBinderId z_type_binder(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  return node && node->kind == Z_TYPE_NODE_BINDER ? node->as.binder.binder : Z_TYPE_BINDER_ID_INVALID;
}

ZTypeId z_type_const_inner(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  return node && node->kind == Z_TYPE_NODE_CONST ? node->as.constant.inner : Z_TYPE_ID_INVALID;
}

const ZStaticValue *z_type_array_length(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  return node && node->kind == Z_TYPE_NODE_ARRAY ? &node->as.array.length : NULL;
}

ZTypeId z_type_array_element(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  return node && node->kind == Z_TYPE_NODE_ARRAY ? node->as.array.element : Z_TYPE_ID_INVALID;
}

size_t z_type_apply_arg_len(const ZTypeArena *arena, ZTypeId type) {
  const ZTypeNode *node = type_node(arena, type);
  return node && node->kind == Z_TYPE_NODE_APPLY ? node->as.apply.arg_len : 0;
}

const ZTypeArg *z_type_apply_arg(const ZTypeArena *arena, ZTypeId type, size_t index) {
  const ZTypeNode *node = type_node(arena, type);
  if (!node || node->kind != Z_TYPE_NODE_APPLY || index >= node->as.apply.arg_len) return NULL;
  return &node->as.apply.args[index];
}
