//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The code model of a session type declaration (`session Name { send T; recv
//!   T; choose { .. } or { .. } }`): its steps as written, and the protocol
//!   they describe as a graph the session checker (`expr::session`) walks.
//!
//! # The protocol
//!
//! A session declaration describes what one endpoint of a connected pair
//! does, in order: `send T` (this endpoint sends a `T`), `recv T` (it
//! receives one), `choose { A } or { B } ..` (it decides which branch the
//! conversation takes) and `offer { A } or { B } ..` (the other endpoint
//! decides). When the steps run out the protocol has ended. Steps written
//! after a choice continue every branch: `choose { A } or { B }; C` is
//! `choose { A; C } or { B; C }`.
//!
//! The other endpoint follows the *dual* protocol, written `dual Name`:
//! every `send` becomes a `recv` and every `choose` an `offer`, and the other
//! way round. The graph is stored once, for the declared side; a dual
//! endpoint reads it through [`Protocol::step`] with `dual` set.
//!
//! Nodes are hash-consed: two branches that continue with the same steps
//! reach the *same* node, so "both branches of an `if` left the endpoint at
//! the same point of its protocol" is an equality of node ids.
use std::sync::Arc;

use codira_hir_input::FileId;
use codira_syntax::{
    ast::{self, SessionStepKind},
    AstPtr, SyntaxNodePtr,
};
use rustc_hash::FxHashMap;

use super::Module;
use crate::{
    diagnostics::SessionError,
    has_module::HasModule,
    ids::{Lookup, SessionId},
    resolve::HasResolver,
    ty::lower::{lower_types, LowerTyMap},
    type_ref::{LocalTypeRefId, TypeRefMap, TypeRefMapBuilder, TypeRefSourceMap},
    visibility::RawVisibility,
    DefDatabase, DiagnosticSink, HasVisibility, HirDatabase, HirDisplay, Name, Ty, TyKind,
    Visibility,
};

/// A `session` declaration.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Ord, PartialOrd)]
pub struct Session {
    pub(crate) id: SessionId,
}

impl From<SessionId> for Session {
    fn from(id: SessionId) -> Self {
        Session { id }
    }
}

/// One step of a session as written.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum SessionStepData {
    Send(LocalTypeRefId),
    Recv(LocalTypeRefId),
    Choose(Vec<Vec<SessionStepData>>),
    Offer(Vec<Vec<SessionStepData>>),
}

/// The data of a session declaration, read from its source.
#[derive(Debug, PartialEq, Eq)]
pub struct SessionData {
    pub name: Name,
    pub visibility: RawVisibility,
    pub steps: Vec<SessionStepData>,
    type_ref_map: TypeRefMap,
    type_ref_source_map: TypeRefSourceMap,
}

fn lower_steps(
    types: &mut TypeRefMapBuilder,
    protocol: Option<ast::SessionProtocol>,
) -> Vec<SessionStepData> {
    protocol
        .into_iter()
        .flat_map(|protocol| protocol.steps())
        .filter_map(|step| {
            let branches = |types: &mut TypeRefMapBuilder| {
                step.branches()
                    .map(|branch| lower_steps(types, Some(branch)))
                    .collect()
            };
            Some(match step.kind()? {
                SessionStepKind::Send => {
                    SessionStepData::Send(types.alloc_from_node_opt(step.type_ref().as_ref()))
                }
                SessionStepKind::Recv => {
                    SessionStepData::Recv(types.alloc_from_node_opt(step.type_ref().as_ref()))
                }
                SessionStepKind::Choose => SessionStepData::Choose(branches(types)),
                SessionStepKind::Offer => SessionStepData::Offer(branches(types)),
            })
        })
        .collect()
}

impl SessionData {
    pub(crate) fn session_data_query(db: &dyn DefDatabase, id: SessionId) -> Arc<SessionData> {
        let loc = id.lookup(db);
        let item_tree = db.item_tree(loc.id.file_id);
        let session = &item_tree[loc.id.value];
        let src = item_tree.source(db, loc.id.value);

        let mut types = TypeRefMap::builder();
        let steps = lower_steps(&mut types, src.protocol());
        let (type_ref_map, type_ref_source_map) = types.finish();

        Arc::new(SessionData {
            name: session.name.clone(),
            visibility: item_tree[session.visibility].clone(),
            steps,
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
}

/// A point in a protocol: an index into [`Protocol`]'s nodes.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct ProtocolNodeId(u32);

/// One point of a protocol, from the declared side's view.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum ProtocolNode {
    /// Nothing more to do.
    End,
    Send(Ty, ProtocolNodeId),
    Recv(Ty, ProtocolNodeId),
    Choose(Vec<ProtocolNodeId>),
    Offer(Vec<ProtocolNodeId>),
}

/// What an endpoint must do next: a [`ProtocolNode`] read from one side.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ProtocolStep<'a> {
    End,
    Send(&'a Ty, ProtocolNodeId),
    Recv(&'a Ty, ProtocolNodeId),
    Choose(&'a [ProtocolNodeId]),
    Offer(&'a [ProtocolNodeId]),
}

/// A session's protocol as a hash-consed graph (see the module docs).
#[derive(Debug, PartialEq, Eq)]
pub struct Protocol {
    nodes: Vec<ProtocolNode>,
    start: ProtocolNodeId,
}

struct ProtocolBuilder<'a> {
    nodes: Vec<ProtocolNode>,
    ids: FxHashMap<ProtocolNode, ProtocolNodeId>,
    types: &'a LowerTyMap,
}

impl ProtocolBuilder<'_> {
    fn intern(&mut self, node: ProtocolNode) -> ProtocolNodeId {
        if let Some(&id) = self.ids.get(&node) {
            return id;
        }
        let id = ProtocolNodeId(u32::try_from(self.nodes.len()).expect("fewer than 2^32 nodes"));
        self.nodes.push(node.clone());
        self.ids.insert(node, id);
        id
    }

    /// `steps` followed by `then`.
    fn build(&mut self, steps: &[SessionStepData], then: ProtocolNodeId) -> ProtocolNodeId {
        let mut next = then;
        for step in steps.iter().rev() {
            let node = match step {
                SessionStepData::Send(ty) => ProtocolNode::Send(self.types[*ty].clone(), next),
                SessionStepData::Recv(ty) => ProtocolNode::Recv(self.types[*ty].clone(), next),
                SessionStepData::Choose(branches) => {
                    ProtocolNode::Choose(branches.iter().map(|b| self.build(b, next)).collect())
                }
                SessionStepData::Offer(branches) => {
                    ProtocolNode::Offer(branches.iter().map(|b| self.build(b, next)).collect())
                }
            };
            next = self.intern(node);
        }
        next
    }
}

impl Protocol {
    /// Where an endpoint starts.
    pub fn start(&self) -> ProtocolNodeId {
        self.start
    }

    /// What an endpoint at `node` must do next; `dual` reads the node from
    /// the other side.
    pub fn step(&self, node: ProtocolNodeId, dual: bool) -> ProtocolStep<'_> {
        match (&self.nodes[node.0 as usize], dual) {
            (ProtocolNode::End, _) => ProtocolStep::End,
            (ProtocolNode::Send(ty, next), false) | (ProtocolNode::Recv(ty, next), true) => {
                ProtocolStep::Send(ty, *next)
            }
            (ProtocolNode::Recv(ty, next), false) | (ProtocolNode::Send(ty, next), true) => {
                ProtocolStep::Recv(ty, *next)
            }
            (ProtocolNode::Choose(branches), false) | (ProtocolNode::Offer(branches), true) => {
                ProtocolStep::Choose(branches)
            }
            (ProtocolNode::Offer(branches), false) | (ProtocolNode::Choose(branches), true) => {
                ProtocolStep::Offer(branches)
            }
        }
    }

    /// A short description of what is expected at `node`, for diagnostics:
    /// `send i64`, `recv bool`, `choose (2 branches)`, `offer (2 branches)`,
    /// or `end`.
    pub fn describe(&self, db: &dyn HirDatabase, node: ProtocolNodeId, dual: bool) -> String {
        match self.step(node, dual) {
            ProtocolStep::End => "end (the protocol is finished)".to_string(),
            ProtocolStep::Send(ty, _) => format!("send {}", ty.display(db)),
            ProtocolStep::Recv(ty, _) => format!("recv {}", ty.display(db)),
            ProtocolStep::Choose(branches) => {
                format!("choose (one of {} branches, `.choose(n)`)", branches.len())
            }
            ProtocolStep::Offer(branches) => {
                format!(
                    "offer (`offer ep {{ .. }}` with {} branches)",
                    branches.len()
                )
            }
        }
    }
}

impl Session {
    pub fn module(self, db: &dyn HirDatabase) -> Module {
        self.id.module(db).into()
    }

    pub fn file_id(self, db: &dyn HirDatabase) -> FileId {
        self.id.lookup(db).id.file_id
    }

    pub fn data(self, db: &dyn DefDatabase) -> Arc<SessionData> {
        db.session_data(self.id)
    }

    pub fn name(self, db: &dyn HirDatabase) -> Name {
        self.data(db).name.clone()
    }

    /// Resolves every payload type of the session.
    pub fn lower(self, db: &dyn HirDatabase) -> Arc<LowerTyMap> {
        db.lower_session(self)
    }

    /// The protocol graph.
    pub fn protocol(self, db: &dyn HirDatabase) -> Arc<Protocol> {
        db.session_protocol(self)
    }

    /// Reports unresolved payload types, and payloads a session cannot carry:
    /// like a channel, it carries integers, floats and `bool` only.
    pub fn diagnostics(self, db: &dyn HirDatabase, sink: &mut DiagnosticSink<'_>) {
        let data = self.data(db);
        let lower = self.lower(db);
        let file = self.file_id(db);
        lower.add_diagnostics(db, file, data.type_ref_source_map(), sink);
        for (type_ref, _) in data.type_ref_map().iter() {
            let ty = &lower[type_ref];
            let scalar = matches!(
                ty.interned(),
                TyKind::Int(_) | TyKind::Float(_) | TyKind::Bool | TyKind::Unknown
            );
            if scalar {
                continue;
            }
            let Some(ptr) = data.type_ref_source_map().type_ref_syntax(type_ref) else {
                continue;
            };
            sink.push(SessionError {
                file,
                node: ptr_node(ptr),
                message: format!(
                    "a session carries integers, floats or `bool`, not `{}`: tasks share \
                     nothing else",
                    ty.display(db)
                ),
            });
        }
    }
}

fn ptr_node(ptr: AstPtr<ast::TypeRef>) -> SyntaxNodePtr {
    ptr.syntax_node_ptr()
}

pub(crate) fn lower_session_query(db: &dyn HirDatabase, session: Session) -> Arc<LowerTyMap> {
    let data = session.data(db);
    lower_types(db, &session.id.resolver(db), data.type_ref_map())
}

pub(crate) fn session_protocol_query(db: &dyn HirDatabase, session: Session) -> Arc<Protocol> {
    let data = session.data(db);
    let lower = session.lower(db);
    let mut builder = ProtocolBuilder {
        nodes: Vec::new(),
        ids: FxHashMap::default(),
        types: &lower,
    };
    let end = builder.intern(ProtocolNode::End);
    let start = builder.build(&data.steps, end);
    Arc::new(Protocol {
        nodes: builder.nodes,
        start,
    })
}

impl HasVisibility for Session {
    fn visibility(&self, db: &dyn HirDatabase) -> Visibility {
        self.data(db).visibility.resolve(db, &self.id.resolver(db))
    }
}

impl crate::HasSource for Session {
    type Ast = ast::SessionDef;
    fn source(&self, db: &dyn DefDatabase) -> crate::InFile<Self::Ast> {
        self.id.lookup(db).source(db)
    }
}
