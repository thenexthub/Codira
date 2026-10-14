//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Checks `@asm(...)` declarations (see `crate::inline_asm`) before codegen
//!   hands them to LLVM.

use codira_syntax::{AstNode, SyntaxNodePtr};

use super::ExprValidator;
use crate::{
    code_model::src::HasSource,
    diagnostics::{DiagnosticSink, InvalidInlineAsm},
    display::HirDisplay,
    inline_asm::{asm_variants, count_constraints, has_asm_attribute, select_variant},
    Ty, TyKind,
};

/// Whether a value of `ty` can be an inline-assembly operand: it has to fit
/// a register, which is what Codira's integers and floats do. `bool` is left
/// out: it is an `i1` in LLVM, which register constraints do not reliably
/// accept, so a flag crosses as a `u8` instead.
fn is_register_type(ty: &Ty) -> bool {
    matches!(ty.interned(), TyKind::Int(_) | TyKind::Float(_))
}

impl ExprValidator<'_> {
    pub fn validate_inline_asm(&self, sink: &mut DiagnosticSink<'_>) {
        let src = self.func.source(self.db);
        if !has_asm_attribute(&src.value) {
            return;
        }
        let mut report = |message: String| {
            sink.push(InvalidInlineAsm {
                file: src.file_id,
                func: SyntaxNodePtr::new(src.value.syntax()),
                message,
            });
        };

        if !self.func.is_extern(self.db) {
            report(
                "inline assembly is declared on a body-less `extern func`, and this function is \
                 not one"
                    .to_owned(),
            );
            return;
        }
        if self.func.is_intrinsic(self.db) {
            report(
                "a function in an `extern \"codira-intrinsic\"` block is already emitted by the \
                 compiler and cannot also carry `@asm`"
                    .to_owned(),
            );
            return;
        }

        let (variants, errors) = asm_variants(&src.value);
        for error in errors {
            report(error);
        }
        if variants.is_empty() {
            return;
        }

        let Some(sig) = self.func.ty(self.db).callable_sig(self.db) else {
            return;
        };
        for (index, param) in sig.params().iter().enumerate() {
            if !is_register_type(param) {
                report(format!(
                    "parameter {} is a `{}`; inline assembly operands are integers and floats",
                    index + 1,
                    param.display(self.db)
                ));
            }
        }
        let ret = sig.ret();
        let returns_value = !ret.is_empty() && !ret.is_never();
        if returns_value && !is_register_type(ret) {
            report(format!(
                "it returns a `{}`; inline assembly produces integers and floats",
                ret.display(self.db)
            ));
        }

        for variant in &variants {
            let where_ = variant
                .arch
                .as_ref()
                .map_or_else(String::new, |arch| format!(" for `{arch}`"));
            // `count_constraints` already accepted it when the variant was
            // read, or the variant would not be here.
            let Ok(counts) = count_constraints(&variant.constraints) else {
                continue;
            };
            let expected_outputs = usize::from(returns_value);
            if counts.outputs != expected_outputs {
                report(format!(
                    "the constraints{where_} name {} output(s), but the function {}",
                    counts.outputs,
                    if returns_value {
                        "returns one value"
                    } else {
                        "returns nothing"
                    }
                ));
            }
            if counts.inputs != sig.params().len() {
                report(format!(
                    "the constraints{where_} name {} input(s), but the function takes {} \
                     parameter(s)",
                    counts.inputs,
                    sig.params().len()
                ));
            }
        }

        let arch = self.db.target().arch;
        if select_variant(&variants, &arch).is_none() {
            let written: Vec<String> = variants
                .iter()
                .filter_map(|v| v.arch.as_ref().map(|a| format!("`{a}`")))
                .collect();
            report(format!(
                "there is no variant for `{arch}`, the target being compiled for (variants exist \
                 for {}); add `@asm(arch: \"{arch}\", ...)` or one without `arch:`",
                written.join(", ")
            ));
        }
    }
}
