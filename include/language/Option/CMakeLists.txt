set(TOOLCHAIN_TARGET_DEFINITIONS Options.td)
language_tablegen(Options.inc -gen-opt-parser-defs)
language_add_public_tablegen_target(CodiraOptions)
