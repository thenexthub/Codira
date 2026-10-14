//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
use std::collections::BTreeMap;

use codira_hir::{HasVisibility, ModuleDef};
use inkwell::{module::Module, types::PointerType, values::UnnamedAddress, AddressSpace};
use rustc_hash::FxHashSet;

use super::{
    closures::FnValueTargets,
    dispatch_table::{DispatchTable, DispatchTableBuilder},
    intrinsics,
    type_table::{TypeTable, TypeTableBuilder},
};
use crate::{
    code_gen::CodeGenContext,
    module_group::ModuleGroup,
    value::{IrTypeContext, IrValueContext},
};

/// The IR generated for a group of files. It is used to generate IR for all of
/// the group's files and the resulting `Assembly`'s symbols.
#[derive(Debug, PartialEq, Eq)]
pub struct FileGroupIr<'ink> {
    /// The LLVM module that contains the IR
    pub(crate) llvm_module: Module<'ink>,
    /// The dispatch table
    pub(crate) dispatch_table: DispatchTable<'ink>,
    /// The type table
    pub(crate) type_table: TypeTable<'ink>,
    /// The allocator handle, if it exists
    pub(crate) allocator_handle_type: Option<PointerType<'ink>>,
    /// The type of the `fuelCounter` global, if the group is fuel-instrumented
    pub(crate) fuel_counter_type: Option<PointerType<'ink>>,
    /// The modules that contain code that was referenced from this group of
    /// modules
    pub(crate) referenced_modules: FxHashSet<codira_hir::Module>,
}

/// Generates IR that is shared among the group's files.
pub(crate) fn gen_file_group_ir<'ink>(
    code_gen: &CodeGenContext<'_, 'ink>,
    module_group: &ModuleGroup,
) -> FileGroupIr<'ink> {
    let llvm_module = code_gen.context.create_module("group_name");

    // Use a `BTreeMap` to guarantee deterministically ordered output.
    let mut intrinsics_map = BTreeMap::new();
    let mut needs_alloc = false;

    // Collect all intrinsic functions, wrapper function, and generate struct
    // declarations.
    // See `file::gen_file_ir`: `all_functions` covers `extend` block methods
    // too, which `declarations` alone would miss.
    for f in module_group
        .iter()
        .flat_map(|module| module.all_functions(code_gen.db))
    {
        if f.is_extern(code_gen.db) {
            // TODO: Extern types for functions?
            continue;
        }

        intrinsics::collect_fn_body(
            code_gen.context,
            code_gen.target_machine.get_target_data(),
            code_gen.db,
            &mut intrinsics_map,
            &mut needs_alloc,
            &f.body(code_gen.db),
            &f.infer(code_gen.db),
        );

        let fn_sig = f.ty(code_gen.db).callable_sig(code_gen.db).unwrap();
        if f.visibility(code_gen.db).is_externally_visible() && !fn_sig.marshallable(code_gen.db) {
            intrinsics::collect_wrapper_body(
                code_gen.context,
                code_gen.target_machine.get_target_data(),
                &mut intrinsics_map,
                &mut needs_alloc,
            );
        }
    }

    // Fuel-instrumented code calls back into the runtime when it runs dry.
    let fuel = code_gen.fuel;
    if fuel {
        intrinsics::collect_fuel(
            code_gen.context,
            code_gen.target_machine.get_target_data(),
            &mut intrinsics_map,
        );
    }

    // The closures and named functions the group turns into function values
    // (see `ir::closures`). A thunk may box value-struct arguments for a
    // call through the dispatch table, which allocates.
    let fn_values = FnValueTargets::collect(code_gen.db, module_group);
    if !fn_values.targets().is_empty() {
        intrinsics::collect_wrapper_body(
            code_gen.context,
            code_gen.target_machine.get_target_data(),
            &mut intrinsics_map,
            &mut needs_alloc,
        );
    }

    // Collect all exposed functions' bodies.
    let mut dispatch_table_builder = DispatchTableBuilder::new(
        code_gen.context,
        code_gen.target_machine.get_target_data(),
        code_gen.db,
        &llvm_module,
        &intrinsics_map,
        &code_gen.hir_types,
        module_group,
    );
    for f in module_group
        .iter()
        .flat_map(|module| module.all_functions(code_gen.db))
    {
        // Find all functions that must be present in the dispatch table
        if !f.is_extern(code_gen.db) {
            let body = f.body(code_gen.db);
            let infer = f.infer(code_gen.db);
            dispatch_table_builder.collect_body(&body, &infer);
        }
    }

    dispatch_table_builder.collect_fn_values(fn_values);
    let (dispatch_table, referenced_modules) = dispatch_table_builder.build();

    let target_data = code_gen.target_machine.get_target_data();
    let type_context = IrTypeContext {
        context: code_gen.context,
        target_data: &target_data,
        struct_types: &code_gen.rust_types,
    };
    let value_context = IrValueContext {
        type_context: &type_context,
        context: code_gen.context,
        module: &llvm_module,
    };
    let mut type_table_builder = TypeTableBuilder::new(
        code_gen.db,
        &value_context,
        intrinsics_map.keys(),
        &dispatch_table,
        &code_gen.hir_types,
        module_group,
    );

    // Collect all used types
    for def in module_group
        .iter()
        .flat_map(|module| module.declarations(code_gen.db))
    {
        match def {
            ModuleDef::Struct(s) => {
                type_table_builder.collect_struct(s);
            }
            ModuleDef::Function(_)
            | ModuleDef::PrimitiveType(_)
            | ModuleDef::TypeAlias(_)
            | ModuleDef::Effect(_)
            | ModuleDef::Module(_) => (),
        }
    }
    for f in module_group
        .iter()
        .flat_map(|module| module.all_functions(code_gen.db))
    {
        type_table_builder.collect_fn(f);
    }
    type_table_builder.collect_fn_values(dispatch_table.fn_values());

    let type_table = type_table_builder.build();

    // Create the allocator handle global value
    let allocator_handle_type = if needs_alloc {
        let allocator_handle_type = code_gen.context.ptr_type(AddressSpace::default());
        let global = llvm_module.add_global(allocator_handle_type, None, "allocatorHandle");
        global.set_initializer(&allocator_handle_type.const_null());
        global.set_unnamed_address(UnnamedAddress::Global);
        Some(allocator_handle_type)
    } else {
        None
    };

    // The fuel counter: a pointer the runtime sets through
    // `set_fuel_counter`. Until it does, it points at a private counter
    // holding `i64::MAX`, so an instrumented library loaded by a host that
    // never sets one runs as if fuel were unlimited.
    let fuel_counter_type = if fuel {
        let counter_type = code_gen.context.ptr_type(AddressSpace::default());
        let fallback = llvm_module.add_global(code_gen.context.i64_type(), None, "fuelFallback");
        fallback.set_initializer(
            &code_gen
                .context
                .i64_type()
                .const_int(i64::MAX as u64, false),
        );
        fallback.set_linkage(inkwell::module::Linkage::Private);
        let global = llvm_module.add_global(counter_type, None, "fuelCounter");
        global.set_initializer(&fallback.as_pointer_value());
        Some(counter_type)
    } else {
        None
    };

    FileGroupIr {
        llvm_module,
        dispatch_table,
        type_table,
        allocator_handle_type,
        fuel_counter_type,
        referenced_modules,
    }
}
