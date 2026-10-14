//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
use std::{io::Write, path::Path};

use codira_target::spec;
use inkwell::targets::{FileType, TargetMachine};
use tempfile::NamedTempFile;

use crate::{code_gen::CodeGenerationError, linker};

pub struct ObjectFile {
    target: spec::Target,
    obj_file: NamedTempFile,
    /// Whether the linked shared object keeps a symbol table with every
    /// function in it; see [`ObjectFile::with_symbol_table`].
    symbol_table: bool,
}

impl ObjectFile {
    /// Constructs a new object file from the specified `module` for `target`
    pub fn new(
        target: &spec::Target,
        target_machine: &TargetMachine,
        module: &inkwell::module::Module<'_>,
    ) -> Result<Self, anyhow::Error> {
        let obj = target_machine
            .write_to_memory_buffer(module, FileType::Object)
            .map_err(|e| CodeGenerationError::MachineCodeError(e.to_string()))?;

        let mut obj_file = tempfile::NamedTempFile::new()
            .map_err(CodeGenerationError::CouldNotCreateObjectFile)?;
        obj_file
            .write_all(obj.as_slice())
            .map_err(CodeGenerationError::CouldNotCreateObjectFile)?;

        Ok(Self {
            target: target.clone(),
            obj_file,
            symbol_table: false,
        })
    }

    /// Makes the linked shared object keep a symbol table naming every
    /// function, local ones included (`codira build --certify`). ELF and
    /// Mach-O linkers keep one by default; see
    /// `linker::Linker::keep_symbol_table`.
    pub fn with_symbol_table(mut self, enabled: bool) -> Self {
        self.symbol_table = enabled;
        self
    }

    /// Returns where the object file was written.
    #[cfg(test)]
    pub fn path(&self) -> &Path {
        self.obj_file.path()
    }

    /// Links the object file into a shared object.
    ///
    /// `exported_symbols` names the `@export("C")` functions that must be
    /// reachable from outside the assembly -- see
    /// `linker::Linker::build_shared_object` for why only some platforms
    /// need them spelled out.
    pub fn into_shared_object(
        self,
        output_path: &Path,
        exported_symbols: &[String],
        native: Option<&crate::NativeInputs>,
    ) -> Result<(), anyhow::Error> {
        // Construct a linker for the target
        let mut linker = linker::create_with_target(&self.target);
        linker.add_object(self.obj_file.path())?;
        if let Some(native) = native {
            linker.add_native(native)?;
        }

        if self.symbol_table {
            linker.keep_symbol_table();
        }

        // Link the object
        linker.build_shared_object(output_path, exported_symbols)?;
        linker.finalize()?;

        Ok(())
    }
}
