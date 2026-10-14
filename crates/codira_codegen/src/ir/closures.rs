//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The module-group half of function values: which closures and named
//!   functions a group turns into values, their code symbols, the layout of
//!   their environments, and the runtime type information both need.
//!
//! # Representation
//!
//! A function value (any value of type `func(..) -> R`) is a reference to a
//! garbage-collected object, exactly like a class instance:
//!
//! ```text
//! object = { usize cell, capture_0, .., capture_n-1 }
//! ```
//!
//! * `cell` is the address of a *code cell* owned by the runtime: one word
//!   holding the address of the code to call. A call loads the object, then the
//!   cell, then calls the code with the function value itself as a hidden first
//!   argument (from which the code reads its captures) followed by the ordinary
//!   arguments.
//! * A closure's object has one field per captured binding after `cell`
//!   (captured by value when the closure is created). A named function used as
//!   a value has no captures; its object is just the cell, and its code is a
//!   *thunk* that calls the function.
//!
//! The function *type* `func(A) -> R` is described to the runtime as a
//! garbage-collected struct with the single field `cell`: every function
//! value object starts with it, so a field, array element or capture of
//! function type is traced as a reference, and the object it points to is
//! traced by its own, fuller type.
//!
//! # Why a cell, not a code pointer
//!
//! Hot reloading replaces a module's library: the old code is unmapped. A
//! function value that held a raw code pointer would keep calling into the
//! old library and crash once it is gone. The cell belongs to the runtime
//! and outlives every library. Each module group lists, in its dispatch
//! table, one entry per target it can create values for:
//!
//! * `codira.fn_cell:<key>` -- left empty; the runtime writes the address of
//!   the cell for `<key>` into it when linking. Creating a function value loads
//!   it.
//! * `codira.fn_code:<key>` -- pre-filled with the target's code. The runtime
//!   reads it after every link and relink and points the cell for `<key>` at
//!   the code currently loaded.
//!
//! `<key>` is the target's symbol plus its signature, so a reload that keeps
//! a closure but changes its signature leaves the old values' cell without
//! code: calling one then stops the program with a message instead of
//! calling code with the wrong signature.

use std::{
    collections::{BTreeSet, HashMap},
    sync::Arc,
};

use codira_abi::Guid;
use codira_hir::{ExprId, HirDatabase, IntTy, ModuleDef, Pat, PatId, Ty, TyKind};
use inkwell::types::StructType;

use crate::{
    ir::ty::HirTypeCache,
    module_group::ModuleGroup,
    type_info::{TypeId, TypeIdData},
};

/// What a function-value target's code does.
#[derive(Debug, Clone, Eq, PartialEq)]
pub(crate) enum TargetKind {
    /// The body of the closure literal `expr` in `owner`'s body.
    Closure {
        owner: codira_hir::Function,
        expr: ExprId,
    },
    /// A thunk that calls the named function.
    Item(codira_hir::Function),
}

/// One captured binding of a closure.
#[derive(Debug, Clone, Eq, PartialEq)]
pub(crate) struct Capture {
    pub pat: PatId,
    /// The field name in the environment's runtime type information.
    pub name: String,
    pub ty: Ty,
}

/// Code a function value can call.
#[derive(Debug, Clone, Eq, PartialEq)]
pub(crate) struct FnValueTarget {
    /// The LLVM symbol of the code, and the target's identity: the
    /// enclosing function's full name and the closure's index for a closure
    /// (`mod::f#closure0`), the function's full name and the creating group
    /// for a thunk (`mod::square#fn@mod`).
    pub symbol: String,
    /// `symbol` plus the signature: what cells are keyed by.
    pub key: String,
    pub kind: TargetKind,
    /// The function type of values of this target.
    pub fn_ty: Ty,
    /// The captures, in object field order after `cell`.
    pub captures: Vec<Capture>,
    /// A function of the group whose body generator compiles the target.
    pub creator: codira_hir::Function,
}

/// A garbage-collected struct the runtime needs type information for that
/// has no `codira_hir::Struct` behind it: a function type, or a closure
/// environment.
pub(crate) struct SyntheticStruct<'ink> {
    pub name: String,
    pub guid: Guid,
    pub fields: Vec<(String, Arc<TypeId>)>,
    pub ir_type: StructType<'ink>,
}

/// Every function-value target of a module group, and every function type it
/// mentions.
#[derive(Debug, Default, Clone, Eq, PartialEq)]
pub(crate) struct FnValueTargets {
    targets: Vec<FnValueTarget>,
    closures: HashMap<(codira_hir::Function, ExprId), usize>,
    items: HashMap<codira_hir::Function, usize>,
    /// The guid strings of the function types the group mentions, sorted so
    /// their definitions come out in a stable order.
    fn_types: BTreeSet<String>,
}

impl FnValueTargets {
    /// Collects the targets of every function of `module_group`.
    pub fn collect(db: &dyn HirDatabase, module_group: &ModuleGroup) -> Self {
        let mut targets = FnValueTargets::default();
        let functions: Vec<codira_hir::Function> = module_group
            .iter()
            .flat_map(|module| module.all_functions(db))
            .filter(|f| !f.is_extern(db))
            .collect();

        for &function in &functions {
            let infer = function.infer(db);
            let body = function.body(db);
            let full_name = function.full_name(db);
            for (idx, (expr, info)) in infer.closures().into_iter().enumerate() {
                let symbol = format!("{full_name}#closure{idx}");
                let fn_ty = Ty::function(info.params.clone(), info.ret.clone());
                let mut used_names: Vec<String> = Vec::new();
                let captures = info
                    .captures
                    .iter()
                    .map(|&pat| {
                        let base = match &body[pat] {
                            Pat::Bind { name } => name.to_string(),
                            _ => "capture".to_string(),
                        };
                        let mut name = base.clone();
                        let mut n = 1;
                        while used_names.contains(&name) {
                            name = format!("{base}#{n}");
                            n += 1;
                        }
                        used_names.push(name.clone());
                        Capture {
                            pat,
                            name,
                            ty: infer[pat].clone(),
                        }
                    })
                    .collect();
                targets.push(
                    FnValueTarget {
                        key: key(db, &symbol, &fn_ty),
                        symbol,
                        kind: TargetKind::Closure {
                            owner: function,
                            expr,
                        },
                        fn_ty,
                        captures,
                        creator: function,
                    },
                    db,
                );
            }
            for (_, item) in infer.function_values() {
                if targets.items.contains_key(&item) {
                    continue;
                }
                let sig = item
                    .ty(db)
                    .callable_sig(db)
                    .expect("a function is callable");
                let fn_ty = Ty::function(sig.params().to_vec(), sig.ret().clone());
                let symbol = format!("{}#fn@{}", item.full_name(db), module_group.name);
                targets.push(
                    FnValueTarget {
                        key: key(db, &symbol, &fn_ty),
                        symbol,
                        kind: TargetKind::Item(item),
                        fn_ty,
                        captures: Vec::new(),
                        creator: function,
                    },
                    db,
                );
            }

            // Every function type the body or signature mentions needs
            // runtime type information: it may be a field, an array
            // element, a capture or a parameter the runtime checks.
            for (_, ty) in infer.expr_types() {
                targets.note_fn_types(db, ty);
            }
            for (_, ty) in infer.pat_types() {
                targets.note_fn_types(db, ty);
            }
            let sig = function
                .ty(db)
                .callable_sig(db)
                .expect("a function is callable");
            for ty in sig.params().iter().chain(std::iter::once(sig.ret())) {
                targets.note_fn_types(db, ty);
            }
        }
        for def in module_group
            .iter()
            .flat_map(|module| module.declarations(db))
        {
            if let ModuleDef::Struct(s) = def {
                for field in s.fields(db) {
                    targets.note_fn_types(db, &field.ty(db));
                }
            }
        }
        targets
    }

    fn push(&mut self, target: FnValueTarget, db: &dyn HirDatabase) {
        let idx = self.targets.len();
        match &target.kind {
            TargetKind::Closure { owner, expr } => {
                self.closures.insert((*owner, *expr), idx);
            }
            TargetKind::Item(item) => {
                self.items.insert(*item, idx);
            }
        }
        self.note_fn_types(db, &target.fn_ty.clone());
        for capture in target.captures.clone() {
            self.note_fn_types(db, &capture.ty);
        }
        self.targets.push(target);
    }

    fn note_fn_types(&mut self, db: &dyn HirDatabase, ty: &Ty) {
        use codira_hir::TypeWalk;
        ty.walk(&mut |inner| {
            if matches!(inner.interned(), TyKind::Function(_)) {
                if let Some(name) = inner.guid_string(db) {
                    self.fn_types.insert(name);
                }
            }
        });
    }

    /// Every target, in collection order.
    pub fn targets(&self) -> &[FnValueTarget] {
        &self.targets
    }

    /// The target of the closure literal `expr` in `owner`.
    pub fn closure(&self, owner: codira_hir::Function, expr: ExprId) -> &FnValueTarget {
        &self.targets[self.closures[&(owner, expr)]]
    }

    /// The thunk target of the named function `item`.
    pub fn item(&self, item: codira_hir::Function) -> &FnValueTarget {
        &self.targets[self.items[&item]]
    }

    /// The LLVM type of a target's object: `cell`, then the captures.
    pub fn object_type<'ink>(
        target: &FnValueTarget,
        hir_types: &HirTypeCache<'_, 'ink>,
    ) -> StructType<'ink> {
        let mut fields = vec![hir_types.get_usize_type().into()];
        fields.extend(target.captures.iter().map(|capture| {
            hir_types
                .get_basic_type(&capture.ty)
                .expect("a capture has a basic type")
        }));
        hir_types.context().struct_type(&fields, false)
    }

    /// The runtime type id of a target's object. A thunk's object is just
    /// the cell, so it is an object of the function type itself; a closure
    /// gets a type of its own, named after it.
    pub fn object_type_id(
        target: &FnValueTarget,
        db: &dyn HirDatabase,
        hir_types: &HirTypeCache<'_, '_>,
    ) -> Arc<TypeId> {
        if target.captures.is_empty() {
            return hir_types.type_id(&target.fn_ty);
        }
        Arc::new(TypeId {
            name: target.symbol.clone(),
            data: TypeIdData::Concrete(Guid::from_str(&env_guid_string(db, target))),
        })
    }

    /// The type definitions this group must give the runtime: one per
    /// function type it mentions, and one per closure environment.
    pub fn synthetic_structs<'ink>(
        &self,
        db: &dyn HirDatabase,
        hir_types: &HirTypeCache<'_, 'ink>,
    ) -> Vec<SyntheticStruct<'ink>> {
        let usize_id = hir_types.type_id(&TyKind::Int(IntTy::usize()).intern());
        let cell_only = hir_types
            .context()
            .struct_type(&[hir_types.get_usize_type().into()], false);
        let mut structs: Vec<SyntheticStruct<'ink>> = self
            .fn_types
            .iter()
            .map(|name| SyntheticStruct {
                name: name.clone(),
                guid: Guid::from_str(name),
                fields: vec![("cell".to_string(), usize_id.clone())],
                ir_type: cell_only,
            })
            .collect();
        for target in &self.targets {
            if target.captures.is_empty() {
                continue;
            }
            let mut fields = vec![("cell".to_string(), usize_id.clone())];
            fields.extend(
                target
                    .captures
                    .iter()
                    .map(|c| (c.name.clone(), hir_types.type_id(&c.ty))),
            );
            structs.push(SyntheticStruct {
                name: target.symbol.clone(),
                guid: Guid::from_str(&env_guid_string(db, target)),
                fields,
                ir_type: Self::object_type(target, hir_types),
            });
        }
        structs
    }
}

/// The identity cells are keyed by: the symbol and the signature.
fn key(db: &dyn HirDatabase, symbol: &str, fn_ty: &Ty) -> String {
    let signature = fn_ty
        .guid_string(db)
        .expect("a function value's type has a guid string");
    format!("{symbol}:{signature}")
}

/// A closure environment's guid string: its name and its fields, like a
/// struct's, so a reload that changes what a closure captures changes the
/// type and migrates its live environments field by field.
fn env_guid_string(db: &dyn HirDatabase, target: &FnValueTarget) -> String {
    let fields: Vec<String> = target
        .captures
        .iter()
        .map(|c| {
            format!(
                "{}: {}",
                c.name,
                c.ty.guid_string(db)
                    .expect("a capture's type has a guid string")
            )
        })
        .collect();
    format!("closure {}{{cell,{}}}", target.symbol, fields.join(","))
}
