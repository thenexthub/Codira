add_gyb_target(generated_sourcekit_uids
  ProtocolUIDs.def.gyb
  DEPENDS
    "${LANGUAGE_SOURCE_DIR}/utils/gyb_sourcekit_support/__init__.py"
    "${LANGUAGE_SOURCE_DIR}/utils/gyb_sourcekit_support/UIDs.py")
