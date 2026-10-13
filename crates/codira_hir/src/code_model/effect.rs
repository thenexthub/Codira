//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The code model of an algebraic effect declaration (`effect Name { .. }`,
//!   `spec/LANGUAGE_SPEC.md` section 6): its operations and their resolved
//!   parameter and return types.
//!
//! An effect is a set of operation *signatures*. It has no body and no
//! runtime representation of its own: a `handle` expression supplies the
//! implementation of every operation, and code generation passes a pointer
//! to that handler record (the "evidence") down to whoever performs it. See
//! `codira_codegen::ir::effects` for the runtime side.
use std::sync::Arc;

use codira_hir_input::FileId;
use codira_syntax::ast::{self, NameOwner, TypeAscriptionOwner};

use super::Module;
use crate::{
    has_module::HasModule,
    ids::{EffectId, Lookup},
    name::AsName,
    resolve::HasResolver,
    ty::lower::{lower_types, LowerTyMap},
    type_ref::{LocalTypeRefId, TypeRefMap, TypeRefSourceMap},
    visibility::RawVisibility,
    DefDatabase, DiagnosticSink, HasVisibility, HirDatabase, Name, Ty, TyKind, Visibility,
};

/// An `effect` declaration.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Ord, PartialOrd)]
pub struct Effect {
    pub(crate) id: EffectId,
}

impl From<EffectId> for Effect {
    fn from(id: EffectId) -> Self {
        Effect { id }
    }
}

/// One operation of an effect: `func log(message: i32) -> R`.
#[derive(Debug, PartialEq, Eq)]
pub struct EffectOpData {
    pub name: Name,
    /// The parameter types, in order. Parameter names are documentation only:
    /// a handler arm binds its own names, and `perform` passes positionally.
    pub params: Vec<LocalTypeRefId>,
    /// The type `resume` takes and `perform` produces; unit when the
    /// operation declares none.
    pub ret: LocalTypeRefId,
}

/// The data of an effect declaration, read from its source.
#[derive(Debug, PartialEq, Eq)]
pub struct EffectData {
    pub name: Name,
    pub visibility: RawVisibility,
    pub ops: Vec<EffectOpData>,
    type_ref_map: TypeRefMap,
    type_ref_source_map: TypeRefSourceMap,
}

impl EffectData {
    pub(crate) fn effect_data_query(db: &dyn DefDatabase, id: EffectId) -> Arc<EffectData> {
        let loc = id.lookup(db);
        let item_tree = db.item_tree(loc.id.file_id);
        let effect = &item_tree[loc.id.value];
        let src = item_tree.source(db, loc.id.value);

        let mut types = TypeRefMap::builder();
        let ops = src
            .effect_op_list()
            .into_iter()
            .flat_map(|list| list.ops())
            .filter_map(|op| {
                let name = op.name()?.as_name();
                let params = op
                    .param_list()
                    .into_iter()
                    .flat_map(|list| list.params())
                    .map(|param| types.alloc_from_node_opt(param.ascribed_type().as_ref()))
                    .collect();
                let ret = match op.ret_type().and_then(|rt| rt.type_ref()) {
                    Some(ty) => types.alloc_from_node(&ty),
                    None => types.unit(),
                };
                Some(EffectOpData { name, params, ret })
            })
            .collect();
        let (type_ref_map, type_ref_source_map) = types.finish();

        Arc::new(EffectData {
            name: effect.name.clone(),
            visibility: item_tree[effect.visibility].clone(),
            ops,
            type_ref_map,
            type_ref_source_map,
        })
    }

    pub fn type_ref_map(&self) -> &TypeRefMap {
        &self.type_ref_map
    }

    pub fn type_ref_source_map(&self) -> &TypeRefSourceMap {
        &self.type_ref_source_map
    }

    /// The index of the operation called `name`, if there is one.
    pub fn op_index(&self, name: &Name) -> Option<usize> {
        self.ops.iter().position(|op| &op.name == name)
    }
}

/// The resolved signature of one effect operation.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EffectOpSig {
    pub name: Name,
    pub params: Vec<Ty>,
    pub ret: Ty,
}

impl Effect {
    pub fn module(self, db: &dyn HirDatabase) -> Module {
        self.id.module(db).into()
    }

    pub fn file_id(self, db: &dyn HirDatabase) -> FileId {
        self.id.lookup(db).id.file_id
    }

    pub fn data(self, db: &dyn DefDatabase) -> Arc<EffectData> {
        db.effect_data(self.id)
    }

    pub fn name(self, db: &dyn HirDatabase) -> Name {
        self.data(db).name.clone()
    }

    /// Resolves every type mentioned in the effect's operations.
    pub fn lower(self, db: &dyn HirDatabase) -> Arc<LowerTyMap> {
        db.lower_effect(self)
    }

    /// The number of operations the effect declares.
    pub fn op_count(self, db: &dyn HirDatabase) -> usize {
        self.data(db).ops.len()
    }

    /// The resolved signature of the operation at `index`.
    pub fn op_sig(self, db: &dyn HirDatabase, index: usize) -> EffectOpSig {
        let data = self.data(db);
        let lower = self.lower(db);
        let op = &data.ops[index];
        // An alias is seen through, exactly as `fn_sig_for_fn` does for a
        // function's signature: what flows on is the type it names.
        let resolve = |tr: LocalTypeRefId| {
            let ty = lower[tr].clone();
            match ty.interned() {
                TyKind::TypeAlias(alias) => alias.target_type(db),
                _ => ty,
            }
        };
        EffectOpSig {
            name: op.name.clone(),
            params: op.params.iter().map(|&p| resolve(p)).collect(),
            ret: resolve(op.ret),
        }
    }

    /// Every operation's resolved signature, in declaration order.
    pub fn op_sigs(self, db: &dyn HirDatabase) -> Vec<EffectOpSig> {
        (0..self.op_count(db))
            .map(|idx| self.op_sig(db, idx))
            .collect()
    }

    /// Reports unresolved types in the operation signatures.
    pub fn diagnostics(self, db: &dyn HirDatabase, sink: &mut DiagnosticSink<'_>) {
        let data = self.data(db);
        let lower = self.lower(db);
        lower.add_diagnostics(db, self.file_id(db), data.type_ref_source_map(), sink);
    }
}

pub(crate) fn lower_effect_query(db: &dyn HirDatabase, effect: Effect) -> Arc<LowerTyMap> {
    let data = effect.data(db);
    lower_types(db, &effect.id.resolver(db), data.type_ref_map())
}

impl HasVisibility for Effect {
    fn visibility(&self, db: &dyn HirDatabase) -> Visibility {
        self.data(db).visibility.resolve(db, &self.id.resolver(db))
    }
}

impl crate::HasSource for Effect {
    type Ast = ast::EffectDef;
    fn source(&self, db: &dyn DefDatabase) -> crate::InFile<Self::Ast> {
        self.id.lookup(db).source(db)
    }
}

impl crate::Function {
    /// The effects this function declares with `uses`, resolved, without
    /// duplicates, in declaration order.
    ///
    /// The order is load-bearing: code generation passes one evidence
    /// pointer per entry, in this order, after the ordinary parameters. A
    /// name that does not resolve to an effect is left out here and
    /// reported by the effect validator.
    pub fn effects(self, db: &dyn HirDatabase) -> Vec<Effect> {
        let data = self.data(db);
        if data.effects().is_empty() {
            return Vec::new();
        }
        let resolver = self.id.resolver(db);
        let mut effects = Vec::new();
        for name in data.effects() {
            let path = crate::Path::from(name.clone());
            if let Some((id, _)) = resolver.resolve_path_as_effect(db, &path) {
                let effect = Effect::from(id);
                if !effects.contains(&effect) {
                    effects.push(effect);
                }
            }
        }
        effects
    }
}
