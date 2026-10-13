//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Native code compiled from a package's `[native]` sources (C, C++,
//!   assembly), as codegen consumes it.
//!
//! C and C++ arrive as LLVM bitcode from the same LLVM version Codira
//! compiles with, and are linked into each module that calls them *before*
//! that module is optimized. A C function is then just another function in
//! the module: LLVM inlines it into its Codira callers, constant-folds
//! through it and vectorizes across the boundary, exactly as it would inside
//! one C file. Assembly sources arrive as object files and go to the linker.
//!
//! An `extern` declaration whose name one of these defines is called
//! directly, by name, rather than through the dispatch table: the symbol is
//! inside the assembly being built, so there is nothing for the runtime to
//! look up at load time.
//!
//! Every module group is its own shared library, so a module group that
//! calls native code gets its own copy of it. Two modules that both use one C
//! file therefore do not share that file's global variables.

use std::{
    collections::BTreeSet,
    path::{Path, PathBuf},
};

use inkwell::{context::Context, module::Linkage};
use object::{BinaryFormat, Object, ObjectSymbol, SymbolKind};

/// The native code of the package being compiled.
#[derive(Debug, Clone, Default, PartialEq, Eq, Hash)]
pub struct NativeInputs {
    /// LLVM bitcode compiled from C and C++ sources.
    pub bitcode: Vec<PathBuf>,
    /// Object files assembled from assembly sources.
    pub objects: Vec<PathBuf>,
    /// Extra linker inputs: library paths, or names the platform's linker
    /// resolves.
    pub libraries: Vec<String>,
    /// Whether any source is C or C++, which then needs the C runtime.
    pub needs_c_runtime: bool,
    /// Whether any source is C++, which also needs the C++ runtime.
    pub needs_cxx_runtime: bool,
    /// The functions the bitcode and objects define with external linkage.
    pub symbols: BTreeSet<String>,
}

impl NativeInputs {
    /// Collects the symbols `bitcode` and `objects` define.
    pub fn new(
        bitcode: Vec<PathBuf>,
        objects: Vec<PathBuf>,
        libraries: Vec<String>,
        needs_c_runtime: bool,
        needs_cxx_runtime: bool,
    ) -> anyhow::Result<Self> {
        let mut symbols = BTreeSet::new();
        for path in &bitcode {
            bitcode_symbols(path, &mut symbols)?;
        }
        for path in &objects {
            object_symbols(path, &mut symbols)?;
        }
        Ok(NativeInputs {
            bitcode,
            objects,
            libraries,
            needs_c_runtime,
            needs_cxx_runtime,
            symbols,
        })
    }

    pub fn is_empty(&self) -> bool {
        self.bitcode.is_empty() && self.objects.is_empty() && self.libraries.is_empty()
    }
}

fn bitcode_symbols(path: &Path, symbols: &mut BTreeSet<String>) -> anyhow::Result<()> {
    let context = Context::create();
    let module = inkwell::module::Module::parse_bitcode_from_path(path, &context)
        .map_err(|e| anyhow::anyhow!("could not read bitcode '{}': {e}", path.display()))?;
    let mut function = module.get_first_function();
    while let Some(f) = function {
        if f.count_basic_blocks() > 0 && f.get_linkage() == Linkage::External {
            symbols.insert(f.get_name().to_string_lossy().into_owned());
        }
        function = f.get_next_function();
    }
    Ok(())
}

fn object_symbols(path: &Path, symbols: &mut BTreeSet<String>) -> anyhow::Result<()> {
    let bytes = std::fs::read(path)
        .map_err(|e| anyhow::anyhow!("could not read object '{}': {e}", path.display()))?;
    let file = object::File::parse(&*bytes)
        .map_err(|e| anyhow::anyhow!("could not parse object '{}': {e}", path.display()))?;
    // Mach-O puts an underscore in front of every C-level name.
    let mangling_prefix = if file.format() == BinaryFormat::MachO {
        "_"
    } else {
        ""
    };
    for symbol in file.symbols() {
        if !symbol.is_definition() || !symbol.is_global() || symbol.kind() != SymbolKind::Text {
            continue;
        }
        if let Ok(name) = symbol.name() {
            let name = name.strip_prefix(mangling_prefix).unwrap_or(name);
            symbols.insert(name.to_owned());
        }
    }
    Ok(())
}
