
# Ahead Of Time Compilation

This document describes Ahead Of Time compilation in Ark VM.

## File format

AOT file has `.an` extension and it is a valid ELF file.

### ELF layout

At the current moment it has following segments and sections layout:

| Segment | Flags | Sections |
|---------|-------|----------|
| LAOD    | R     | .hash .dynstr .dynsym .aot |
| LAOD    | R-W   | .aot_got |
| LAOD    | R-E   | .text    |
| DYNAMIC | R-W   | .dynamic |
| LAOD    | R-W   | .dynamic |

- `.hash`, `.dynstr`, `.dynsym`, `.dynamic` - Standard ELF sections.
- `.text` - Contains compiled native code. Code of each compiled method is placed sequentially into this section with
special alignment, mostly it is 16 bytes (see `CODE_ALIGNMENT` in `libpandabase/utils/arch.h` file).
- `.aot_got` - Contains table of the runtime's entrypoint handlers. Must be placed right before `.text` section.
- `.aot` - Contains all information that describes AOT structure. All used structures can be found at
`compiler/aot/aot_headers.h`.

To access to the `.text` and `.aot` sections from AOT loader following symbols are defined:
- `code` - points to the start of `.text` section
- `code_end` - points to the end of `.text` section
- `aot` - points to the start of `.aot` section
- `aot_end` - points to the end of `.aot` section

### AOT headers

`.aot` section contains combination of the following structures:
- `AotHeader` - This header describes main information, such as version, file name string, command line string,
offsets to other headers, etc.
- `PandaFileHeader` - Describes corresponding panda file.
- `ClassHeader` - Describes single class.
- `MethodHeader` - Describe single method.

More details about AOT headers you can find in `<src>/compiler/aot/aot_headers.h`.

Classes and methods are uniquely identified (inside one file) by their panda file id (`panda_file::File::EntityId`),
therefore compiled Ark bytecode file must be bit by bit equal to the file loaded in runtime.

## Special `.aot_got` section

There is one strict rule: `.text` section must be placed right after `.aot_got` section, and this special section is
filled with appropriate data during AOT file loading at runtime. This data allows compiler's codegen to generate
runtime calls and work with [PLT](../compiler/docs/plt.md) resolvers.

```
========= .aot_got ========
; PLT-GOT table, see the PLT link above
 . . .
; Three PLT resolvers addresses
-NN-24: PLT InitClass Resolver
-NN-16: PLT Class Resolver
-NN-08: PLT VirtualCall Resolver
; start of entrypoint table
-NN: address of handler 0, NN = N * 8
 . . .
-16: address of handler N-1
-08: address of handler N
========== .text ==========
00:
 . . .
```

## Runtime calls

Knowing offset to the `.aot_got` table, codegen calculates offset to the corresponding entrypoint and make load
instruction from this address. Example for arm64:

```
========= .aot_got ========
 . . .
; start of entrypoint table
-NN: address of handler 0, NN = N * 8
 . . .
-16: address of handler N-1
-08: address of handler N <-----
========== .text ==========    |
00:                            |
 . . .  ; First funcion header |
; Function executable code     |
48:                            |
 . . .                         |
80: adr x30, #-88 -------------- ; Put to the x30 address of last entry in the table
84: ldr x30, [x30]               ; Load address of the entrypoint handler
88: blr x30                      ; Jump to the handler
 . . .
```

<sup>*</sup> Aarch64 `adr` instruction has restriction to the maximum offset, thus in case of big offset `adrp`
instruction is used.

## PLT Resolvers

In AOT mode for `CallStatic`, `CallVirtual`, and opcodes related to `Class` resolving there exists a special way
to cache pointers which are recieved from runtime calls - PLT resolvers. They are described in a separate [doc](../compiler/docs/plt.md).

## String resolution

AOT-compiled code may use special PLT-slots to load resolved string without runtime calls.
Refer to [doc](../compiler/docs/aot_resolve_string.md) for details.

## Usage

#### Compilation

`ark_aot` tool aims to compile input panda files into the single AOT file that can be consumed by
Panda runtime.

ark_aot has following options:

- `--panda-files` - list of input panda files to be compiled
- `--output` - path to the output AOT file (default is `out.an`)
- `--location` - path where panda files are actually stored in the device
- `--arch` - target architecture: arm, arm64, x86, x86_64 (default is arm64)

Paoc uses Panda runtime inside, thus, runtime's options are also may be specified. If paoc is ran not from the build
directory then path to `arkstdlib.abc` should be specified via `--boot-panda-files` option.

Additional information could be found [here](../compiler/docs/paoc.md).

#### AOT in Panda

To pass AOT file to the Panda `--aot-file` option should be specified:

`ark --aot-file file.an file.abc _GLOBAL::main`

- `--panda-files` parameter specifies list of `ark_aot` or `ark` necessary input panda files which is not within `--boot-panda-files`

Example of usage:

`ark_aot --panda-files=file1.abc:file2.abc --output file.an`

Panda file shall be passed with same name as it was compiled by ark_aot, otherwise AOT loader won't find the file because
of different names. To avoid this restriction `--location` ark_aot's option may be used.

Example:
```
Good:
    ark_aot --panda-files file.abc --output file.an
    ark --aot-file file.an file.abc _GLOBAL::main

Bad ("file.abc" != "/local/data/tmp/file.abc"):
    ark_aot --panda-files file.abc --output file.an
    ark --aot-file file.an /local/data/tmp/file.abc _GLOBAL::main

Solution:
    ark_aot --panda-files file.abc --output file.an --location /local/data/tmp
    ark --aot-file file.an /local/data/tmp/file.abc _GLOBAL::main
```

#### AOT debugging

There is tool, named `ark_aotdump`, that aims to dump content of AOT files in yaml format. It can print compiled code
as disassembly(`--show-code disasm`) or in base64 binary(`--show-code binary`)

Example of usage:

`ark_aotdump --show-code disasm file.an`

Sometimes it is interesting to see what code had generated for specific IR instruction, to do so there is an option
`--compiler-disasm-dump` that dumps disassembler, IR instruction and all generated native code for it after each compiler optimization pass.
Disassembler creates files with following name format `disasm-<METHOD_NAME>.txt`. IR dumps are written into files with name `<METHOD_NAME>.ir`
 in `ir_dump` folder if option `--compiler-dump-folder` is not set. Also option `--compiler-dump-bytecode`
enables printing byte code instruction after ir instruction in dump ir. There is one more option `--compiler-dump-final` which enables
to dump ir only after last (codegen) pass. All of these options are used at compile time.

Example of usage:

`ark_aot --compiler-disasm-dump --panda-files file.abc --output file.an`

`ark_disasm` prints full IR disassembler.

Example of usage:

`ark_disasm <input panda-bytecode file> <output file>`
