include(TableGen)

# This needs to be a macro since tablegen (which is a function) needs to set
# variables in its parent scope.
macro(language_tablegen)
  tablegen(LLVM ${ARGN})
endmacro()

# This needs to be a macro since add_public_tablegen_target (which is a
# function) needs to set variables in its parent scope.
macro(language_add_public_tablegen_target target)
  add_public_tablegen_target(${target})
endmacro()
