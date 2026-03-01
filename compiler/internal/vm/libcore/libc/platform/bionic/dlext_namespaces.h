/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <sys/cdefs.h>

#include <android/dlext.h>
#include <stdbool.h>
#include <stdint.h>

__BEGIN_DECLS

/**
 * Initializes the anonymous namespace.
 *
 * `shared_libs_sonames` is a list of sonames to be shared with the default namespace,
 * separated by colons (such as "libc.so:libm.so:libdl.so").
 *
 * `library_search_path` is the search path for the anonymous namespace.
 * The anonymous namespace is used when the linker cannot identify the caller of
 * dlopen() or dlsym(). This happens for code not loaded by the dynamic linker,
 * such as calls from a custom JIT.
 */
extern bool android_init_anonymous_namespace(const char* shared_libs_sonames,
                                             const char* library_search_path);

/**
 * Bitmask flags for the android_create_namespace() `type` argument.
 */
enum {
  /**
   * A regular namespace is a namespace with a custom search path that does
   * not impose any restrictions on the location of native libraries.
   */
  ANDROID_NAMESPACE_TYPE_REGULAR = 0,

  /**
   * An isolated namespace requires all the libraries to be on the search path
   * or under `permitted_when_isolated_path`. The search path is the union of
   * `ld_library_path` and `default_library_path`.
   */
  ANDROID_NAMESPACE_TYPE_ISOLATED = 1,

  /**
   * "Share" the caller namespace's list of libraries.
   *
   * This actually _clones_ the list of libraries of the caller namespace
   * upon creation rather than actually sharing:
   *
   * 1. Both the caller namespace and the new one will use the same copy of a
   *    library if it was already loaded in the caller namespace.
   *
   * but
   *
   * 2. Libraries loaded after the namespace is created will not be shared.
   *
   * Shared namespaces can be isolated or regular.
   *
   * Shared namespaces do not inherit the search path or permitted path from
   * the caller namespace.
   */
  ANDROID_NAMESPACE_TYPE_SHARED = 2,

  /**
   * Enable the exempt-list workaround for the namespace.
   * See http://b/26394120 for details.
   */
  ANDROID_NAMESPACE_TYPE_EXEMPT_LIST_ENABLED = 0x08000000,

  /**
   * Use this namespace as the anonymous namespace.
   *
   * There can be only one anonymous namespace in a process.
   * If there is already an anonymous namespace in the process,
   * using this flag when creating a new namespace is an error.
   */
  ANDROID_NAMESPACE_TYPE_ALSO_USED_AS_ANONYMOUS = 0x10000000,

  /** A common combination. */
  ANDROID_NAMESPACE_TYPE_SHARED_ISOLATED = ANDROID_NAMESPACE_TYPE_SHARED |
                                           ANDROID_NAMESPACE_TYPE_ISOLATED,
};

/**
 * Create a new linker namespace.
 *
 * `ld_library_path` and `default_library_path` represent the search path
 * for the libraries in the namespace.
 *
 * The libraries in the namespace are searched in the following order:
 * 1. `ld_library_path` (think of this as a namespace-local $LD_LIBRARY_PATH).
 * 2. In directories specified by DT_RUNPATH of the "needed by" binary.
 * 3. `default_library_path` (think of this as a namespace-local default library path).
 *
 * If the ANDROID_NAMESPACE_TYPE_ISOLATED bit is set in `type`,
 * the resulting namespace requires all of the libraries to be on the search
 * path or under the `permitted_when_isolated_path`;
 * the search path is `ld_library_path` followed by `default_library_path`.
 * Note that the `permitted_when_isolated_path` path is not part of the search
 * path and does not affect the search order: it's a way to allow loading
 * libraries from specific locations when using absolute paths.
 *
 * If a library or any of its dependencies are outside of the `permitted_when_isolated_path`
 * and search path, and not part of the public namespace, dlopen() will fail.
 */
extern struct android_namespace_t* android_create_namespace(const char* name,
                                                            const char* ld_library_path,
                                                            const char* default_library_path,
                                                            uint64_t type,
                                                            const char* permitted_when_isolated_path,
                                                            struct android_namespace_t* parent);

extern bool android_link_namespaces(struct android_namespace_t* from,
                                    struct android_namespace_t* to,
                                    const char* shared_libs_sonames);

extern bool android_link_namespaces_all_libs(struct android_namespace_t* from,
                                             struct android_namespace_t* to);

extern struct android_namespace_t* android_get_exported_namespace(const char* name);

// TODO: move this somewhere else, since it's unrelated to linker namespaces.
extern void android_set_application_target_sdk_version(int target);

__END_DECLS
