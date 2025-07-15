//===--- CodiraLookupTable.cpp - Codira Lookup Table ------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//
//
// This file implements support for Codira name lookup tables stored in Clang
// modules.
//
//===----------------------------------------------------------------------===//
#include "CodiraLookupTable.h"
#include "ImporterImpl.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsClangImporter.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/STLExtras.h"
#include "language/Basic/Version.h"
#include "language/ClangImporter/ClangImporter.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Sema.h"
#include "clang/Serialization/ASTBitCodes.h"
#include "clang/Serialization/ASTReader.h"
#include "clang/Serialization/ASTWriter.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Bitcode/BitcodeConvenience.h"
#include "toolchain/Bitstream/BitstreamReader.h"
#include "toolchain/Bitstream/BitstreamWriter.h"
#include "toolchain/Support/DJB.h"
#include "toolchain/Support/OnDiskHashTable.h"

using namespace language;
using namespace importer;
using namespace toolchain::support;

/// Determine whether the new declarations matches an existing declaration.
static bool matchesExistingDecl(clang::Decl *decl, clang::Decl *existingDecl) {
  // If the canonical declarations are equivalent, we have a match.
  if (decl->getCanonicalDecl() == existingDecl->getCanonicalDecl()) {
    return true;
  }

  return false;
}

template <typename value_type, typename CharT>
[[nodiscard]] static inline value_type readNext(const CharT *&memory) {
  return toolchain::support::endian::readNext<value_type, toolchain::endianness::little,
                                         toolchain::support::unaligned>(memory);
}

namespace {
  using StoredSingleEntry = CodiraLookupTable::StoredSingleEntry;

  class BaseNameToEntitiesTableReaderInfo;
  class GlobalsAsMembersTableReaderInfo;

  using SerializedBaseNameToEntitiesTable =
    toolchain::OnDiskIterableChainedHashTable<BaseNameToEntitiesTableReaderInfo>;

  using SerializedGlobalsAsMembersTable =
    toolchain::OnDiskIterableChainedHashTable<BaseNameToEntitiesTableReaderInfo>;

  using SerializedGlobalsAsMembersIndex =
    toolchain::OnDiskIterableChainedHashTable<GlobalsAsMembersTableReaderInfo>;
} // end anonymous namespace

namespace language {
/// Module file extension writer for the Codira lookup tables.
class CodiraLookupTableWriter : public clang::ModuleFileExtensionWriter {
  clang::ASTWriter &Writer;

  ASTContext &languageCtx;
  importer::ClangSourceBufferImporter &buffersForDiagnostics;
  const PlatformAvailability &availability;

  ClangImporter::Implementation *importerImpl;

public:
  CodiraLookupTableWriter(
      clang::ModuleFileExtension *extension, clang::ASTWriter &writer,
      ASTContext &ctx,
      importer::ClangSourceBufferImporter &buffersForDiagnostics,
      const PlatformAvailability &avail,
      ClangImporter::Implementation *importerImpl)
      : ModuleFileExtensionWriter(extension), Writer(writer), languageCtx(ctx),
        buffersForDiagnostics(buffersForDiagnostics), availability(avail),
        importerImpl(importerImpl) {}

  void writeExtensionContents(clang::Sema &sema,
                              toolchain::BitstreamWriter &stream) override;

  void populateTable(CodiraLookupTable &table, NameImporter &);

  void populateTableWithDecl(CodiraLookupTable &table,
                             NameImporter &nameImporter, clang::Decl *decl);
};

/// Module file extension reader for the Codira lookup tables.
class CodiraLookupTableReader : public clang::ModuleFileExtensionReader {
  clang::ASTReader &Reader;
  clang::serialization::ModuleFile &ModuleFile;
  std::function<void()> OnRemove;

  std::unique_ptr<SerializedBaseNameToEntitiesTable> SerializedTable;
  ArrayRef<clang::serialization::DeclID> Categories;
  std::unique_ptr<SerializedGlobalsAsMembersTable> GlobalsAsMembersTable;
  std::unique_ptr<SerializedGlobalsAsMembersIndex> GlobalsAsMembersIndex;

  CodiraLookupTableReader(clang::ModuleFileExtension *extension,
                         clang::ASTReader &reader,
                         clang::serialization::ModuleFile &moduleFile,
                         std::function<void()> onRemove,
                         std::unique_ptr<SerializedBaseNameToEntitiesTable>
                             serializedTable,
                         ArrayRef<clang::serialization::DeclID> categories,
                         std::unique_ptr<SerializedGlobalsAsMembersTable>
                             globalsAsMembersTable,
                         std::unique_ptr<SerializedGlobalsAsMembersIndex>
                            globalsAsMembersIndex)
      : ModuleFileExtensionReader(extension), Reader(reader),
        ModuleFile(moduleFile), OnRemove(onRemove),
        SerializedTable(std::move(serializedTable)), Categories(categories),
        GlobalsAsMembersTable(std::move(globalsAsMembersTable)),
        GlobalsAsMembersIndex(std::move(globalsAsMembersIndex)) {}

public:
  /// Create a new lookup table reader for the given AST reader and stream
  /// position.
  static std::unique_ptr<CodiraLookupTableReader>
  create(clang::ModuleFileExtension *extension, clang::ASTReader &reader,
         clang::serialization::ModuleFile &moduleFile,
         std::function<void()> onRemove, const toolchain::BitstreamCursor &stream);

  ~CodiraLookupTableReader() override;

  /// Retrieve the AST reader associated with this lookup table reader.
  clang::ASTReader &getASTReader() const { return Reader; }

  /// Retrieve the module file associated with this lookup table reader.
  clang::serialization::ModuleFile &getModuleFile() { return ModuleFile; }

  /// Retrieve the set of base names that are stored in the on-disk hash table.
  SmallVector<SerializedCodiraName, 4> getBaseNames();

  /// Retrieve the set of entries associated with the given base name.
  ///
  /// \returns true if we found anything, false otherwise.
  bool lookup(SerializedCodiraName baseName,
              SmallVectorImpl<CodiraLookupTable::FullTableEntry> &entries);

  /// Retrieve the declaration IDs of the categories.
  ArrayRef<clang::serialization::DeclID> categories() const {
    return Categories;
  }

  /// Retrieve the set of contexts that have globals-as-members
  /// injected into them.
  SmallVector<CodiraLookupTable::StoredContext, 4> getGlobalsAsMembersContexts();

  SmallVector<SerializedCodiraName, 4> getGlobalsAsMembersBaseNames();

  /// Retrieve the set of global declarations that are going to be
  /// imported as members into the given context.
  ///
  /// \returns true if we found anything, false otherwise.
  bool lookupGlobalsAsMembersInContext(
      CodiraLookupTable::StoredContext context,
      SmallVectorImpl<StoredSingleEntry> &entries);

  /// Retrieve the set of global declarations that are going to be imported as members under the given
  /// Codira base name.
  ///
  /// \returns true if we found anything, false otherwise.
  bool lookupGlobalsAsMembers(
      SerializedCodiraName baseName,
      SmallVectorImpl<CodiraLookupTable::FullTableEntry> &entries);
};
} // namespace language

DeclBaseName SerializedCodiraName::toDeclBaseName(ASTContext &Context) const {
  switch (Kind) {
  case DeclBaseName::Kind::Normal:
    return Context.getIdentifier(Name);
  case DeclBaseName::Kind::Subscript:
    return DeclBaseName::createSubscript();
  case DeclBaseName::Kind::Constructor:
    return DeclBaseName::createConstructor();
  case DeclBaseName::Kind::Destructor:
    return DeclBaseName::createDestructor();
  }
  toolchain_unreachable("unhandled kind");
}

bool CodiraLookupTable::contextRequiresName(ContextKind kind) {
  switch (kind) {
  case ContextKind::ObjCClass:
  case ContextKind::ObjCProtocol:
  case ContextKind::Tag:
  case ContextKind::Typedef:
    return true;

  case ContextKind::TranslationUnit:
    return false;
  }

  toolchain_unreachable("Invalid ContextKind.");
}

/// Try to translate the given Clang declaration into a context.
static std::optional<CodiraLookupTable::StoredContext>
translateDeclToContext(clang::NamedDecl *decl) {
  // Tag declaration.
  if (auto tag = dyn_cast<clang::TagDecl>(decl)) {
    if (tag->getIdentifier())
      return std::make_pair(CodiraLookupTable::ContextKind::Tag, tag->getName());
    if (auto typedefDecl = tag->getTypedefNameForAnonDecl())
      return std::make_pair(CodiraLookupTable::ContextKind::Tag,
                            typedefDecl->getName());
    if (auto enumDecl = dyn_cast<clang::EnumDecl>(tag)) {
      if (auto typedefType =
              dyn_cast<clang::TypedefType>(getUnderlyingType(enumDecl))) {
        if (importer::isUnavailableInCodira(typedefType->getDecl(), nullptr,
                                           true)) {
          return std::make_pair(CodiraLookupTable::ContextKind::Tag,
                                typedefType->getDecl()->getName());
        }
      }
    }

    return std::nullopt;
  }

  // Namespace declaration.
  if (auto namespaceDecl = dyn_cast<clang::NamespaceDecl>(decl)) {
    if (namespaceDecl->getIdentifier())
      return std::make_pair(CodiraLookupTable::ContextKind::Tag,
                            namespaceDecl->getName());
    return std::nullopt;
  }

  // Objective-C class context.
  if (auto objcClass = dyn_cast<clang::ObjCInterfaceDecl>(decl))
    return std::make_pair(CodiraLookupTable::ContextKind::ObjCClass,
                          objcClass->getName());

  // Objective-C protocol context.
  if (auto objcProtocol = dyn_cast<clang::ObjCProtocolDecl>(decl))
    return std::make_pair(CodiraLookupTable::ContextKind::ObjCProtocol,
                          objcProtocol->getName());

  // Typedefs.
  if (auto typedefName = dyn_cast<clang::TypedefNameDecl>(decl)) {
    // If this typedef is merely a restatement of a tag declaration's type,
    // return the result for that tag.
    if (auto tag = typedefName->getUnderlyingType()->getAsTagDecl())
      return translateDeclToContext(const_cast<clang::TagDecl *>(tag));

    // Otherwise, this must be a typedef mapped to a strong type.
    return std::make_pair(CodiraLookupTable::ContextKind::Typedef,
                          typedefName->getName());
  }

  return std::nullopt;
}

auto CodiraLookupTable::translateDeclContext(const clang::DeclContext *dc)
    -> std::optional<CodiraLookupTable::StoredContext> {
  // Translation unit context.
  if (dc->isTranslationUnit())
    return std::make_pair(ContextKind::TranslationUnit, StringRef());

  // Tag declaration context.
  if (auto tag = dyn_cast<clang::TagDecl>(dc))
    return translateDeclToContext(const_cast<clang::TagDecl *>(tag));

  // Namespace declaration context.
  if (auto namespaceDecl = dyn_cast<clang::NamespaceDecl>(dc))
    return translateDeclToContext(
        const_cast<clang::NamespaceDecl *>(namespaceDecl));

  // Objective-C class context.
  if (auto objcClass = dyn_cast<clang::ObjCInterfaceDecl>(dc))
    return std::make_pair(ContextKind::ObjCClass, objcClass->getName());

  // Objective-C protocol context.
  if (auto objcProtocol = dyn_cast<clang::ObjCProtocolDecl>(dc))
    return std::make_pair(ContextKind::ObjCProtocol, objcProtocol->getName());

  return std::nullopt;
}

std::optional<CodiraLookupTable::StoredContext>
CodiraLookupTable::translateContext(EffectiveClangContext context) {
  switch (context.getKind()) {
  case EffectiveClangContext::DeclContext: {
    return translateDeclContext(context.getAsDeclContext());
  }

  case EffectiveClangContext::TypedefContext:
    return std::make_pair(ContextKind::Typedef,
                          context.getTypedefName()->getName());

  case EffectiveClangContext::UnresolvedContext:
    // Resolve the context.
    if (auto decl = resolveContext(context.getUnresolvedName()))
      return translateDeclToContext(decl);

    return std::nullopt;
  }

  toolchain_unreachable("Invalid EffectiveClangContext.");
}

/// Lookup an unresolved context name and resolve it to a Clang
/// declaration context or typedef name.
clang::NamedDecl *CodiraLookupTable::resolveContext(StringRef unresolvedName) {
  // Look for a context with the given Codira name.
  for (auto entry :
       lookup(SerializedCodiraName(unresolvedName),
              std::make_pair(ContextKind::TranslationUnit, StringRef()))) {
    if (auto decl = entry.dyn_cast<clang::NamedDecl *>()) {
      if (isa<clang::TagDecl>(decl) ||
          isa<clang::ObjCInterfaceDecl>(decl) ||
          isa<clang::TypedefNameDecl>(decl))
        return decl;
    }
  }

  // FIXME: Search imported modules to resolve the context.

  return nullptr;
}

void CodiraLookupTable::addCategory(clang::ObjCCategoryDecl *category) {
  // Force deserialization to occur before appending.
  (void) categories();

  // Add the category.
  Categories.push_back(category);
}

bool CodiraLookupTable::resolveUnresolvedEntries(
    SmallVectorImpl<SingleEntry> &unresolved) {
  // Common case: nothing left to resolve.
  unresolved.clear();
  if (UnresolvedEntries.empty()) return false;

  // Reprocess each of the unresolved entries to see if it can be
  // resolved now that we're done. This occurs when a language_name'd
  // entity becomes a member of an entity that follows it in the
  // translation unit, e.g., given:
  //
  // \code
  //   typedef enum FooSomeEnumeration __attribute__((Foo.SomeEnum)) {
  //     ...
  //   } FooSomeEnumeration;
  //
  //   typedef struct Foo {
  //     
  //   } Foo;
  // \endcode
  //
  // FooSomeEnumeration belongs inside "Foo", but we haven't actually
  // seen "Foo" yet. Therefore, we will reprocess FooSomeEnumeration
  // at the end, once "Foo" is available. There are several reasons
  // this loop can execute:
  //
  // * Import-as-member places an entity inside of an another entity
  // that comes later in the translation unit. The number of
  // iterations that can be caused by this is bounded by the nesting
  // depth. (At present, that depth is limited to 2).
  //
  // * An erroneous import-as-member will cause an extra iteration at
  // the end, so that the loop can detect that nothing changed and
  // return a failure.
  while (true) {
    // Take the list of unresolved entries to process.
    auto prevNumUnresolvedEntries = UnresolvedEntries.size();
    auto currentUnresolved = std::move(UnresolvedEntries);
    UnresolvedEntries.clear();

    // Process each of the currently-unresolved entries.
    for (const auto &entry : currentUnresolved)
      addEntry(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry));

    // Are we done?
    if (UnresolvedEntries.empty()) return false;

    // If nothing changed, fail: something is unresolvable, and the
    // caller should complain.
    if (UnresolvedEntries.size() == prevNumUnresolvedEntries) {
      for (const auto &entry : UnresolvedEntries)
        unresolved.push_back(std::get<1>(entry));
      return true;
    }

    // Something got resolved, so loop again.
    assert(UnresolvedEntries.size() < prevNumUnresolvedEntries);
  }
}

/// Determine whether the entry is a global declaration that is being
/// mapped as a member of a particular type or extension thereof.
///
/// This should only return true when the entry isn't already nested
/// within a context. For example, it will return false for
/// enumerators, because those are naturally nested within the
/// enumeration declaration.
static bool isGlobalAsMember(CodiraLookupTable::SingleEntry entry,
                             CodiraLookupTable::StoredContext context) {
  switch (context.first) {
  case CodiraLookupTable::ContextKind::TranslationUnit:
    // We're not mapping this as a member of anything.
    return false;

  case CodiraLookupTable::ContextKind::Tag:
  case CodiraLookupTable::ContextKind::ObjCClass:
  case CodiraLookupTable::ContextKind::ObjCProtocol:
  case CodiraLookupTable::ContextKind::Typedef:
    // We're mapping into a type context.
    break;
  }

  // Macros are never stored within a non-translation-unit context in
  // Clang.
  if (entry.is<clang::MacroInfo *>()) return true;

  // We have a declaration.
  auto decl = entry.get<clang::NamedDecl *>();

  // Enumerators have the translation unit as their redeclaration context,
  // but members of anonymous enums are still allowed to be in the
  // global-as-member category.
  if (isa<clang::EnumConstantDecl>(decl)) {
    const auto *theEnum = cast<clang::EnumDecl>(decl->getDeclContext());
    return !theEnum->hasNameForLinkage();
  }

  // If the redeclaration context is namespace-scope, then we're
  // mapping as a member.
  return decl->getDeclContext()->getRedeclContext()->isFileContext();
}

bool CodiraLookupTable::addLocalEntry(
         SingleEntry newEntry,
         SmallVectorImpl<StoredSingleEntry> &entries) {
  // Check whether this entry matches any existing entry.
  auto decl = newEntry.dyn_cast<clang::NamedDecl *>();
  auto macro = newEntry.dyn_cast<clang::MacroInfo *>();
  auto moduleMacro = newEntry.dyn_cast<clang::ModuleMacro *>();

  for (auto &existingEntry : entries) {
    // If it matches an existing declaration, there's nothing to do.
    if (decl && existingEntry.isDeclEntry() &&
        matchesExistingDecl(decl, mapStoredDecl(existingEntry)))
      return false;

    // If a textual macro matches an existing macro, just drop the new
    // definition.
    if (macro && existingEntry.isMacroEntry()) {
      return false;
    }

    // If a module macro matches an existing macro, be a bit more discerning.
    //
    // Specifically, if the innermost explicit submodule containing the new
    // macro contains the innermost explicit submodule containing the existing
    // macro, the new one should replace the old one; if they're the same
    // module, the old one should stay in place. Otherwise, they don't share an
    // explicit module, and should be considered alternatives.
    //
    // Note that the above assumes that macro definitions are processed in
    // reverse order, i.e. the first definition seen is the last in a
    // translation unit.
    if (moduleMacro && existingEntry.isMacroEntry()) {
      SingleEntry decodedEntry = mapStoredMacro(existingEntry,
                                                /*assumeModule*/true);
      const auto *existingMacro = decodedEntry.get<clang::ModuleMacro *>();

      const clang::Module *newModule = moduleMacro->getOwningModule();
      const clang::Module *existingModule = existingMacro->getOwningModule();

      // A simple redeclaration: drop the new definition.
      if (existingModule == newModule)
        return false;

      // A broader-scoped redeclaration: drop the old definition.
      if (existingModule->isSubModuleOf(newModule)) {
        // FIXME: What if there are /multiple/ old definitions we should be
        // dropping? What if one of the earlier early exits makes us miss
        // entries later in the list that would match this?
        existingEntry = StoredSingleEntry(moduleMacro);
        return false;
      }

      // Otherwise, just allow both definitions to coexist.
    }
  }

  // Add an entry to this context.
  if (decl)
    entries.push_back(StoredSingleEntry(decl));
  else if (macro)
    entries.push_back(StoredSingleEntry(macro));
  else
    entries.push_back(StoredSingleEntry(moduleMacro));
  return true;
}

void CodiraLookupTable::addEntry(DeclName name, SingleEntry newEntry,
                                EffectiveClangContext effectiveContext) {
  assert(newEntry);

  // Translate the context.
  auto contextOpt = translateContext(effectiveContext);
  if (!contextOpt) {
    // We might be able to resolve this later.
    if (newEntry.is<clang::NamedDecl *>()) {
      UnresolvedEntries.push_back(
        std::make_tuple(name, newEntry, effectiveContext));
    }

    return;
  }

  auto updateTableWithEntry = [this](SingleEntry newEntry, StoredContext context,
                                     TableType::value_type::second_type &entries){
    for (auto &entry : entries) {
      if (entry.Context == context) {
        // We have entries for this context.
        (void)addLocalEntry(newEntry, entry.DeclsOrMacros);
        return;
      } else {
        (void)newEntry;
      }
    }

     // This is a new context for this name. Add it.
    auto decl = newEntry.dyn_cast<clang::NamedDecl *>();
    auto macro = newEntry.dyn_cast<clang::MacroInfo *>();
    auto moduleMacro = newEntry.dyn_cast<clang::ModuleMacro *>();

     FullTableEntry entry;
    entry.Context = context;
    if (decl)
      entry.DeclsOrMacros.push_back(StoredSingleEntry(decl));
    else if (macro)
      entry.DeclsOrMacros.push_back(StoredSingleEntry(macro));
    else
      entry.DeclsOrMacros.push_back(StoredSingleEntry(moduleMacro));

     entries.push_back(entry);
  };

  // If this is a global imported as a member, record is as such.
  auto context = *contextOpt;
  if (isGlobalAsMember(newEntry, context)) {
    // Populate cache from reader if necessary.
    findOrCreate(GlobalsAsMembers, name.getBaseName(),
                 [](auto &results, auto &Reader, auto Name) {
      return (void)Reader.lookupGlobalsAsMembers(Name, results);
    });
    updateTableWithEntry(newEntry, context,
                         GlobalsAsMembers[name.getBaseName()]);

    // Populate the index as well.
    auto &entries = GlobalsAsMembersIndex[context];
    (void)addLocalEntry(newEntry, entries);
  }

  // Populate cache from reader if necessary.
  findOrCreate(LookupTable, name.getBaseName(),
               [](auto &results, auto &Reader, auto Name) {
    return (void)Reader.lookup(Name, results);
  });
  updateTableWithEntry(newEntry, context, LookupTable[name.getBaseName()]);
}

CodiraLookupTable::TableType::iterator
CodiraLookupTable::findOrCreate(TableType &Table,
                               SerializedCodiraName baseName,
                               toolchain::function_ref<CacheCallback> create) {
  // If there is no base name, there is nothing to find.
  if (baseName.empty()) return Table.end();

  // Find entries for this base name.
  auto known = Table.find(baseName);

  // If we found something, we're done.
  if (known != Table.end()) return known;
  
  // If there's no reader, we've found all there is to find.
  if (!Reader) return known;

  // Lookup this base name in the module file.
  SmallVector<FullTableEntry, 2> results;
  create(results, *Reader, baseName);

  // Add an entry to the table so we don't look again.
  known = Table.insert({ std::move(baseName), std::move(results) }).first;

  return known;
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
CodiraLookupTable::lookup(SerializedCodiraName baseName,
                         std::optional<StoredContext> searchContext) {
  SmallVector<CodiraLookupTable::SingleEntry, 4> result;

  // Find the lookup table entry for this base name.
  auto known = findOrCreate(LookupTable, baseName,
                            [](auto &results, auto &Reader, auto Name) {
    return (void)Reader.lookup(Name, results);
  });
  if (known == LookupTable.end()) return result;

  // Walk each of the entries.
  for (auto &entry : known->second) {
    // If we're looking in a particular context and it doesn't match the
    // entry context, we're done.
    if (searchContext && entry.Context != *searchContext)
      continue;

    // Map each of the declarations.
    for (auto &stored : entry.DeclsOrMacros)
      if (auto entry = mapStored(stored))
        result.push_back(entry);
  }

  return result;
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
CodiraLookupTable::lookupGlobalsAsMembersImpl(
    SerializedCodiraName baseName, std::optional<StoredContext> searchContext) {
  SmallVector<CodiraLookupTable::SingleEntry, 4> result;

  // Find entries for this base name.
  auto known = findOrCreate(GlobalsAsMembers, baseName,
                            [](auto &results, auto &Reader, auto Name) {
     return (void)Reader.lookupGlobalsAsMembers(Name, results);
  });
  if (known == GlobalsAsMembers.end()) return result;


  // Walk each of the entries.
  for (auto &entry : known->second) {
    // If we're looking in a particular context and it doesn't match the
    // entry context, we're done.
    if (searchContext && entry.Context != *searchContext)
      continue;

    // Map each of the declarations.
    for (auto &stored : entry.DeclsOrMacros)
      if (auto entry = mapStored(stored))
        result.push_back(entry);
  }

  return result;
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
CodiraLookupTable::allGlobalsAsMembersInContext(StoredContext context) {
  SmallVector<CodiraLookupTable::SingleEntry, 4> result;

  // Find entries for this base name.
  auto known = GlobalsAsMembersIndex.find(context);

  // If we didn't find anything...
  if (known == GlobalsAsMembersIndex.end()) {
    // If there's no reader, we've found all there is to find.
    if (!Reader) return result;

    // Lookup this base name in the module extension file.
    SmallVector<StoredSingleEntry, 2> results;
    (void)Reader->lookupGlobalsAsMembersInContext(context, results);

    // Add an entry to the table so we don't look again.
    known = GlobalsAsMembersIndex.insert({ std::move(context),
                                           std::move(results) }).first;
  }

  // Map each of the results.
  for (auto &entry : known->second) {
    result.push_back(mapStored(entry));
  }

  return result;
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
CodiraLookupTable::lookupGlobalsAsMembers(SerializedCodiraName baseName,
                                         EffectiveClangContext searchContext) {
  // Propagate the null search context.
  if (!searchContext)
    return lookupGlobalsAsMembersImpl(baseName, std::nullopt);

  std::optional<StoredContext> storedContext = translateContext(searchContext);
  if (!storedContext) return { };

  return lookupGlobalsAsMembersImpl(baseName, *storedContext);
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
CodiraLookupTable::allGlobalsAsMembersInContext(EffectiveClangContext context) {
  if (!context) return { };

  std::optional<StoredContext> storedContext = translateContext(context);
  if (!storedContext) return { };

  return allGlobalsAsMembersInContext(*storedContext);
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
CodiraLookupTable::allGlobalsAsMembers() {
  // If we have a reader, deserialize all of the globals-as-members data.
  if (Reader) {
    for (auto context : Reader->getGlobalsAsMembersContexts()) {
      (void)allGlobalsAsMembersInContext(context);
    }
  }

  // Collect all of the keys and sort them.
  SmallVector<StoredContext, 8> contexts;
  for (const auto &globalAsMember : GlobalsAsMembersIndex) {
    contexts.push_back(globalAsMember.first);
  }
  toolchain::array_pod_sort(contexts.begin(), contexts.end());

  // Collect all of the results in order.
  SmallVector<CodiraLookupTable::SingleEntry, 4> results;
  for (const auto &context : contexts) {
    for (auto &entry : GlobalsAsMembersIndex[context])
      results.push_back(mapStored(entry));
  }
  return results;
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
CodiraLookupTable::lookup(SerializedCodiraName baseName,
                         EffectiveClangContext searchContext) {
  // Translate context.
  std::optional<StoredContext> context;
  if (searchContext) {
    context = translateContext(searchContext);
    if (!context) return { };
  }

  return lookup(baseName, context);
}

SmallVector<SerializedCodiraName, 4> CodiraLookupTable::allBaseNames() {
  // If we have a reader, enumerate its base names.
  if (Reader) return Reader->getBaseNames();

  // Otherwise, walk the lookup table.
  SmallVector<SerializedCodiraName, 4> result;
  for (const auto &entry : LookupTable) {
    result.push_back(entry.first);
  }
  return result;
}

SmallVector<clang::NamedDecl *, 4>
CodiraLookupTable::lookupObjCMembers(SerializedCodiraName baseName) {
  SmallVector<clang::NamedDecl *, 4> result;

  // Find the lookup table entry for this base name.
  auto known = findOrCreate(LookupTable, baseName,
                             [](auto &results, auto &Reader, auto Name) {
     return (void)Reader.lookup(Name, results);
   });
  if (known == LookupTable.end()) return result;

  // Walk each of the entries.
  for (auto &entry : known->second) {
    // If we're looking in a particular context and it doesn't match the
    // entry context, we're done.
    switch (entry.Context.first) {
    case ContextKind::TranslationUnit:
    case ContextKind::Tag:
      continue;

    case ContextKind::ObjCClass:
    case ContextKind::ObjCProtocol:
    case ContextKind::Typedef:
      break;
    }

    // Map each of the declarations.
    for (auto &stored : entry.DeclsOrMacros) {
      assert(stored.isDeclEntry() && "Not a declaration?");
      result.push_back(mapStoredDecl(stored));
    }
  }

  return result;
}

SmallVector<clang::NamedDecl *, 4>
CodiraLookupTable::lookupMemberOperators(SerializedCodiraName baseName) {
  SmallVector<clang::NamedDecl *, 4> result;

  // Find the lookup table entry for this base name.
  auto known = findOrCreate(LookupTable, baseName,
                            [](auto &results, auto &Reader, auto Name) {
                              return (void)Reader.lookup(Name, results);
                            });
  if (known == LookupTable.end())
    return result;

  // Walk each of the entries.
  for (auto &entry : known->second) {
    // We're only looking for C++ operators
    if (entry.Context.first != ContextKind::Tag) {
      continue;
    }

    // Map each of the declarations.
    for (auto &stored : entry.DeclsOrMacros) {
      assert(stored.isDeclEntry() && "Not a declaration?");
      result.push_back(mapStoredDecl(stored));
    }
  }

  return result;
}

ArrayRef<clang::ObjCCategoryDecl *> CodiraLookupTable::categories() {
  if (!Categories.empty() || !Reader) return Categories;

  // Map categories known to the reader.
  for (auto declID : Reader->categories()) {
    auto localID = clang::LocalDeclID::get(Reader->getASTReader(),
                                           Reader->getModuleFile(), declID);
    auto category = cast_or_null<clang::ObjCCategoryDecl>(
        Reader->getASTReader().GetLocalDecl(Reader->getModuleFile(), localID));
    if (category)
      Categories.push_back(category);
  }

  return Categories;
}

static void printName(clang::NamedDecl *named, toolchain::raw_ostream &out) {
  // If there is a name, print it.
  if (!named->getDeclName().isEmpty()) {
    // If we have an Objective-C method, print the class name along
    // with '+'/'-'.
    if (auto objcMethod = dyn_cast<clang::ObjCMethodDecl>(named)) {
      out << (objcMethod->isInstanceMethod() ? '-' : '+') << '[';
      if (auto classDecl = objcMethod->getClassInterface()) {
        classDecl->printName(out);
        out << ' ';
      } else if (auto proto = dyn_cast<clang::ObjCProtocolDecl>(
                                objcMethod->getDeclContext())) {
        proto->printName(out);
        out << ' ';
      }
      named->printName(out);
      out << ']';
      return;
    }

    // If we have an Objective-C property, print the class name along
    // with the property name.
    if (auto objcProperty = dyn_cast<clang::ObjCPropertyDecl>(named)) {
      auto dc = objcProperty->getDeclContext();
      if (auto classDecl = dyn_cast<clang::ObjCInterfaceDecl>(dc)) {
        classDecl->printName(out);
        out << '.';
      } else if (auto categoryDecl = dyn_cast<clang::ObjCCategoryDecl>(dc)) {
        categoryDecl->getClassInterface()->printName(out);
        out << '.';
      } else if (auto proto = dyn_cast<clang::ObjCProtocolDecl>(dc)) {
        proto->printName(out);
        out << '.';
      }
      named->printName(out);
      return;
    }

    named->printName(out);
    return;
  }

  // If this is an anonymous tag declaration with a typedef name, use that.
  if (auto tag = dyn_cast<clang::TagDecl>(named)) {
    if (auto typedefName = tag->getTypedefNameForAnonDecl()) {
      printName(typedefName, out);
      return;
    }
  }
}

void CodiraLookupTable::deserializeAll() {
  if (!Reader) return;

  for (auto baseName : Reader->getBaseNames()) {
    (void)lookup(baseName, std::nullopt);
  }

  for (auto baseName : Reader->getGlobalsAsMembersBaseNames()) {
    (void)lookupGlobalsAsMembersImpl(baseName, std::nullopt);
  }

  (void)categories();

  for (auto context : Reader->getGlobalsAsMembersContexts()) {
    (void)allGlobalsAsMembersInContext(context);
  }
}

/// Print a stored context to the given output stream for debugging purposes.
static void printStoredContext(CodiraLookupTable::StoredContext context,
                               toolchain::raw_ostream &out) {
  switch (context.first) {
  case CodiraLookupTable::ContextKind::TranslationUnit:
    out << "TU";
    break;

  case CodiraLookupTable::ContextKind::Tag:
  case CodiraLookupTable::ContextKind::ObjCClass:
  case CodiraLookupTable::ContextKind::ObjCProtocol:
  case CodiraLookupTable::ContextKind::Typedef:
    out << context.second;
    break;
  }
}

/// Print a stored entry (Clang macro or declaration) for debugging purposes.
static void printStoredEntry(const CodiraLookupTable *table,
                             StoredSingleEntry &entry,
                             toolchain::raw_ostream &out) {
  if (entry.isSerializationIDEntry()) {
    if (entry.isDeclEntry()) {
      toolchain::errs() << "decl ID #" << entry.getSerializationID();
    } else {
      auto moduleID = entry.getModuleID();
      if (moduleID == 0) {
        toolchain::errs() << "macro ID #" << entry.getSerializationID();
      } else {
        toolchain::errs() << "macro with name ID #"
                     << entry.getSerializationID() << "in submodule #"
                     << moduleID;
      }
    }
  } else if (entry.isMacroEntry()) {
    toolchain::errs() << "Macro";
  } else {
    auto decl = const_cast<CodiraLookupTable *>(table)->mapStoredDecl(entry);
    printName(decl, toolchain::errs());
  }
}

void CodiraLookupTable::dump() const {
  dump(toolchain::errs());
}

void CodiraLookupTable::dump(raw_ostream &os) const {
  // Dump the base name -> full table entry mappings.
  SmallVector<SerializedCodiraName, 4> baseNames;
  for (const auto &entry : LookupTable) {
    baseNames.push_back(entry.first);
  }
  toolchain::array_pod_sort(baseNames.begin(), baseNames.end());
  os << "Base name -> entry mappings:\n";
  for (auto baseName : baseNames) {
    switch (baseName.Kind) {
    case DeclBaseName::Kind::Normal:
      os << "  " << baseName.Name << ":\n";
      break;
    case DeclBaseName::Kind::Subscript:
      os << "  subscript:\n";
      break;
    case DeclBaseName::Kind::Constructor:
      os << "  init:\n";
      break;
    case DeclBaseName::Kind::Destructor:
      os << "  deinit:\n";
      break;
    }
    const auto &entries = LookupTable.find(baseName)->second;
    for (const auto &entry : entries) {
      os << "    ";
      printStoredContext(entry.Context, os);
      os << ": ";

      toolchain::interleave(
          entry.DeclsOrMacros.begin(), entry.DeclsOrMacros.end(),
          [this, &os](StoredSingleEntry entry) {
            printStoredEntry(this, entry, os);
          },
          [&os] { os << ", "; });
      os << "\n";
    }
  }

  if (!Categories.empty()) {
    os << "Categories: ";
    toolchain::interleave(
        Categories.begin(), Categories.end(),
        [&os](clang::ObjCCategoryDecl *category) {
          os << category->getClassInterface()->getName() << "("
             << category->getName() << ")";
        },
        [&os] { os << ", "; });
    os << "\n";
  } else if (Reader && !Reader->categories().empty()) {
    os << "Categories: ";
    toolchain::interleave(
        Reader->categories().begin(), Reader->categories().end(),
        [&os](clang::serialization::DeclID declID) {
          os << "decl ID #" << declID;
        },
        [&os] { os << ", "; });
    os << "\n";
  }

  if (!GlobalsAsMembersIndex.empty()) {
    os << "Globals-as-members mapping:\n";
    SmallVector<StoredContext, 4> contexts;
    for (const auto &entry : GlobalsAsMembersIndex) {
      contexts.push_back(entry.first);
    }
    toolchain::array_pod_sort(contexts.begin(), contexts.end());
    for (auto context : contexts) {
      os << "  ";
      printStoredContext(context, os);
      os << ": ";

      const auto &entries = GlobalsAsMembersIndex.find(context)->second;
      toolchain::interleave(
          entries.begin(), entries.end(),
          [this, &os](StoredSingleEntry entry) {
            printStoredEntry(this, entry, os);
          },
          [&os] { os << ", "; });
      os << "\n";
    }
  }
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------
using toolchain::BCArray;
using toolchain::BCBlob;
using toolchain::BCFixed;
using toolchain::BCGenericRecordLayout;
using toolchain::BCRecordLayout;
using toolchain::BCVBR;

namespace {
  enum RecordTypes {
    /// Record that contains the mapping from base names to entities with that
    /// name.
    BASE_NAME_TO_ENTITIES_RECORD_ID
      = clang::serialization::FIRST_EXTENSION_RECORD_ID,

    /// Record that contains the list of Objective-C category/extension IDs.
    CATEGORIES_RECORD_ID,

    /// Record that contains the mapping from contexts to the list of
    /// globals that will be injected as members into those contexts.
    GLOBALS_AS_MEMBERS_RECORD_ID,

    /// Record that contains the mapping from contexts to the list of
    /// globals that will be injected as members into those contexts.
    GLOBALS_AS_MEMBERS_INDEX_RECORD_ID,
  };

  using BaseNameToEntitiesTableRecordLayout
    = BCRecordLayout<BASE_NAME_TO_ENTITIES_RECORD_ID, BCVBR<16>, BCBlob>;

  using CategoriesRecordLayout
    = toolchain::BCRecordLayout<CATEGORIES_RECORD_ID, BCBlob>;

  using GlobalsAsMembersTableRecordLayout
    = BCRecordLayout<GLOBALS_AS_MEMBERS_RECORD_ID, BCVBR<16>, BCBlob>;

  using GlobalsAsMembersIndexRecordLayout
    = BCRecordLayout<GLOBALS_AS_MEMBERS_INDEX_RECORD_ID, BCVBR<16>, BCBlob>;

  constexpr size_t SizeOfEmittedStoredSingleEntry
    = sizeof(StoredSingleEntry::SerializationID)
      + sizeof(StoredSingleEntry::SubmoduleID);

  static void emitStoredSingleEntry(CodiraLookupTable::SingleEntry &mappedEntry,
                                    clang::ASTWriter &astWriter,
                                    endian::Writer &blobWriter) {
    StoredSingleEntry ids;

    // Construct a StoredSingleEntry with the ID(s) for `mappedEntry`.
    if (auto *decl = mappedEntry.dyn_cast<clang::NamedDecl *>()) {
      ids = StoredSingleEntry::forSerializedDecl(
                                 astWriter.getDeclID(decl).getRawValue());
    } else if (auto *macro = mappedEntry.dyn_cast<clang::MacroInfo *>()) {
      ids = StoredSingleEntry::forSerializedMacro(astWriter.getMacroID(macro));
    } else {
      auto *moduleMacro = mappedEntry.get<clang::ModuleMacro *>();
      StoredSingleEntry::SerializationID nameID =
        astWriter.getIdentifierRef(moduleMacro->getName());
      StoredSingleEntry::SubmoduleID submoduleID =
        astWriter.getLocalOrImportedSubmoduleID(moduleMacro->getOwningModule());
      ids = StoredSingleEntry::forSerializedMacro(nameID, submoduleID);
    }

    // Write it out.
    auto idsData = ids.getData();
    blobWriter.write<StoredSingleEntry::SerializationID>(idsData.first);
    blobWriter.write<StoredSingleEntry::SubmoduleID>(idsData.second);
  }

  /// Trait used to write the on-disk hash table for the base name -> entities
  /// mapping.
  class BaseNameToEntitiesTableWriterInfo {
    static_assert(sizeof(DeclBaseName::Kind) <= sizeof(uint8_t),
                  "kind serialized as uint8_t");

    CodiraLookupTable &Table;
    clang::ASTWriter &Writer;

  public:
    using key_type = SerializedCodiraName;
    using key_type_ref = key_type;
    using data_type = SmallVector<CodiraLookupTable::FullTableEntry, 2>;
    using data_type_ref = data_type &;
    using hash_value_type = uint32_t;
    using offset_type = unsigned;

    BaseNameToEntitiesTableWriterInfo(CodiraLookupTable &table,
                                      clang::ASTWriter &writer)
      : Table(table), Writer(writer)
    {
    }

    hash_value_type ComputeHash(key_type_ref key) {
      return static_cast<hash_value_type>(key.Kind) + toolchain::djbHash(key.Name);
    }

    std::pair<unsigned, unsigned> EmitKeyDataLength(raw_ostream &out,
                                                    key_type_ref key,
                                                    data_type_ref data) {
      uint32_t keyLength = sizeof(uint8_t); // For the flag of the name's kind
      if (key.Kind == DeclBaseName::Kind::Normal) {
        keyLength += key.Name.size(); // The name's length
      }
      assert(keyLength == static_cast<uint16_t>(keyLength));

      // # of entries
      uint32_t dataLength = sizeof(uint16_t);

      // Storage per entry.
      for (const auto &entry : data) {
        // Context info.
        dataLength += 1;
        if (CodiraLookupTable::contextRequiresName(entry.Context.first)) {
          dataLength += sizeof(uint16_t) + entry.Context.second.size();
        }

        // # of entries.
        dataLength += sizeof(uint16_t);

        // Actual entries.
        dataLength += (SizeOfEmittedStoredSingleEntry
                        * entry.DeclsOrMacros.size());
      }

      endian::Writer writer(out, toolchain::endianness::little);
      writer.write<uint16_t>(keyLength);
      writer.write<uint32_t>(dataLength);
      return { keyLength, dataLength };
    }

    void EmitKey(raw_ostream &out, key_type_ref key, unsigned len) {
      endian::Writer writer(out, toolchain::endianness::little);
      writer.write<uint8_t>((uint8_t)key.Kind);
      if (key.Kind == language::DeclBaseName::Kind::Normal)
        writer.OS << key.Name;
    }

    void EmitData(raw_ostream &out, key_type_ref key, data_type_ref data,
                  unsigned len) {
      endian::Writer writer(out, toolchain::endianness::little);

      // # of entries
      writer.write<uint16_t>(data.size());
      assert(data.size() == static_cast<uint16_t>(data.size()));

      bool isModule = Writer.getLangOpts().isCompilingModule();
      for (auto &fullEntry : data) {
        // Context.
        writer.write<uint8_t>(static_cast<uint8_t>(fullEntry.Context.first));
        if (CodiraLookupTable::contextRequiresName(fullEntry.Context.first)) {
          writer.write<uint16_t>(fullEntry.Context.second.size());
          out << fullEntry.Context.second;
        }

        // # of entries.
        writer.write<uint16_t>(fullEntry.DeclsOrMacros.size());

        // Write the declarations and macros.
        for (auto &entry : fullEntry.DeclsOrMacros) {
          auto mappedEntry = Table.mapStored(entry, isModule);
          emitStoredSingleEntry(mappedEntry, Writer, writer);
        }
      }
    }
  };

  /// Trait used to write the on-disk hash table for the
  /// globals-as-members mapping.
  class GlobalsAsMembersTableWriterInfo {
    CodiraLookupTable &Table;
    clang::ASTWriter &Writer;

  public:
    using key_type = std::pair<CodiraLookupTable::ContextKind, StringRef>;
    using key_type_ref = key_type;
    using data_type = SmallVector<StoredSingleEntry, 2>;
    using data_type_ref = data_type &;
    using hash_value_type = uint32_t;
    using offset_type = unsigned;

    GlobalsAsMembersTableWriterInfo(CodiraLookupTable &table,
                                    clang::ASTWriter &writer)
      : Table(table), Writer(writer)
    {
    }

    hash_value_type ComputeHash(key_type_ref key) {
      return static_cast<hash_value_type>(key.first) +
             toolchain::djbHash(key.second);
    }

    std::pair<unsigned, unsigned> EmitKeyDataLength(raw_ostream &out,
                                                    key_type_ref key,
                                                    data_type_ref data) {
      // The length of the key.
      uint32_t keyLength = 1;
      if (CodiraLookupTable::contextRequiresName(key.first))
        keyLength += key.second.size();
      assert(keyLength == static_cast<uint16_t>(keyLength));

      // # of entries
      uint32_t dataLength =
        sizeof(uint16_t) + SizeOfEmittedStoredSingleEntry * data.size();
      assert(dataLength == static_cast<uint32_t>(dataLength));

      endian::Writer writer(out, toolchain::endianness::little);
      writer.write<uint16_t>(keyLength);
      writer.write<uint32_t>(dataLength);
      return { keyLength, dataLength };
    }

    void EmitKey(raw_ostream &out, key_type_ref key, unsigned len) {
      endian::Writer writer(out, toolchain::endianness::little);
      writer.write<uint8_t>(static_cast<unsigned>(key.first) - 2);
      if (CodiraLookupTable::contextRequiresName(key.first))
        out << key.second;
    }

    void EmitData(raw_ostream &out, key_type_ref key, data_type_ref data,
                  unsigned len) {
      endian::Writer writer(out, toolchain::endianness::little);

      // # of entries
      writer.write<uint16_t>(data.size());

      // Actual entries.
      bool isModule = Writer.getLangOpts().isCompilingModule();
      for (auto &entry : data) {
        auto mappedEntry = Table.mapStored(entry, isModule);
        emitStoredSingleEntry(mappedEntry, Writer, writer);
      }
    }
  };

} // end anonymous namespace

void CodiraLookupTableWriter::writeExtensionContents(
       clang::Sema &sema,
       toolchain::BitstreamWriter &stream) {
  NameImporter nameImporter(languageCtx, availability, sema, importerImpl);

  // Populate the lookup table.
  CodiraLookupTable table(nullptr);
  populateTable(table, nameImporter);

  SmallVector<uint64_t, 64> ScratchRecord;

  // First, gather the sorted list of base names.
  SmallVector<SerializedCodiraName, 2> baseNames;
  for (const auto &entry : table.LookupTable)
    baseNames.push_back(entry.first);
  toolchain::array_pod_sort(baseNames.begin(), baseNames.end());

  // Form the mapping from base names to entities with their context.
  {
    toolchain::SmallString<4096> hashTableBlob;
    uint32_t tableOffset;
    {
      toolchain::OnDiskChainedHashTableGenerator<BaseNameToEntitiesTableWriterInfo>
        generator;
      BaseNameToEntitiesTableWriterInfo info(table, Writer);
      for (auto baseName : baseNames)
        generator.insert(baseName, table.LookupTable[baseName], info);

      toolchain::raw_svector_ostream blobStream(hashTableBlob);
      // Make sure that no bucket is at offset 0
      endian::write<uint32_t>(blobStream, 0, toolchain::endianness::little);
      tableOffset = generator.Emit(blobStream, info);
    }

    BaseNameToEntitiesTableRecordLayout layout(stream);
    layout.emit(ScratchRecord, tableOffset, hashTableBlob);
  }

  // Write the categories, if there are any.
  if (!table.Categories.empty()) {
    SmallVector<clang::serialization::DeclID, 4> categoryIDs;
    for (auto category : table.Categories) {
      categoryIDs.push_back(Writer.getDeclID(category).getRawValue());
    }

    StringRef blob(reinterpret_cast<const char *>(categoryIDs.data()),
                   categoryIDs.size() * sizeof(clang::serialization::DeclID));
    CategoriesRecordLayout layout(stream);
    layout.emit(ScratchRecord, blob);
  }

  // Write the globals-as-members table, if non-empty.
  if (!table.GlobalsAsMembers.empty()) {
    // First, gather the sorted list of base names.
    SmallVector<SerializedCodiraName, 2> baseNames;
    for (const auto &entry : table.GlobalsAsMembers)
      baseNames.push_back(entry.first);
    toolchain::array_pod_sort(baseNames.begin(), baseNames.end());

    // Form the mapping from base names to entities with their context.
    {
      toolchain::SmallString<4096> hashTableBlob;
      uint32_t tableOffset;
      {
        toolchain::OnDiskChainedHashTableGenerator<BaseNameToEntitiesTableWriterInfo>
          generator;
        BaseNameToEntitiesTableWriterInfo info(table, Writer);
        for (auto baseName : baseNames)
          generator.insert(baseName, table.GlobalsAsMembers[baseName], info);

        toolchain::raw_svector_ostream blobStream(hashTableBlob);
        // Make sure that no bucket is at offset 0
        endian::write<uint32_t>(blobStream, 0, toolchain::endianness::little);
        tableOffset = generator.Emit(blobStream, info);
      }

      GlobalsAsMembersTableRecordLayout layout(stream);
      layout.emit(ScratchRecord, tableOffset, hashTableBlob);
    }
  }

  // Write the globals-as-members index, if non-empty.
  if (!table.GlobalsAsMembersIndex.empty()) {
    // Sort the keys.
    SmallVector<CodiraLookupTable::StoredContext, 4> contexts;
    for (const auto &entry : table.GlobalsAsMembersIndex) {
      contexts.push_back(entry.first);
    }
    toolchain::array_pod_sort(contexts.begin(), contexts.end());

    // Create the on-disk hash table.
    toolchain::SmallString<4096> hashTableBlob;
    uint32_t tableOffset;
    {
      toolchain::OnDiskChainedHashTableGenerator<GlobalsAsMembersTableWriterInfo>
        generator;
      GlobalsAsMembersTableWriterInfo info(table, Writer);
      for (auto context : contexts)
        generator.insert(context, table.GlobalsAsMembersIndex[context], info);

      toolchain::raw_svector_ostream blobStream(hashTableBlob);
      // Make sure that no bucket is at offset 0
      endian::write<uint32_t>(blobStream, 0, toolchain::endianness::little);
      tableOffset = generator.Emit(blobStream, info);
    }

    GlobalsAsMembersIndexRecordLayout layout(stream);
    layout.emit(ScratchRecord, tableOffset, hashTableBlob);
  }
}

namespace {
  StoredSingleEntry readNextStoredSingleEntry(const uint8_t *&data) {
    std::pair<StoredSingleEntry::SerializationID,
              StoredSingleEntry::SubmoduleID> ids;
    ids.first = readNext<StoredSingleEntry::SerializationID>(data);
    ids.second = readNext<StoredSingleEntry::SubmoduleID>(data);
    return StoredSingleEntry(ids);
  }

  /// Used to deserialize the on-disk base name -> entities table.
  class BaseNameToEntitiesTableReaderInfo {
  public:
    using internal_key_type = SerializedCodiraName;
    using external_key_type = internal_key_type;
    using data_type = SmallVector<CodiraLookupTable::FullTableEntry, 2>;
    using hash_value_type = uint32_t;
    using offset_type = unsigned;

    internal_key_type GetInternalKey(external_key_type key) {
      return key;
    }

    external_key_type GetExternalKey(internal_key_type key) {
      return key;
    }

    hash_value_type ComputeHash(internal_key_type key) {
      return static_cast<hash_value_type>(key.Kind) + toolchain::djbHash(key.Name);
    }

    static bool EqualKey(internal_key_type lhs, internal_key_type rhs) {
      return lhs == rhs;
    }

    static std::pair<unsigned, unsigned>
    ReadKeyDataLength(const uint8_t *&data) {
      unsigned keyLength = readNext<uint16_t>(data);
      unsigned dataLength = readNext<uint32_t>(data);
      return { keyLength, dataLength };
    }

    static internal_key_type ReadKey(const uint8_t *data, unsigned length) {
      uint8_t kind = readNext<uint8_t>(data);
      switch (kind) {
      case (uint8_t)DeclBaseName::Kind::Normal: {
        StringRef str(reinterpret_cast<const char *>(data),
                      length - sizeof(uint8_t));
        return SerializedCodiraName(str);
      }
      case (uint8_t)DeclBaseName::Kind::Subscript:
        return SerializedCodiraName(DeclBaseName::Kind::Subscript);
      case (uint8_t)DeclBaseName::Kind::Constructor:
        return SerializedCodiraName(DeclBaseName::Kind::Constructor);
      case (uint8_t)DeclBaseName::Kind::Destructor:
        return SerializedCodiraName(DeclBaseName::Kind::Destructor);
      default:
        toolchain_unreachable("Unknown kind for DeclBaseName");
      }
    }

    static data_type ReadData(internal_key_type key, const uint8_t *data,
                              unsigned length) {
      data_type result;

      // # of entries.
      unsigned numEntries = readNext<uint16_t>(data);
      result.reserve(numEntries);

      // Read all of the entries.
      while (numEntries--) {
        CodiraLookupTable::FullTableEntry entry;

        // Read the context.
        entry.Context.first =
            static_cast<CodiraLookupTable::ContextKind>(readNext<uint8_t>(data));
        if (CodiraLookupTable::contextRequiresName(entry.Context.first)) {
          uint16_t length = readNext<uint16_t>(data);
          entry.Context.second = StringRef((const char *)data, length);
          data += length;
        }

        // Read the declarations and macros.
        unsigned numDeclsOrMacros = readNext<uint16_t>(data);
        while (numDeclsOrMacros--) {
          entry.DeclsOrMacros.push_back(readNextStoredSingleEntry(data));
        }

        result.push_back(entry);
      }

      return result;
    }
  };

  /// Used to deserialize the on-disk globals-as-members table.
  class GlobalsAsMembersTableReaderInfo {
  public:
    using internal_key_type = CodiraLookupTable::StoredContext;
    using external_key_type = internal_key_type;
    using data_type = SmallVector<StoredSingleEntry, 2>;
    using hash_value_type = uint32_t;
    using offset_type = unsigned;

    internal_key_type GetInternalKey(external_key_type key) {
      return key;
    }

    external_key_type GetExternalKey(internal_key_type key) {
      return key;
    }

    hash_value_type ComputeHash(internal_key_type key) {
      return static_cast<hash_value_type>(key.first) + toolchain::djbHash(key.second);
    }

    static bool EqualKey(internal_key_type lhs, internal_key_type rhs) {
      return lhs == rhs;
    }

    static std::pair<unsigned, unsigned>
    ReadKeyDataLength(const uint8_t *&data) {
      unsigned keyLength = readNext<uint16_t>(data);
      unsigned dataLength = readNext<uint32_t>(data);
      return { keyLength, dataLength };
    }

    static internal_key_type ReadKey(const uint8_t *data, unsigned length) {
      return internal_key_type(
               static_cast<CodiraLookupTable::ContextKind>(*data + 2),
               StringRef((const char *)data + 1, length - 1));
    }

    static data_type ReadData(internal_key_type key, const uint8_t *data,
                              unsigned length) {
      data_type result;

      // # of entries.
      unsigned numEntries = readNext<uint16_t>(data);
      result.reserve(numEntries);

      // Read all of the entries.
      while (numEntries--) {
        result.push_back(readNextStoredSingleEntry(data));
      }

      return result;
    }
  };

} // end anonymous namespace

clang::NamedDecl *CodiraLookupTable::mapStoredDecl(StoredSingleEntry &entry) {
  assert(entry.isDeclEntry() && "Not a declaration entry");

  // If we have an AST node here, just cast it.
  if (entry.isASTNodeEntry()) {
    return static_cast<clang::NamedDecl *>(entry.getASTNode());
  }

  // Otherwise, resolve the declaration.
  assert(Reader && "Cannot resolve the declaration without a reader");
  auto declID = entry.getSerializationID();
  auto localID = clang::LocalDeclID::get(Reader->getASTReader(),
                                         Reader->getModuleFile(), declID);
  auto decl = cast_or_null<clang::NamedDecl>(
      Reader->getASTReader().GetLocalDecl(Reader->getModuleFile(), localID));

  // Update the entry now that we've resolved the declaration.
  entry = StoredSingleEntry(decl);
  return decl;
}

static bool isPCH(CodiraLookupTableReader &reader) {
  return reader.getModuleFile().Kind == clang::serialization::MK_PCH;
}

CodiraLookupTable::SingleEntry
CodiraLookupTable::mapStoredMacro(StoredSingleEntry &entry, bool assumeModule) {
  assert(entry.isMacroEntry() && "Not a macro entry");

  // If we have an AST node here, just cast it.
  if (entry.isASTNodeEntry()) {
    if (assumeModule || (Reader && !isPCH(*Reader)))
      return static_cast<clang::ModuleMacro *>(entry.getASTNode());
    else
      return static_cast<clang::MacroInfo *>(entry.getASTNode());
  }

  // Otherwise, resolve the macro.
  assert(Reader && "Cannot resolve the macro without a reader");
  clang::ASTReader &astReader = Reader->getASTReader();

  if (!assumeModule && entry.getModuleID() == 0) {
    assert(isPCH(*Reader));
    // Not a module, and the second key is actually a macroID.
    auto macro =
        astReader.getMacro(astReader.getGlobalMacroID(Reader->getModuleFile(),
                                              entry.getSerializationID()));

    // Update the entry now that we've resolved the macro.
    entry = StoredSingleEntry(macro);
    return macro;
  }

  // FIXME: Clang should help us out here, but it doesn't. It can only give us
  // MacroInfos and not ModuleMacros.
  assert(!isPCH(*Reader));
  clang::IdentifierInfo *name =
      astReader.getLocalIdentifier(Reader->getModuleFile(), 
                                   entry.getSerializationID());
  auto submoduleID = astReader.getGlobalSubmoduleID(Reader->getModuleFile(),
                                   entry.getModuleID());
  clang::Module *submodule = astReader.getSubmodule(submoduleID);
  assert(submodule);

  clang::Preprocessor &pp = Reader->getASTReader().getPreprocessor();
  // Force the ModuleMacro to be loaded if this module is visible.
  (void)pp.getLeafModuleMacros(name);
  clang::ModuleMacro *macro = pp.getModuleMacro(submodule, name);
  // This might still be NULL if the module has been imported but not made
  // visible. We need a better answer here.
  if (macro)
    entry = StoredSingleEntry(macro);
  return macro;
}

CodiraLookupTable::SingleEntry
CodiraLookupTable::mapStored(StoredSingleEntry &entry, bool assumeModule) {
  if (entry.isDeclEntry())
    return mapStoredDecl(entry);
  return mapStoredMacro(entry, assumeModule);
}

CodiraLookupTableReader::~CodiraLookupTableReader() {
  OnRemove();
}

std::unique_ptr<CodiraLookupTableReader>
CodiraLookupTableReader::create(clang::ModuleFileExtension *extension,
                               clang::ASTReader &reader,
                               clang::serialization::ModuleFile &moduleFile,
                               std::function<void()> onRemove,
                               const toolchain::BitstreamCursor &stream)
{
  // Look for the base name -> entities table record.
  SmallVector<uint64_t, 64> scratch;
  auto cursor = stream;
  toolchain::Expected<toolchain::BitstreamEntry> maybeNext = cursor.advance();
  if (!maybeNext) {
    // FIXME this drops the error on the floor.
    consumeError(maybeNext.takeError());
    return nullptr;
  }
  toolchain::BitstreamEntry next = maybeNext.get();
  std::unique_ptr<SerializedBaseNameToEntitiesTable> serializedTable;
  std::unique_ptr<SerializedGlobalsAsMembersIndex> globalsAsMembersIndex;
  std::unique_ptr<SerializedGlobalsAsMembersTable> globalsAsMembersTable;
  ArrayRef<clang::serialization::DeclID> categories;

  while (next.Kind != toolchain::BitstreamEntry::EndBlock) {
    if (next.Kind == toolchain::BitstreamEntry::Error)
      return nullptr;

    if (next.Kind == toolchain::BitstreamEntry::SubBlock) {
      // Unknown sub-block, possibly for use by a future version of the
      // API notes format.
      if (cursor.SkipBlock())
        return nullptr;

      maybeNext = cursor.advance();
      if (!maybeNext) {
        // FIXME this drops the error on the floor.
        consumeError(maybeNext.takeError());
        return nullptr;
      }
      next = maybeNext.get();
      continue;
    }

    scratch.clear();
    StringRef blobData;
    toolchain::Expected<unsigned> maybeKind =
        cursor.readRecord(next.ID, scratch, &blobData);
    if (!maybeKind) {
      // FIXME this drops the error on the floor.
      consumeError(maybeNext.takeError());
      return nullptr;
    }
    unsigned kind = maybeKind.get();
    switch (kind) {
    case BASE_NAME_TO_ENTITIES_RECORD_ID: {
      // Already saw base name -> entities table.
      if (serializedTable)
        return nullptr;

      uint32_t tableOffset;
      BaseNameToEntitiesTableRecordLayout::readRecord(scratch, tableOffset);
      auto base = reinterpret_cast<const uint8_t *>(blobData.data());

      serializedTable.reset(
        SerializedBaseNameToEntitiesTable::Create(base + tableOffset,
                                                  base + sizeof(uint32_t),
                                                  base));
      break;
    }

    case GLOBALS_AS_MEMBERS_INDEX_RECORD_ID: {
      // Already saw globals as members index.
      if (globalsAsMembersIndex)
        return nullptr;

      uint32_t tableOffset;
      GlobalsAsMembersIndexRecordLayout::readRecord(scratch, tableOffset);
      auto base = reinterpret_cast<const uint8_t *>(blobData.data());

      globalsAsMembersIndex.reset(
        SerializedGlobalsAsMembersIndex::Create(base + tableOffset,
                                                base + sizeof(uint32_t),
                                                base));
      break;
    }

    case CATEGORIES_RECORD_ID: {
      // Already saw categories; input is malformed.
      if (!categories.empty()) return nullptr;

      auto start =
        reinterpret_cast<const clang::serialization::DeclID *>(blobData.data());
      unsigned numElements
        = blobData.size() / sizeof(clang::serialization::DeclID);
      categories = toolchain::ArrayRef(start, numElements);
      break;
    }

    case GLOBALS_AS_MEMBERS_RECORD_ID: {
      // Already saw globals-as-members table.
      if (globalsAsMembersTable)
        return nullptr;

      uint32_t tableOffset;
      GlobalsAsMembersTableRecordLayout::readRecord(scratch, tableOffset);
      auto base = reinterpret_cast<const uint8_t *>(blobData.data());

      globalsAsMembersTable.reset(
        SerializedGlobalsAsMembersTable::Create(base + tableOffset,
                                                base + sizeof(uint32_t),
                                                base));
      break;
    }

    default:
      // Unknown record, possibly for use by a future version of the
      // module format.
      break;
    }

    maybeNext = cursor.advance();
    if (!maybeNext) {
      // FIXME this drops the error on the floor.
      consumeError(maybeNext.takeError());
      return nullptr;
    }
    next = maybeNext.get();
  }

  if (!serializedTable) return nullptr;

  // Create the reader.
  // Note: This doesn't use std::make_unique because the constructor is
  // private.
  return std::unique_ptr<CodiraLookupTableReader>(
           new CodiraLookupTableReader(extension, reader, moduleFile, onRemove,
                                      std::move(serializedTable), categories,
                                      std::move(globalsAsMembersTable),
                                      std::move(globalsAsMembersIndex)));

}

SmallVector<SerializedCodiraName, 4> CodiraLookupTableReader::getBaseNames() {
  SmallVector<SerializedCodiraName, 4> results;
  for (auto key : SerializedTable->keys()) {
    results.push_back(key);
  }
  return results;
}

bool CodiraLookupTableReader::lookup(
    SerializedCodiraName baseName,
    SmallVectorImpl<CodiraLookupTable::FullTableEntry> &entries) {
  // Look for an entry with this base name.
  auto known = SerializedTable->find(baseName);
  if (known == SerializedTable->end()) return false;

  // Grab the results.
  entries = std::move(*known);
  return true;
}

SmallVector<CodiraLookupTable::StoredContext, 4>
CodiraLookupTableReader::getGlobalsAsMembersContexts() {
  SmallVector<CodiraLookupTable::StoredContext, 4> results;
  if (!GlobalsAsMembersIndex) return results;

  for (auto key : GlobalsAsMembersIndex->keys()) {
    results.push_back(key);
  }
  return results;
}

bool CodiraLookupTableReader::lookupGlobalsAsMembersInContext(
       CodiraLookupTable::StoredContext context,
       SmallVectorImpl<StoredSingleEntry> &entries) {
  if (!GlobalsAsMembersIndex) return false;

  // Look for an entry with this context name.
  auto known = GlobalsAsMembersIndex->find(context);
  if (known == GlobalsAsMembersIndex->end()) return false;

  // Grab the results.
  entries = std::move(*known);
  return true;
}

SmallVector<SerializedCodiraName, 4>
CodiraLookupTableReader::getGlobalsAsMembersBaseNames() {
  SmallVector<SerializedCodiraName, 4> results;
  if (!GlobalsAsMembersTable) return {};

  for (auto key : GlobalsAsMembersTable->keys()) {
    results.push_back(key);
  }
  return results;
}

bool CodiraLookupTableReader::lookupGlobalsAsMembers(
       SerializedCodiraName baseName,
       SmallVectorImpl<CodiraLookupTable::FullTableEntry> &entries) {
  if (!GlobalsAsMembersTable) return false;

  // Look for an entry with this context name.
  auto known = GlobalsAsMembersTable->find(baseName);
  if (known == GlobalsAsMembersTable->end()) return false;

  // Grab the results.
  entries = std::move(*known);
  return true;
}

clang::ModuleFileExtensionMetadata
CodiraNameLookupExtension::getExtensionMetadata() const {
  clang::ModuleFileExtensionMetadata metadata;
  metadata.BlockName = "language.lookup";
  metadata.MajorVersion = LANGUAGE_LOOKUP_TABLE_VERSION_MAJOR;
  metadata.MinorVersion = LANGUAGE_LOOKUP_TABLE_VERSION_MINOR;
  metadata.UserInfo =
      version::getCodiraFullVersion(languageCtx.LangOpts.EffectiveLanguageVersion);
  return metadata;
}

void
CodiraNameLookupExtension::hashExtension(ExtensionHashBuilder &HBuilder) const {
  HBuilder.add(StringRef("language.lookup"));
  HBuilder.add(LANGUAGE_LOOKUP_TABLE_VERSION_MAJOR);
  HBuilder.add(LANGUAGE_LOOKUP_TABLE_VERSION_MINOR);
  HBuilder.add(version::getCodiraFullVersion());
}

void importer::addEntryToLookupTable(CodiraLookupTable &table,
                                     clang::NamedDecl *named,
                                     NameImporter &nameImporter) {
  auto &clangContext = nameImporter.getClangContext();
  clang::PrettyStackTraceDecl trace(
      named, named->getLocation(), clangContext.getSourceManager(),
      "while adding CodiraName lookup table entries for clang declaration");

  // Determine whether this declaration is suppressed in Codira.
  if (shouldSuppressDeclImport(named))
    return;

  // Leave incomplete struct/enum/union types out of the table; Codira only
  // handles pointers to them.
  // FIXME: At some point we probably want to be importing incomplete types,
  // so that pointers to different incomplete types themselves have distinct
  // types. At that time it will be necessary to make the decision of whether
  // or not to import an incomplete type declaration based on whether it's
  // actually the struct backing a CF type:
  //
  //    typedef struct CGColor *CGColorRef;
  //
  // The best way to do this is probably to change CFDatabase.def to include
  // struct names when relevant, not just pointer names. That way we can check
  // both CFDatabase.def and the objc_bridge attribute and cover all our bases.
  if (auto *tagDecl = dyn_cast<clang::TagDecl>(named)) {
    // We add entries for ClassTemplateSpecializations that don't have
    // definition. It's possible that the decl will be instantiated by
    // CodiraDeclConverter later on. We cannot force instantiating
    // ClassTemplateSPecializations here because we're currently writing the
    // AST, so we cannot modify it.
    if (!isa<clang::ClassTemplateSpecializationDecl>(named) &&
        !tagDecl->getDefinition()) {
      return;
    }
  }
  // If we have a name to import as, add this entry to the table.
  auto currentVersion =
      ImportNameVersion::fromOptions(nameImporter.getLangOpts());
  auto failed = nameImporter.forEachDistinctImportName(
      named, currentVersion,
      [&](ImportedName importedName, ImportNameVersion version) {
        table.addEntry(importedName.getDeclName(), named,
                       importedName.getEffectiveContext());

        // Also add the subscript entry, if needed.
        if (version == currentVersion && importedName.isSubscriptAccessor()) {
          table.addEntry(DeclName(nameImporter.getContext(),
                                  DeclBaseName::createSubscript(),
                                  {Identifier()}),
                         named, importedName.getEffectiveContext());
        }

        return true;
      });
  if (failed) {
    if (auto category = dyn_cast<clang::ObjCCategoryDecl>(named)) {
      // If the category is invalid, don't add it.
      if (category->isInvalidDecl())
        return;

      table.addCategory(category);
    }
  }

  // Class template instantiations are imported lazily, however, the lookup
  // table must include their mangled name (__CxxTemplateInst...) to make it
  // possible to find these decls during deserialization. For any C++ typedef
  // that defines a name for a class template instantiation (e.g. std::string),
  // import the mangled name of this instantiation, and add it to the table.
  auto addTemplateSpecialization =
      [&](clang::ClassTemplateSpecializationDecl *specializationDecl) {
        auto name = nameImporter.importName(specializationDecl, currentVersion);

        // Avoid adding duplicate entries into the table.
        auto existingEntries =
            table.lookup(DeclBaseName(name.getDeclName().getBaseName()),
                         name.getEffectiveContext());
        if (existingEntries.empty()) {
          table.addEntry(name.getDeclName(), specializationDecl,
                         name.getEffectiveContext());
        }
      };
  if (auto typedefNameDecl = dyn_cast<clang::TypedefNameDecl>(named)) {
    auto underlyingDecl = typedefNameDecl->getUnderlyingType()->getAsTagDecl();

    if (auto specializationDecl =
            dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
                underlyingDecl)) {
      addTemplateSpecialization(specializationDecl);
    }
  }
  if (auto valueDecl = dyn_cast<clang::ValueDecl>(named)) {
    auto valueTypeDecl = valueDecl->getType()->getAsTagDecl();

    if (auto specializationDecl =
            dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
                valueTypeDecl)) {
      addTemplateSpecialization(specializationDecl);
    }
  }

  // Walk the members of any context that can have nested members.
  if (isa<clang::TagDecl>(named) || isa<clang::ObjCInterfaceDecl>(named) ||
      isa<clang::ObjCProtocolDecl>(named) ||
      isa<clang::ObjCCategoryDecl>(named)) {
    clang::DeclContext *dc = cast<clang::DeclContext>(named);
    for (auto member : dc->decls()) {
      if (auto friendDecl = dyn_cast<clang::FriendDecl>(member))
        if (auto underlyingDecl = friendDecl->getFriendDecl())
          member = underlyingDecl;

      if (auto namedMember = dyn_cast<clang::NamedDecl>(member))
        addEntryToLookupTable(table, namedMember, nameImporter);
    }
  }
  if (isa<clang::NamespaceDecl>(named)) {
    toolchain::SmallPtrSet<clang::Decl *, 8> alreadyAdded;
    alreadyAdded.insert(named->getCanonicalDecl());

    auto dc = cast<clang::DeclContext>(named);
    for (auto member : dc->decls()) {
      auto canonicalMember = isa<clang::NamespaceDecl>(member)
                                 ? member
                                 : member->getCanonicalDecl();
      if (!alreadyAdded.insert(canonicalMember).second)
        continue;

      if (auto namedMember = dyn_cast<clang::NamedDecl>(canonicalMember)) {
        // Make sure we're looking at the definition, otherwise, there won't
        // be any members to add.
        if (auto recordDecl = dyn_cast<clang::RecordDecl>(namedMember))
          if (auto def = recordDecl->getDefinition())
            namedMember = def;
        addEntryToLookupTable(table, namedMember, nameImporter);
      }
    }
  }
  if (auto usingDecl = dyn_cast<clang::UsingDecl>(named)) {
    for (auto usingShadowDecl : usingDecl->shadows()) {
      if (isa<clang::CXXMethodDecl>(usingShadowDecl->getTargetDecl()))
        addEntryToLookupTable(table, usingShadowDecl, nameImporter);
    }
  }
}

/// Returns the nearest parent of \p module that is marked \c explicit in its
/// module map. If \p module is itself explicit, it is returned; if no module
/// in the parent chain is explicit, the top-level module is returned.
static const clang::Module *
getExplicitParentModule(const clang::Module *module) {
  while (!module->IsExplicit && module->Parent)
    module = module->Parent;
  return module;
}

void importer::addMacrosToLookupTable(CodiraLookupTable &table,
                                      NameImporter &nameImporter) {
  auto &pp = nameImporter.getClangPreprocessor();
  auto *tu = nameImporter.getClangContext().getTranslationUnitDecl();
  bool isModule = pp.getLangOpts().isCompilingModule();
  for (const auto &macro : pp.macros(false)) {
    auto maybeAddMacro = [&](clang::MacroInfo *info,
                             clang::ModuleMacro *moduleMacro) {
      // If this is a #undef, return.
      if (!info)
        return;

      // If we hit a builtin macro, we're done.
      if (info->isBuiltinMacro())
        return;

      // If we hit a macro with invalid or predefined location, we're done.
      auto loc = info->getDefinitionLoc();
      if (loc.isInvalid())
        return;
      if (pp.getSourceManager().getFileID(loc) == pp.getPredefinesFileID())
        return;

      // If we're in a module, we really need moduleMacro to be valid.
      if (isModule && !moduleMacro) {
        // FIXME: "public" visibility macros should actually be added to the 
        // table.
        return;
      }

      // Add this entry.
      auto name = nameImporter.importMacroName(macro.first, info);
      if (name.empty())
        return;
      if (moduleMacro)
        table.addEntry(name, moduleMacro, tu);
      else
        table.addEntry(name, info, tu);
    };

    ArrayRef<clang::ModuleMacro *> moduleMacros =
        macro.second.getActiveModuleMacros(pp, macro.first);
    if (moduleMacros.empty()) {
      // Handle the bridging header case.
      clang::MacroDirective *MD = pp.getLocalMacroDirective(macro.first);
      if (!MD)
        continue;

      maybeAddMacro(MD->getMacroInfo(), nullptr);

    } else {
      clang::Module *currentModule = pp.getCurrentModule();
      SmallVector<clang::ModuleMacro *, 8> worklist;
      toolchain::copy_if(moduleMacros, std::back_inserter(worklist),
                    [currentModule](const clang::ModuleMacro *next) -> bool {
        return next->getOwningModule()->isSubModuleOf(currentModule);
      });

      while (!worklist.empty()) {
        clang::ModuleMacro *moduleMacro = worklist.pop_back_val();
        maybeAddMacro(moduleMacro->getMacroInfo(), moduleMacro);

        // Also visit overridden macros that are in a different explicit
        // submodule. This isn't a perfect way to tell if these two macros are
        // supposed to be independent, but it's close enough in practice.
        clang::Module *owningModule = moduleMacro->getOwningModule();
        auto *explicitParent = getExplicitParentModule(owningModule);
        toolchain::copy_if(moduleMacro->overrides(), std::back_inserter(worklist),
                      [&](const clang::ModuleMacro *next) -> bool {
          const clang::Module *nextModule =
              getExplicitParentModule(next->getOwningModule());
          if (!nextModule->isSubModuleOf(currentModule))
            return false;
          return nextModule != explicitParent;
        });
      }
    }
  }
}

void importer::finalizeLookupTable(
    CodiraLookupTable &table, NameImporter &nameImporter,
    ClangSourceBufferImporter &buffersForDiagnostics) {
  // Resolve any unresolved entries.
  SmallVector<CodiraLookupTable::SingleEntry, 4> unresolved;
  if (table.resolveUnresolvedEntries(unresolved)) {
    // Complain about unresolved entries that remain.
    for (auto entry : unresolved) {
      auto decl = entry.get<clang::NamedDecl *>();
      auto languageName = decl->getAttr<clang::CodiraNameAttr>();

      if (languageName
          // Clang didn't previously attach CodiraNameAttrs to forward
          // declarations, but this changed and we started diagnosing spurious
          // warnings on @class declarations. Suppress them.
          // FIXME: Can we avoid processing these decls in the first place?
          && !importer::isForwardDeclOfType(decl)) {
        clang::SourceLocation diagLoc = languageName->getLocation();
        if (!diagLoc.isValid())
          diagLoc = decl->getLocation();
        SourceLoc languageSourceLoc = buffersForDiagnostics.resolveSourceLocation(
            nameImporter.getClangContext().getSourceManager(), diagLoc);

        DiagnosticEngine &languageDiags = nameImporter.getContext().Diags;
        languageDiags.diagnose(languageSourceLoc, diag::unresolvable_clang_decl,
                            decl->getNameAsString(), languageName->getName());
        StringRef moduleName =
            nameImporter.getClangContext().getLangOpts().CurrentModule;
        if (!moduleName.empty()) {
          languageDiags.diagnose(languageSourceLoc,
                              diag::unresolvable_clang_decl_is_a_framework_bug,
                              moduleName);
        }
      }
    }
  }
}

void CodiraLookupTableWriter::populateTableWithDecl(CodiraLookupTable &table,
                                                   NameImporter &nameImporter,
                                                   clang::Decl *decl) {
  // Skip anything from an AST file.
  if (decl->isFromASTFile())
    return;

  // Iterate into extern "C" {} type declarations.
  if (auto linkageDecl = dyn_cast<clang::LinkageSpecDecl>(decl)) {
    for (auto *decl : linkageDecl->noload_decls()) {
      populateTableWithDecl(table, nameImporter, decl);
    }
    return;
  }

  // Skip non-named declarations.
  auto named = dyn_cast<clang::NamedDecl>(decl);
  if (!named)
    return;

  // Add this entry to the lookup table.
  addEntryToLookupTable(table, named, nameImporter);
}

void CodiraLookupTableWriter::populateTable(CodiraLookupTable &table,
                                           NameImporter &nameImporter) {
  auto &sema = nameImporter.getClangSema();
  for (auto decl : sema.Context.getTranslationUnitDecl()->noload_decls()) {
    populateTableWithDecl(table, nameImporter, decl);
  }

  // Add macros to the lookup table.
  addMacrosToLookupTable(table, nameImporter);

  // Finalize the lookup table, which may fail.
  finalizeLookupTable(table, nameImporter, buffersForDiagnostics);
}

std::unique_ptr<clang::ModuleFileExtensionWriter>
CodiraNameLookupExtension::createExtensionWriter(clang::ASTWriter &writer) {
  return std::make_unique<CodiraLookupTableWriter>(this, writer, languageCtx,
                                                  buffersForDiagnostics,
                                                  availability, importerImpl);
}

std::unique_ptr<clang::ModuleFileExtensionReader>
CodiraNameLookupExtension::createExtensionReader(
    const clang::ModuleFileExtensionMetadata &metadata,
    clang::ASTReader &reader, clang::serialization::ModuleFile &mod,
    const toolchain::BitstreamCursor &stream) {
  // Make sure we have a compatible block. Since these values are part
  // of the hash, it should never be wrong.
  assert(metadata.BlockName == "language.lookup");
  assert(metadata.MajorVersion == LANGUAGE_LOOKUP_TABLE_VERSION_MAJOR);
  assert(metadata.MinorVersion == LANGUAGE_LOOKUP_TABLE_VERSION_MINOR);

  std::function<void()> onRemove = [](){};
  std::unique_ptr<CodiraLookupTable> *target = nullptr;

  if (mod.Kind == clang::serialization::MK_PCH) {
    // PCH imports unconditionally overwrite the provided pchLookupTable.
    target = &pchLookupTable;
  } else {
    // Check whether we already have an entry in the set of lookup tables.
    target = &lookupTables[mod.ModuleName];
    if (*target) return nullptr;

    // Local function used to remove this entry when the reader goes away.
    std::string moduleName = mod.ModuleName;
    onRemove = [this, moduleName]() {
      lookupTables.erase(moduleName);
    };
  }

  // Create the reader.
  auto tableReader = CodiraLookupTableReader::create(this, reader, mod, onRemove,
                                                    stream);
  if (!tableReader) return nullptr;

  // Create the lookup table.
  target->reset(new CodiraLookupTable(tableReader.get()));

  // Return the new reader.
  return std::move(tableReader);
}
