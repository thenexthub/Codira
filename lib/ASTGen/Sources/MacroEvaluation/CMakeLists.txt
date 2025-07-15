add_pure_language_host_library(languageMacroEvaluation STATIC CXX_INTEROP
  Macros.code
  PluginHost.code
  SourceManager.code

  DEPENDENCIES
    languageAST
  LANGUAGE_DEPENDENCIES
    _CompilerCodiraCompilerPluginMessageHandling
    _CompilerCodiraDiagnostics
    _CompilerCodiraOperators
    _CompilerCodiraParser
    _CompilerCodiraSyntax
    _CompilerCodiraSyntaxMacroExpansion
    languageASTGen
)
