//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
use std::{cell::OnceCell, io::Write, path::Path};

use codira_target::spec;
use inkwell::targets::{FileType, TargetMachine};
use tempfile::NamedTempFile;

use crate::{code_gen::CodeGenerationError, image_link, linker};

pub struct ObjectFile {
    target: spec::Target,
    /// The object file as LLVM wrote it. The direct image writer reads it
    /// from here; only a link with LLD needs it on disk.
    obj: Vec<u8>,
    obj_file: OnceCell<NamedTempFile>,
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

        Ok(Self {
            target: target.clone(),
            obj: obj.as_slice().to_vec(),
            obj_file: OnceCell::new(),
            symbol_table: false,
        })
    }

    /// Makes the linked shared object keep a symbol table naming every
    /// function, local ones included (`codira build --certify`). ELF and
    /// Mach-O linkers keep one by default; see
    /// `linker::Linker::keep_symbol_table`. The direct image writer does not
    /// write one, so it declines such a link.
    pub fn with_symbol_table(mut self, enabled: bool) -> Self {
        self.symbol_table = enabled;
        self
    }

    /// Where the object file is on disk, writing it there first if this is
    /// the first time it is needed.
    fn on_disk(&self) -> Result<&Path, anyhow::Error> {
        if self.obj_file.get().is_none() {
            let mut file = tempfile::NamedTempFile::new()
                .map_err(CodeGenerationError::CouldNotCreateObjectFile)?;
            file.write_all(&self.obj)
                .map_err(CodeGenerationError::CouldNotCreateObjectFile)?;
            // Single-threaded: nothing else can have set it since the check.
            let _ = self.obj_file.set(file);
        }
        Ok(self
            .obj_file
            .get()
            .expect("the object file was just written")
            .path())
    }

    /// Returns where the object file was written.
    #[cfg(test)]
    pub fn path(&self) -> &Path {
        self.on_disk().expect("object file could be written")
    }

    /// Links the object file into a shared object.
    ///
    /// `exported_symbols` names the `@export("C")` functions that must be
    /// reachable from outside the assembly -- see
    /// `linker::Linker::build_shared_object` for why only some platforms
    /// need them spelled out.
    ///
    /// On `x86_64-pc-windows-msvc` the image is written directly by
    /// `codira_image` when it can be, and by LLD otherwise (see
    /// `image_link`).
    pub fn into_shared_object(
        self,
        output_path: &Path,
        exported_symbols: &[String],
        native: Option<&crate::NativeInputs>,
    ) -> Result<(), anyhow::Error> {
        image_link::dump_object(&self.obj, exported_symbols);
        let direct = if self.symbol_table {
            image_link::decline_without_trying(
                &self.target,
                "symbol-table",
                "the library has to keep a symbol table",
            );
            None
        } else {
            image_link::link_directly(
                &self.target,
                &self.obj,
                output_path,
                exported_symbols,
                native.is_some(),
            )
        };
        let Some(image) = direct else {
            return self.link_with_lld(output_path, exported_symbols, native);
        };

        // Self-check: have LLD link the same object and require the same
        // image, then use ours.
        if image_link::verification_enabled() {
            self.link_with_lld(output_path, exported_symbols, native)?;
            let lld = std::fs::read(output_path)?;
            image_link::verify_against(&image, &lld, output_path)?;
        }
        std::fs::write(output_path, &image)?;
        Ok(())
    }

    fn link_with_lld(
        &self,
        output_path: &Path,
        exported_symbols: &[String],
        native: Option<&crate::NativeInputs>,
    ) -> Result<(), anyhow::Error> {
        // Construct a linker for the target
        let mut linker = linker::create_with_target(&self.target);
        linker.add_object(self.on_disk()?)?;
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
