//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Builds a freestanding WebAssembly module for a `wasm32-*` target.
//!
//! # What a module can hold
//!
//! A native build produces a `.codiralib`: a shared library whose dispatch
//! table, GC allocator handle and type table the Codira runtime patches in
//! when it loads it. A WebAssembly module has no such runtime on the other
//! side. So this first slice compiles only what needs none of it: functions
//! marked `@export("C")` whose parameters and results are scalars, calling
//! only functions in the same module that need nothing either. The module
//! exports exactly those functions (and the linear memory LLD gives it) and
//! imports nothing.
//!
//! Anything that needs the runtime is refused before any code is emitted,
//! with the reason: a function reached through the dispatch table (a call
//! into another module, a runtime intrinsic such as allocation), a GC
//! allocation, or runtime type information. The alternative -- emitting the
//! module anyway -- would compile, link, and then trap the first time the
//! zeroed dispatch table was called through.
//!
//! The module goes through the same pipeline as a native build: HIR is
//! lowered by `ir::file`, Eidos runs according to the build mode, LLVM's
//! WebAssembly backend emits the object, and LLD's `wasm-ld` driver links it
//! (in-process where LLD is linked in; see `linker::run_lld`).

use codira_hir::TyKind;
use inkwell::{context::Context, values::BasicValue};
use tempfile::NamedTempFile;

use crate::{
    code_gen::{optimize_module, CodeGenContext, ObjectFile},
    ir::{file::gen_file_ir, file_group::gen_file_group_ir, type_table::TypeTable},
    CodeGenDatabase, ModuleGroupId,
};

/// Whether `target` is a WebAssembly target, built by this module rather
/// than as a `.codiralib`.
pub fn is_wasm_target(target: &codira_target::spec::Target) -> bool {
    target.arch.starts_with("wasm")
}

/// The file extension of a built WebAssembly module.
pub const EXTENSION: &str = "wasm";

/// Builds the WebAssembly module for `group`, or explains why it cannot be
/// one.
pub(crate) fn build_wasm_module(
    db: &dyn CodeGenDatabase,
    group: ModuleGroupId,
) -> Result<NamedTempFile, String> {
    let context = Context::create();
    let code_gen = CodeGenContext::new(&context, db);
    let partition = db.module_partition();
    let module_group = &partition[group];

    let exports: Vec<codira_hir::Function> = module_group
        .iter()
        .flat_map(|module| module.all_functions(db))
        .filter(|f| f.export_abi(db).as_deref() == Some("C"))
        .collect();
    let target = db.target();
    let refuse = |reason: String| {
        format!(
            "cannot build module `{}` for {}: {reason}",
            module_group.name, target.llvm_target
        )
    };
    if exports.is_empty() {
        return Err(refuse(
            "it exports nothing. A WebAssembly module exposes the functions marked \
             `@export(\"C\")`; mark the ones its host should call"
                .to_owned(),
        ));
    }
    for function in &exports {
        let signature = function
            .ty(db)
            .callable_sig(db)
            .expect("a function has a signature");
        let scalar = |ty: &codira_hir::Ty| {
            matches!(
                ty.interned(),
                TyKind::Int(_) | TyKind::Float(_) | TyKind::Bool
            ) || ty.is_empty()
        };
        if !signature.params().iter().all(scalar) || !scalar(signature.ret()) {
            return Err(refuse(format!(
                "`{}` takes or returns a value that is not a scalar; a freestanding module \
                 passes only integers, floats and booleans across its boundary",
                function.name(db)
            )));
        }
    }
    let externs: Vec<String> = module_group
        .iter()
        .flat_map(|module| module.all_functions(db))
        .filter(|f| f.is_extern(db))
        .map(|f| format!("`{}`", f.name(db)))
        .collect();
    if !externs.is_empty() {
        return Err(refuse(format!(
            "it declares extern functions ({}), which a module that imports nothing \
             cannot call",
            externs.join(", ")
        )));
    }

    let group_ir = gen_file_group_ir(&code_gen, module_group);
    let mut needs = Vec::new();
    let dispatched: Vec<String> = group_ir
        .dispatch_table
        .entries()
        .iter()
        .map(|entry| format!("`{}`", entry.prototype.name))
        .collect();
    if !dispatched.is_empty() {
        needs.push(format!(
            "calls through the runtime's dispatch table ({})",
            dispatched.join(", ")
        ));
    }
    if group_ir.allocator_handle_type.is_some() {
        needs.push("allocates garbage-collected memory".to_owned());
    }

    let file_ir = gen_file_ir(&code_gen, &group_ir, module_group);
    let module = code_gen.create_module(&module_group.name);
    module
        .link_in_module(group_ir.llvm_module.clone())
        .map_err(|e| refuse(e.to_string()))?;
    module
        .link_in_module(file_ir.llvm_module.clone())
        .map_err(|e| refuse(e.to_string()))?;

    // The type table is built for every group (it describes the types the
    // reflection data names), but only allocation reads it at run time. What
    // decides is whether the code that was emitted *uses* it: a native
    // runtime fills it in at load time, and in a module it would stay zero.
    if module
        .get_global(TypeTable::NAME)
        .is_some_and(|table| table.as_pointer_value().get_first_use().is_some())
    {
        needs.push("reads runtime type information".to_owned());
    }
    if !needs.is_empty() {
        return Err(refuse(format!(
            "the code {}, and a WebAssembly module has no Codira runtime to provide it. \
             Only self-contained functions over scalars can be compiled for this target \
             today",
            needs.join("; it ")
        )));
    }
    optimize_module(
        &module,
        &code_gen.target_machine,
        code_gen.optimization_level,
    );

    let object = ObjectFile::new(&target, &code_gen.target_machine, &module)
        .map_err(|e| refuse(e.to_string()))?;
    let file = NamedTempFile::new().map_err(|e| refuse(e.to_string()))?;
    let symbols: Vec<String> = exports.iter().map(|f| f.name(db).to_string()).collect();
    object
        .into_shared_object(file.path(), &symbols, None)
        .map_err(|e| refuse(format!("linking failed: {e}")))?;
    Ok(file)
}

#[cfg(test)]
mod tests {
    use codira_hir::HirDatabase;
    use codira_target::spec::Target;

    use crate::{mock::MockDatabase, CodeGenDatabase, EidosOptions};

    /// Builds `source` for `triple` the way `codira build --target` does,
    /// returning the module's bytes or the reason it was refused.
    fn build(source: &str, triple: &str, eidos: EidosOptions) -> Result<Vec<u8>, String> {
        let (mut db, file_id) = MockDatabase::with_single_file(source);
        db.set_target(Target::search(triple).expect("a built-in target"));
        db.set_eidos_options(eidos);
        let group = db
            .module_partition()
            .group_for_file(file_id)
            .expect("a module group for the file");
        let assembly = db.target_assembly(group);
        if let Some(reason) = assembly.error() {
            return Err(reason.to_owned());
        }
        assert_eq!(assembly.extension(), "wasm");
        Ok(std::fs::read(assembly.path()).expect("the module was written"))
    }

    /// Instantiates a module in an independent engine (wasmi, a WebAssembly
    /// interpreter), with *no* imports: a module that needed any would fail
    /// here.
    fn instantiate(bytes: &[u8]) -> (wasmi::Store<()>, wasmi::Instance) {
        let engine = wasmi::Engine::default();
        let module = wasmi::Module::new(&engine, bytes).expect("a valid WebAssembly module");
        assert_eq!(
            module.imports().len(),
            0,
            "a freestanding module imports nothing"
        );
        let mut store = wasmi::Store::new(&engine, ());
        let instance = wasmi::Linker::<()>::new(&engine)
            .instantiate_and_start(&mut store, &module)
            .expect("the module instantiates with no imports");
        (store, instance)
    }

    const SOURCE: &str = r#"
@export("C")
public func add(a: i32, b: i32) -> i32 { a + b }

@export("C")
public func fib(n: i64) -> i64 {
    var a = 0
    var b = 1
    var i = 0
    while i < n {
        let t = a + b
        a = b
        b = t
        i += 1
    }
    a
}

@export("C")
public func hypot_sq(x: f64, y: f64) -> f64 { x * x + y * y }

@export("C")
public func is_even(n: i32) -> bool { n % 2 == 0 }

func helper(x: i64) -> i64 { x * 3 }

@export("C")
public func triple_plus_one(x: i64) -> i64 { helper(x) + 1 }
"#;

    fn check_exports(bytes: &[u8]) {
        let (mut store, instance) = instantiate(bytes);
        let add = instance
            .get_typed_func::<(i32, i32), i32>(&store, "add")
            .unwrap();
        assert_eq!(add.call(&mut store, (2, 40)).unwrap(), 42);
        assert_eq!(add.call(&mut store, (i32::MAX, 1)).unwrap(), i32::MIN);

        let fib = instance.get_typed_func::<i64, i64>(&store, "fib").unwrap();
        let expected = [0i64, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55];
        for (n, want) in expected.iter().enumerate() {
            assert_eq!(fib.call(&mut store, n as i64).unwrap(), *want);
        }
        assert_eq!(fib.call(&mut store, 90).unwrap(), 2_880_067_194_370_816_120);

        let hypot = instance
            .get_typed_func::<(f64, f64), f64>(&store, "hypot_sq")
            .unwrap();
        assert_eq!(hypot.call(&mut store, (3.0, 4.0)).unwrap(), 25.0);

        // `bool` crosses the boundary as an i32, as in C.
        let even = instance
            .get_typed_func::<i32, i32>(&store, "is_even")
            .unwrap();
        assert_eq!(even.call(&mut store, 10).unwrap(), 1);
        assert_eq!(even.call(&mut store, 7).unwrap(), 0);

        let triple = instance
            .get_typed_func::<i64, i64>(&store, "triple_plus_one")
            .unwrap();
        assert_eq!(triple.call(&mut store, 14).unwrap(), 43);

        // Only the exported functions are exported (and the memory LLD
        // creates): `helper` is not.
        assert!(instance.get_func(&store, "helper").is_none());
    }

    #[test]
    fn exported_functions_run_in_an_independent_engine() {
        for triple in ["wasm32-unknown-unknown", "wasm32-wasip1"] {
            let bytes = build(SOURCE, triple, EidosOptions::default())
                .unwrap_or_else(|e| panic!("{triple}: {e}"));
            assert_eq!(&bytes[..4], b"\0asm", "{triple}");
            check_exports(&bytes);
        }
    }

    #[test]
    fn dev_and_release_modules_compute_the_same_results() {
        let dev = build(
            SOURCE,
            "wasm32-unknown-unknown",
            EidosOptions::default().with_mode(crate::BuildMode::Dev),
        )
        .unwrap();
        check_exports(&dev);
    }

    #[test]
    fn code_that_needs_the_runtime_is_refused_with_the_reason() {
        // A class instance is a GC allocation through the runtime's
        // dispatch table; a module has no runtime to provide either.
        let error = build(
            r#"
class Counter {
    var count: i64
}

@export("C")
public func make() -> i64 {
    let c = Counter { count: 3 }
    c.count
}
"#,
            "wasm32-unknown-unknown",
            EidosOptions::default(),
        )
        .unwrap_err();
        assert!(error.contains("no Codira runtime"), "{error}");
        assert!(error.contains("wasm32-unknown-unknown"), "{error}");
    }

    #[test]
    fn a_module_must_export_something_and_only_scalars() {
        let nothing = build(
            "public func f() -> i32 { 1 }",
            "wasm32-unknown-unknown",
            EidosOptions::default(),
        )
        .unwrap_err();
        assert!(nothing.contains("exports nothing"), "{nothing}");

        let tuple = build(
            "@export(\"C\")\npublic func pair() -> (i32, i32) { (1, 2) }",
            "wasm32-unknown-unknown",
            EidosOptions::default(),
        )
        .unwrap_err();
        assert!(tuple.contains("not a scalar"), "{tuple}");
    }
}
