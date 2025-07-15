add_pure_language_host_library(languageIDEUtilsBridging CXX_INTEROP
  NameMatcherBridging.code

  DEPENDENCIES
    languageAST
  LANGUAGE_DEPENDENCIES
    _CompilerCodiraIDEUtils
    _CompilerCodiraSyntax
    _CompilerCodiraIDEUtils
    languageASTGen
)
