#include "mir_verify.h"

#include <stdio.h>

static const char *mir_type_kind_name(IrTypeKind type) {
  switch (type) {
    case IR_TYPE_VOID: return "Void";
    case IR_TYPE_BOOL: return "Bool";
    case IR_TYPE_U8: return "u8";
    case IR_TYPE_U16: return "u16";
    case IR_TYPE_USIZE: return "usize";
    case IR_TYPE_I32: return "i32";
    case IR_TYPE_U32: return "u32";
    case IR_TYPE_I64: return "i64";
    case IR_TYPE_U64: return "u64";
    case IR_TYPE_BYTE_VIEW: return "ByteView";
    case IR_TYPE_ALLOC: return "Alloc";
    case IR_TYPE_VEC: return "Vec";
    case IR_TYPE_MAYBE_BYTE_VIEW: return "Maybe<ByteView>";
    case IR_TYPE_MAYBE_SCALAR: return "Maybe<scalar>";
    case IR_TYPE_RECORD: return "record";
    case IR_TYPE_UNSUPPORTED:
    default:
      return "unsupported";
  }
}

static bool mir_type_is_value(IrTypeKind type) {
  return type == IR_TYPE_U8 || type == IR_TYPE_U16 || type == IR_TYPE_USIZE || type == IR_TYPE_I32 || type == IR_TYPE_U32 || type == IR_TYPE_I64 || type == IR_TYPE_U64;
}

static bool mir_type_is_direct_abi(IrTypeKind type) {
  return type == IR_TYPE_BOOL || mir_type_is_value(type);
}

static bool mir_type_is_direct_fallible_value(IrTypeKind type) {
  return type == IR_TYPE_VOID || type == IR_TYPE_BOOL || type == IR_TYPE_U8 ||
         type == IR_TYPE_U16 || type == IR_TYPE_USIZE || type == IR_TYPE_I32 ||
         type == IR_TYPE_U32;
}

static void mir_verify_mark_unsupported(IrProgram *ir, const char *message, int line, int column, const char *actual) {
  if (!ir || !ir->mir_valid) return;
  ir->mir_valid = false;
  ir->mir_line = line > 0 ? line : 1;
  ir->mir_column = column > 0 ? column : 1;
  snprintf(ir->mir_message, sizeof(ir->mir_message), "%s", message ? message : "MIR verification failed");
  snprintf(ir->mir_expected, sizeof(ir->mir_expected), "direct backend MIR contract");
  snprintf(ir->mir_actual, sizeof(ir->mir_actual), "%s", actual ? actual : "invalid MIR");
  snprintf(ir->mir_help, sizeof(ir->mir_help), "report this compiler bug with the source program that produced it");
  z_backend_blocker_set(&ir->backend_blocker, NULL, NULL, NULL, "lower", ir->mir_actual);
}

static bool mir_verify_direct_function_contract(IrProgram *ir, const IrFunction *fun) {
  if (!ir || !ir->mir_valid || !fun) return false;
  if (fun->param_count > fun->local_len) {
    char actual[128];
    snprintf(actual, sizeof(actual), "%zu parameter(s) with %zu local(s)", fun->param_count, fun->local_len);
    mir_verify_mark_unsupported(ir, "MIR verifier found parameter count outside the local table", fun->line, fun->column, actual);
    return false;
  }
  for (size_t i = 0; i < fun->param_count; i++) {
    const IrLocal *local = &fun->locals[i];
    if (!local->is_param || local->index != i) {
      char actual[160];
      snprintf(actual, sizeof(actual), "parameter slot %zu maps to local index %u", i, local->index);
      mir_verify_mark_unsupported(ir, "MIR verifier found invalid parameter local layout", local->line, local->column, actual);
      return false;
    }
    if (!mir_type_is_direct_abi(local->type)) {
      char actual[160];
      snprintf(actual, sizeof(actual), "parameter %s has %s", local->name ? local->name : "<unnamed>", mir_type_kind_name(local->type));
      mir_verify_mark_unsupported(ir, "MIR verifier found non-ABI parameter type", local->line, local->column, actual);
      return false;
    }
  }
  for (size_t i = 0; i < fun->local_len; i++) {
    const IrLocal *local = &fun->locals[i];
    if (local->index != i) {
      char actual[160];
      snprintf(actual, sizeof(actual), "local %s has index %u at slot %zu", local->name ? local->name : "<unnamed>", local->index, i);
      mir_verify_mark_unsupported(ir, "MIR verifier found local table index mismatch", local->line, local->column, actual);
      return false;
    }
    if (local->byte_size == 0 || local->alignment == 0) {
      char actual[160];
      snprintf(actual, sizeof(actual), "local %s has size %u alignment %u", local->name ? local->name : "<unnamed>", local->byte_size, local->alignment);
      mir_verify_mark_unsupported(ir, "MIR verifier found invalid local storage layout", local->line, local->column, actual);
      return false;
    }
  }
  if (fun->raises) {
    if (fun->return_type != IR_TYPE_I64 || !mir_type_is_direct_fallible_value(fun->value_return_type)) {
      char actual[160];
      snprintf(actual, sizeof(actual), "fallible return %s value %s", mir_type_kind_name(fun->return_type), mir_type_kind_name(fun->value_return_type));
      mir_verify_mark_unsupported(ir, "MIR verifier found invalid fallible return representation", fun->line, fun->column, actual);
      return false;
    }
  } else if (fun->return_type != IR_TYPE_VOID && !mir_type_is_direct_abi(fun->return_type)) {
    char actual[128];
    snprintf(actual, sizeof(actual), "return %s", mir_type_kind_name(fun->return_type));
    mir_verify_mark_unsupported(ir, "MIR verifier found non-ABI return type", fun->line, fun->column, actual);
    return false;
  }
  return true;
}

static bool mir_verify_direct_call_contract(IrProgram *ir, const IrValue *value) {
  if (!ir || !ir->mir_valid || !value || value->kind != IR_VALUE_CALL) return ir && ir->mir_valid;
  if (value->callee_index >= ir->function_len) {
    char actual[128];
    snprintf(actual, sizeof(actual), "callee index %u with %zu MIR function(s)", value->callee_index, ir->function_len);
    mir_verify_mark_unsupported(ir, "MIR verifier found direct call target outside the function table", value->line, value->column, actual);
    return false;
  }
  const IrFunction *callee = &ir->functions[value->callee_index];
  if (value->arg_len != callee->param_count) {
    char actual[160];
    snprintf(actual, sizeof(actual), "%zu argument(s) for %zu parameter(s) on %s", value->arg_len, callee->param_count, callee->name ? callee->name : "<unnamed>");
    mir_verify_mark_unsupported(ir, "MIR verifier found direct call arity mismatch", value->line, value->column, actual);
    return false;
  }
  IrTypeKind expected_call_type = callee->raises ? IR_TYPE_I64 : callee->return_type;
  if (value->type != expected_call_type || value->element_type != callee->value_return_type) {
    char actual[192];
    snprintf(actual, sizeof(actual), "call returns %s value %s but callee returns %s value %s", mir_type_kind_name(value->type), mir_type_kind_name(value->element_type), mir_type_kind_name(expected_call_type), mir_type_kind_name(callee->value_return_type));
    mir_verify_mark_unsupported(ir, "MIR verifier found direct call return mismatch", value->line, value->column, actual);
    return false;
  }
  for (size_t i = 0; i < value->arg_len; i++) {
    const IrValue *arg = value->args[i];
    const IrLocal *param = &callee->locals[i];
    if (!arg) {
      char actual[128];
      snprintf(actual, sizeof(actual), "missing argument %zu for %s", i, callee->name ? callee->name : "<unnamed>");
      mir_verify_mark_unsupported(ir, "MIR verifier found null direct call argument", value->line, value->column, actual);
      return false;
    }
    if (!param->is_param || arg->type != param->type) {
      char actual[192];
      snprintf(actual, sizeof(actual), "argument %zu has %s but parameter has %s", i, mir_type_kind_name(arg->type), mir_type_kind_name(param->type));
      mir_verify_mark_unsupported(ir, "MIR verifier found direct call argument ABI mismatch", arg->line, arg->column, actual);
      return false;
    }
  }
  return true;
}

static bool mir_verify_direct_call_value(IrProgram *ir, const IrValue *value) {
  if (!ir || !ir->mir_valid) return false;
  if (!value) return true;
  if (value->kind == IR_VALUE_CALL && !mir_verify_direct_call_contract(ir, value)) return false;
  for (size_t i = 0; i < value->arg_len; i++) {
    if (!mir_verify_direct_call_value(ir, value->args[i])) return false;
  }
  if (!mir_verify_direct_call_value(ir, value->index)) return false;
  if (!mir_verify_direct_call_value(ir, value->left)) return false;
  if (!mir_verify_direct_call_value(ir, value->right)) return false;
  return true;
}

static bool mir_verify_direct_call_instrs(IrProgram *ir, const IrInstr *instrs, size_t len) {
  if (!ir || !ir->mir_valid) return false;
  for (size_t i = 0; i < len; i++) {
    const IrInstr *instr = &instrs[i];
    if (!mir_verify_direct_call_value(ir, instr->value)) return false;
    if (!mir_verify_direct_call_value(ir, instr->index)) return false;
    if (!mir_verify_direct_call_instrs(ir, instr->then_instrs, instr->then_len)) return false;
    if (!mir_verify_direct_call_instrs(ir, instr->else_instrs, instr->else_len)) return false;
  }
  return true;
}

bool z_mir_verify_direct_contracts(IrProgram *ir) {
  if (!ir || !ir->mir_valid) return false;
  for (size_t i = 0; i < ir->function_len; i++) {
    if (!mir_verify_direct_function_contract(ir, &ir->functions[i])) return false;
  }
  for (size_t i = 0; i < ir->function_len; i++) {
    if (!mir_verify_direct_call_instrs(ir, ir->functions[i].instrs, ir->functions[i].instr_len)) return false;
  }
  return true;
}
