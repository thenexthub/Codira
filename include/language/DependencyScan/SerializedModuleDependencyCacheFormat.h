//=== SerializedModuleDependencyCacheFormat.h - serialized format -*- C++-*-=//
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

#ifndef LANGUAGE_DEPENDENCY_SERIALIZEDCACHEFORMAT_H
#define LANGUAGE_DEPENDENCY_SERIALIZEDCACHEFORMAT_H

#include "toolchain/Bitcode/BitcodeConvenience.h"
#include "toolchain/Bitstream/BitCodes.h"

namespace toolchain {
class MemoryBuffer;
namespace vfs{
class OutputBackend;
}
}

namespace language {

class DiagnosticEngine;
class CodiraDependencyScanningService;

namespace dependencies {
namespace module_dependency_cache_serialization {

using toolchain::BCArray;
using toolchain::BCBlob;
using toolchain::BCFixed;
using toolchain::BCRecordLayout;
using toolchain::BCVBR;

/// Every .moddepcache file begins with these 4 bytes, for easy identification.
const unsigned char MODULE_DEPENDENCY_CACHE_FORMAT_SIGNATURE[] = {'I', 'M', 'D','C'};
const unsigned MODULE_DEPENDENCY_CACHE_FORMAT_VERSION_MAJOR = 10;
/// Increment this on every change.
const unsigned MODULE_DEPENDENCY_CACHE_FORMAT_VERSION_MINOR = 3;

/// Various identifiers in this format will rely on having their strings mapped
/// using this ID.
using IdentifierIDField = BCVBR<13>;
using FileIDField = IdentifierIDField;
using ModuleIDField = IdentifierIDField;
using ContextHashIDField = IdentifierIDField;

/// A bit that indicates whether or not a module is a framework
using IsFrameworkField = BCFixed<1>;
/// A bit that indicates whether or not a module is a system module
using IsSystemField = BCFixed<1>;
/// A bit that indicates whether or not a module is that of a static archive
using IsStaticField = BCFixed<1>;
/// A bit that indicates whether or not a link library is a force-load one
using IsForceLoadField = BCFixed<1>;
/// A bit that indicates whether or not an import statement is optional
using IsOptionalImport = BCFixed<1>;
/// A bit that indicates whether or not an import statement is @_exported
using IsExportedImport = BCFixed<1>;

/// Source location fields
using LineNumberField = BCFixed<32>;
using ColumnNumberField = BCFixed<32>;

/// Access level of an import
using AccessLevelField = BCFixed<8>;

/// Arrays of various identifiers, distinguished for readability
using IdentifierIDArryField = toolchain::BCArray<IdentifierIDField>;
using ModuleIDArryField = toolchain::BCArray<IdentifierIDField>;

/// Identifiers used to refer to the above arrays
using FileIDArrayIDField = IdentifierIDField;
using ContextHashIDField = IdentifierIDField;
using ModuleCacheKeyIDField = IdentifierIDField;
using ImportArrayIDField = IdentifierIDField;
using LinkLibrariesArrayIDField = IdentifierIDField;
using MacroDependenciesArrayIDField = IdentifierIDField;
using SearchPathArrayIDField = IdentifierIDField;
using FlagIDArrayIDField = IdentifierIDField;
using DependencyIDArrayIDField = IdentifierIDField;
using SourceLocationIDArrayIDField = IdentifierIDField;

/// The ID of the top-level block containing the dependency graph
const unsigned GRAPH_BLOCK_ID = toolchain::bitc::FIRST_APPLICATION_BLOCKID;

/// The .moddepcache file format consists of a METADATA record, followed by
/// zero or more IDENTIFIER records that contain various strings seen in the graph
/// (e.g. file names or compiler flags), followed by zero or more IDENTIFIER_ARRAY records
/// which are arrays of identifiers seen in the graph (e.g. list of source files or list of compile flags),
/// followed by zero or more LINK_LIBRARY_NODE records along with associated
///
/// followed by zero or more MODULE_NODE, *_DETAILS_NODE pairs of records.
namespace graph_block {
enum {
  METADATA = 1,
  MODULE_NODE,
  TIME_NODE,
  LINK_LIBRARY_NODE,
  LINK_LIBRARY_ARRAY_NODE,
  MACRO_DEPENDENCY_NODE,
  MACRO_DEPENDENCY_ARRAY_NODE,
  SEARCH_PATH_NODE,
  SEARCH_PATH_ARRAY_NODE,
  IMPORT_STATEMENT_NODE,
  IMPORT_STATEMENT_ARRAY_NODE,
  OPTIONAL_IMPORT_STATEMENT_ARRAY_NODE,
  LANGUAGE_INTERFACE_MODULE_DETAILS_NODE,
  LANGUAGE_SOURCE_MODULE_DETAILS_NODE,
  LANGUAGE_BINARY_MODULE_DETAILS_NODE,
  CLANG_MODULE_DETAILS_NODE,
  IDENTIFIER_NODE,
  IDENTIFIER_ARRAY_NODE
};

// Always the first record in the file.
using MetadataLayout = BCRecordLayout<
    METADATA,       // ID
    BCFixed<16>,    // Inter-Module Dependency graph format major version
    BCFixed<16>,    // Inter-Module Dependency graph format minor version
    BCBlob          // Scanner Invocation Context Hash
    >;

// After the metadata record, emit serialization time-stamp.
using TimeLayout = BCRecordLayout<
    TIME_NODE,       // ID
    BCBlob           // Nanoseconds since epoch as a string
    >;

// After the time stamp record, we have zero or more identifier records,
// for each unique string that is referenced in the graph.
//
// Identifiers are referenced by their sequence number, starting from 1.
// The identifier value 0 is special; it always represents the empty string.
// There is no IDENTIFIER_NODE serialized that corresponds to it, instead
// the first IDENTIFIER_NODE always has a sequence number of 1.
using IdentifierNodeLayout = BCRecordLayout<IDENTIFIER_NODE, BCBlob>;

// After the identifier records we have zero or more identifier array records.
//
// These arrays are also referenced by their sequence number,
// starting from 1, similar to identifiers above. Value 0 indicates an
// empty array. This record is used because individual array fields must
// appear as the last field of whatever record they belong to, and several of
// the below record layouts contain multiple arrays.
using IdentifierArrayLayout =
    BCRecordLayout<IDENTIFIER_ARRAY_NODE, IdentifierIDArryField>;

// A record for a given link library node containing information
// required for the build system client to capture a requirement
// to link a given dependency library.
using LinkLibraryLayout = BCRecordLayout<LINK_LIBRARY_NODE, // ID
                                         IdentifierIDField, // libraryName
                                         IsFrameworkField,  // isFramework
                                         IsStaticField,     // isStatic
                                         IsForceLoadField   // forceLoad
                                         >;
using LinkLibraryArrayLayout =
    BCRecordLayout<LINK_LIBRARY_ARRAY_NODE, IdentifierIDArryField>;

// A record for a Macro module dependency of a given dependency
// node.
using MacroDependencyLayout =
    BCRecordLayout<MACRO_DEPENDENCY_NODE,        // ID
                   IdentifierIDField,            // macroModuleName
                   IdentifierIDField,            // libraryPath
                   IdentifierIDField             // executablePath
                   >;
using MacroDependencyArrayLayout =
    BCRecordLayout<MACRO_DEPENDENCY_ARRAY_NODE, IdentifierIDArryField>;

// A record for a serialized search pathof a given dependency
// node (Codira binary module dependency only).
using SearchPathLayout =
    BCRecordLayout<SEARCH_PATH_NODE,             // ID
                   IdentifierIDField,            // path
                   IsFrameworkField,             // isFramework
                   IsSystemField                 // isSystem
                   >;
using SearchPathArrayLayout =
    BCRecordLayout<SEARCH_PATH_ARRAY_NODE, IdentifierIDArryField>;

// A record capturing information about a given 'import' statement
// captured in a dependency node, including its source location.
using ImportStatementLayout =
    BCRecordLayout<IMPORT_STATEMENT_NODE,        // ID
                   IdentifierIDField,            // importIdentifier
                   IdentifierIDField,            // bufferIdentifier
                   LineNumberField,              // lineNumber
                   ColumnNumberField,            // columnNumber
                   IsOptionalImport,             // isOptional
                   IsExportedImport,             // isExported
                   AccessLevelField              // accessLevel
                   >;
using ImportStatementArrayLayout =
    BCRecordLayout<IMPORT_STATEMENT_ARRAY_NODE, IdentifierIDArryField>;
using OptionalImportStatementArrayLayout =
    BCRecordLayout<OPTIONAL_IMPORT_STATEMENT_ARRAY_NODE, IdentifierIDArryField>;

// After the array records, we have a sequence of Module info
// records, each of which is followed by one of:
// - CodiraInterfaceModuleDetails
// - CodiraSourceModuleDetails
// - CodiraBinaryModuleDetails
// - ClangModuleDetails
using ModuleInfoLayout =
    BCRecordLayout<MODULE_NODE,                    // ID
                   IdentifierIDField,              // moduleName
                   ImportArrayIDField,             // imports
                   ImportArrayIDField,             // optionalImports
                   LinkLibrariesArrayIDField,      // linkLibraries
                   MacroDependenciesArrayIDField,  // macroDependencies
                   DependencyIDArrayIDField,       // importedCodiraModules
                   DependencyIDArrayIDField,       // importedClangModules
                   DependencyIDArrayIDField,       // crossImportOverlayModules
                   DependencyIDArrayIDField,       // languageOverlayDependencies
                   ModuleCacheKeyIDField           // moduleCacheKey
                   >;

using CodiraInterfaceModuleDetailsLayout =
    BCRecordLayout<LANGUAGE_INTERFACE_MODULE_DETAILS_NODE, // ID
                   FileIDField,                         // outputFilePath
                   FileIDField,                         // languageInterfaceFile
                   FileIDArrayIDField,                  // compiledModuleCandidates
                   FlagIDArrayIDField,                  // buildCommandLine
                   ContextHashIDField,                  // contextHash
                   IsFrameworkField,                    // isFramework
                   IsStaticField,                       // isStatic
                   FileIDField,                         // bridgingHeaderFile
                   FileIDArrayIDField,                  // sourceFiles
                   FileIDArrayIDField,                  // bridgingSourceFiles
                   IdentifierIDField,                   // bridgingModuleDependencies
                   IdentifierIDField,                   // CASFileSystemRootID
                   IdentifierIDField,                   // bridgingHeaderIncludeTree
                   IdentifierIDField,                   // moduleCacheKey
                   IdentifierIDField                    // UserModuleVersion
                   >;

using CodiraSourceModuleDetailsLayout =
    BCRecordLayout<LANGUAGE_SOURCE_MODULE_DETAILS_NODE, // ID
                   FileIDField,                      // bridgingHeaderFile
                   FileIDArrayIDField,               // sourceFiles
                   FileIDArrayIDField,               // bridgingSourceFiles
                   FileIDArrayIDField,               // bridgingModuleDependencies
                   IdentifierIDField,                // CASFileSystemRootID
                   IdentifierIDField,                // bridgingHeaderIncludeTree
                   FlagIDArrayIDField,               // buildCommandLine
                   FlagIDArrayIDField,               // bridgingHeaderBuildCommandLine
                   IdentifierIDField,                // chainedBridgingHeaderPath
                   IdentifierIDField                 // chainedBridgingHeaderContent
                   >;

using CodiraBinaryModuleDetailsLayout =
    BCRecordLayout<LANGUAGE_BINARY_MODULE_DETAILS_NODE, // ID
                   FileIDField,                      // compiledModulePath
                   FileIDField,                      // moduleDocPath
                   FileIDField,                      // moduleSourceInfoPath
                   FileIDField,                      // headerImport
                   FileIDField,                      // definingInterfacePath
                   IdentifierIDField,                // headerModuleDependencies
                   FileIDArrayIDField,               // headerSourceFiles
                   SearchPathArrayIDField,           // serializedSearchPaths
                   IsFrameworkField,                 // isFramework
                   IsStaticField,                    // isStatic
                   IdentifierIDField,                // moduleCacheKey
                   IdentifierIDField                 // UserModuleVersion
                   >;

using ClangModuleDetailsLayout =
    BCRecordLayout<CLANG_MODULE_DETAILS_NODE, // ID
                   FileIDField,               // pcmOutputPath
                   FileIDField,               // mappedPCMPath
                   FileIDField,               // moduleMapPath
                   ContextHashIDField,        // contextHash
                   FlagIDArrayIDField,        // commandLine
                   FileIDArrayIDField,        // fileDependencies
                   IdentifierIDField,         // CASFileSystemRootID
                   IdentifierIDField,         // clangIncludeTreeRoot
                   IdentifierIDField,         // moduleCacheKey
                   IsSystemField              // isSystem
                   >;
} // namespace graph_block

/// Tries to read the dependency graph from the given buffer.
/// Returns \c true if there was an error.
bool readInterModuleDependenciesCache(toolchain::MemoryBuffer &buffer,
                                      ModuleDependenciesCache &cache,
                                      toolchain::sys::TimePoint<> &serializedCacheTimeStamp);

/// Tries to read the dependency graph from the given path name.
/// Returns true if there was an error.
bool readInterModuleDependenciesCache(toolchain::StringRef path,
                                      ModuleDependenciesCache &cache,
                                      toolchain::sys::TimePoint<> &serializedCacheTimeStamp);

/// Tries to write the dependency graph to the given path name.
/// Returns true if there was an error.
bool writeInterModuleDependenciesCache(DiagnosticEngine &diags,
                                       toolchain::vfs::OutputBackend &backend,
                                       toolchain::StringRef outputPath,
                                       const ModuleDependenciesCache &cache);

/// Tries to write out the given dependency cache with the given
/// bitstream writer.
void writeInterModuleDependenciesCache(toolchain::BitstreamWriter &Out,
                                       const ModuleDependenciesCache &cache);

} // end namespace module_dependency_cache_serialization
} // end namespace dependencies
} // end namespace language

#endif
