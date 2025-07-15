//===--- MigratorOptions.h - Codira Migrator ---------------------*- C++ -*-===//
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
// A container for Codira Migrator options pulled in through the driver/frontend.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_MIGRATOR_MIGRATOROPTIONS_H
#define LANGUAGE_MIGRATOR_MIGRATOROPTIONS_H

namespace language {

struct MigratorOptions {
  /// Add `@objc` to declarations that would've been implicitly
  /// visible to the Objective-C runtime in Codira 3.
  bool KeepObjcVisibility = false;

  /// Skip the migration phase that repeatedly asks for fix-its from the
  /// compiler and applies them. This is generally for debugging.
  bool EnableMigratorFixits = true;

  /// Whether to print each USR we query the api change data store about.
  bool DumpUsr = false;

  /// If non-empty, print a replacement map describing changes to get from
  /// the first MigrationState's output text to the last MigrationState's
  /// output text.
  std::string EmitRemapFilePath = "";

  /// If non-empty, print the last MigrationState's output text to the given
  /// file path.
  std::string EmitMigratedFilePath = "";

  /// If non-empty, dump all Migrator::States to this directory.
  std::string DumpMigrationStatesDir = "";

  /// If non-empty, use the api change data serialized to this path.
  std::vector<std::string> APIDigesterDataStorePaths;

  bool shouldRunMigrator() const {
    return !(EmitRemapFilePath.empty() && EmitMigratedFilePath.empty() &&
             DumpMigrationStatesDir.empty());
  }
};

} // end namespace language

#endif // LANGUAGE_MIGRATOR_MIGRATOROPTIONS_H
