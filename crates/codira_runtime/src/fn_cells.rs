//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The runtime half of Codira function values: the code cells every function
//!   value calls through, and keeping them pointed at the code that is loaded
//!   now, across hot reloads.
//!
//! A function value (a closure, or a named function used as a value) holds
//! the address of a *cell*, never the address of code (see
//! `codira_codegen::ir::closures`). A cell is one word owned by this
//! runtime: the address of the target's current code. Cells are keyed by the
//! target's identity and signature, created when a library that creates such
//! values is linked (its `codira.fn_cell:<key>` dispatch-table slots receive
//! their addresses), and never freed while the runtime lives, so a value
//! created before a reload stays valid after it.
//!
//! After every link and relink, [`FnCells::refresh`] reads the
//! `codira.fn_code:<key>` slots of the libraries loaded now -- each library
//! pre-fills them with its targets' code -- and points every cell at the code
//! for its key. A cell whose key no loaded library offers (the closure was
//! deleted, or its signature changed) points at [`vanished`], which stops the
//! program with a message rather than calling code that is gone, or code
//! with a different signature.
//!
//! Semantics after a reload, then: an existing function value runs the
//! *new* code of the same target (the same closure literal of the same
//! function, or the same named function), with the captures it was created
//! with -- migrated to the new environment layout field by field, like any
//! other object, when the closure's captures changed.

use std::{
    collections::HashMap,
    ffi::c_void,
    io::Write,
    sync::atomic::{AtomicPtr, Ordering},
};

use codira_abi as abi;

use crate::assembly::Assembly;

/// The cells of one runtime.
#[derive(Default)]
pub(crate) struct FnCells {
    /// Boxed so a cell's address never changes.
    cells: HashMap<String, Box<AtomicPtr<c_void>>>,
}

impl FnCells {
    /// The address of the cell for `key`, creating it (pointing at
    /// [`vanished`] until the next refresh) if needed.
    pub(crate) fn cell_for(&mut self, key: &str) -> *const c_void {
        let cell = self
            .cells
            .entry(key.to_owned())
            .or_insert_with(|| Box::new(AtomicPtr::new(vanished as *mut c_void)));
        std::ptr::from_ref::<AtomicPtr<c_void>>(cell).cast()
    }

    /// Points every cell at the code the given (linked) assemblies offer for
    /// its key, or at [`vanished`] when none does.
    pub(crate) fn refresh<'a>(&self, assemblies: impl Iterator<Item = &'a Assembly>) {
        let mut code: HashMap<&str, *const c_void> = HashMap::new();
        for assembly in assemblies {
            for (ptr, prototype) in assembly.info().dispatch_table.iter() {
                if let Some(key) = prototype.name().strip_prefix(abi::FN_VALUE_CODE_PREFIX) {
                    if !ptr.is_null() {
                        code.insert(key, *ptr);
                    }
                }
            }
        }
        for (key, cell) in &self.cells {
            let target = code
                .get(key.as_str())
                .copied()
                .unwrap_or(vanished as *const c_void);
            cell.store(target.cast_mut(), Ordering::Release);
        }
    }

    /// The code the cell for `key` currently points at, if the cell exists.
    #[cfg(test)]
    fn code_of(&self, key: &str) -> Option<*const c_void> {
        self.cells
            .get(key)
            .map(|cell| cell.load(Ordering::Acquire).cast_const())
    }
}

/// What a function value calls when the code it referred to is no longer
/// loaded. Called with whatever arguments the call had: under the C calling
/// conventions of every supported target the caller owns its arguments, so a
/// function that reads none of them and never returns is safe to call with
/// any.
extern "C" fn vanished() {
    let _ignored = std::io::stdout().flush();
    eprintln!(
        "panic: called a function value whose code is no longer loaded: a hot reload removed \
         the closure or function it was created from, or changed its signature"
    );
    std::process::exit(101)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn a_cell_keeps_its_address_and_starts_out_vanished() {
        let mut cells = FnCells::default();
        let first = cells.cell_for("mod::f#closure0:core::fn() -> ()");
        let _other = cells.cell_for("mod::g#closure0:core::fn() -> ()");
        let again = cells.cell_for("mod::f#closure0:core::fn() -> ()");
        assert_eq!(first, again);
        assert_eq!(
            cells.code_of("mod::f#closure0:core::fn() -> ()"),
            Some(vanished as *const c_void)
        );
    }

    #[test]
    fn refresh_without_code_points_cells_at_vanished() {
        let mut cells = FnCells::default();
        cells.cell_for("k");
        cells.refresh(std::iter::empty());
        assert_eq!(cells.code_of("k"), Some(vanished as *const c_void));
    }
}
