#ifndef CODIRA_C_CALL_RESOLVE_H
#define CODIRA_C_CALL_RESOLVE_H

#include "zero.h"

typedef enum {
  Z_CALL_UNKNOWN,
  Z_CALL_FUNCTION,
  Z_CALL_STDLIB,
  Z_CALL_RECEIVER,
  Z_CALL_SHAPE_NAMESPACE,
  Z_CALL_CONSTRAINED_INTERFACE,
  Z_CALL_CONCRETE_CONSTRAINED_SHAPE,
  Z_CALL_CHOICE_CONSTRUCTOR
} ZCallKind;

typedef struct {
  size_t param_index;
  const Expr *arg_expr;
  char *expected_type;
  char *actual_type;
} ZCallArgument;

typedef struct {
  char *name;
  char *type;
  bool is_static;
  char *static_type;
} ZCallBinding;

typedef struct {
  ZCallKind kind;
  const Expr *call_expr;
  const Expr *callee_expr;
  const Expr *receiver_expr;
  const TypeArgVec *type_args;
  const Function *callee;
  const Shape *shape;
  const InterfaceDecl *interface;
  const Choice *choice;
  const Param *choice_case;
  size_t param_offset;
  char *callee_name;
  char *return_type;
  char *effect_summary_key;
  ZCallArgument *args;
  size_t arg_len;
  size_t arg_cap;
  ZCallBinding *bindings;
  size_t binding_len;
  size_t binding_cap;
  bool fallible;
} ZCallResolution;

void z_call_resolution_init(ZCallResolution *resolution);
void z_call_resolution_free(ZCallResolution *resolution);
void z_call_resolution_set_callee_name(ZCallResolution *resolution, const char *name);
void z_call_resolution_set_return_type(ZCallResolution *resolution, const char *return_type);
void z_call_resolution_set_effect_summary_key(ZCallResolution *resolution, const char *key);
void z_call_resolution_add_arg(ZCallResolution *resolution, size_t param_index, const Expr *arg_expr, const char *expected_type, const char *actual_type);
const char *z_call_resolution_param_type(const ZCallResolution *resolution, size_t param_index);
void z_call_resolution_add_binding(ZCallResolution *resolution, const char *name, const char *type, bool is_static, const char *static_type);
const char *z_call_resolution_binding_type(const ZCallResolution *resolution, const char *name);
const char *z_call_kind_name(ZCallKind kind);

#endif
