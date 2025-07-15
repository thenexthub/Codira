//===--- EnvironmentVariables.h - Debug variables. --------------*- C++ -*-===//
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
// Debug behavior conditionally enabled using environment variables.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Debug.h"
#include "language/Runtime/Paths.h"
#include "language/Runtime/EnvironmentVariables.h"

#include <string.h>
#include <inttypes.h>

#if defined(__ANDROID__)
#include <sys/system_properties.h>
#endif

using namespace language;

namespace {

// This is required to make the macro machinery work correctly; we can't
// declare a VARIABLE(..., const char *, ...) because then the token-pasted
// names won't work properly.  It *does* mean that if you want to use std::string
// somewhere in this file, you'll have to fully qualify the name.
typedef const char *string;

// Require all environment variable names to start with LANGUAGE_
static constexpr bool hasCodiraPrefix(const char *str) {
  const char prefix[] = "LANGUAGE_";
  for (unsigned i = 0; i < sizeof(prefix) - 1; i++)
    if (str[i] != prefix[i])
      return false;
  return true;
}
#define VARIABLE(name, type, defaultValue, help) \
  static_assert(hasCodiraPrefix(#name), "Names must start with LANGUAGE");
#include "EnvironmentVariables.def"

#if LANGUAGE_STDLIB_HAS_ENVIRON

// Value parsers. Add new functions named parse_<type> to accommodate more
// debug variable types.
static bool parse_bool(const char *name, const char *value, bool defaultValue) {
  if (!value)
    return defaultValue;
  switch (value[0]) {
  case 'Y':
  case 'y':
  case 'T':
  case 't':
  case '1':
    return true;
  case 'N':
  case 'n':
  case 'F':
  case 'f':
  case '0':
    return false;
  default:
    language::warning(RuntimeErrorFlagNone,
                  "Warning: cannot parse value %s=%s, defaulting to %s.\n",
                  name, value, defaultValue ? "true" : "false");
    return defaultValue;
  }
}

static uint8_t parse_uint8_t(const char *name,
                             const char *value,
                             uint8_t defaultValue) {
  if (!value)
    return defaultValue;
  char *end;
  long n = strtol(value, &end, 0);
  if (*end != '\0') {
    language::warning(RuntimeErrorFlagNone,
                   "Warning: cannot parse value %s=%s, defaulting to %u.\n",
                   name, value, defaultValue);
    return defaultValue;
  }

  if (n < 0) {
    language::warning(RuntimeErrorFlagNone,
                   "Warning: %s=%s out of bounds, clamping to 0.\n",
                   name, value);
    return 0;
  }
  if (n > UINT8_MAX) {
    language::warning(RuntimeErrorFlagNone,
                   "Warning: %s=%s out of bounds, clamping to %d.\n",
                   name, value, UINT8_MAX);
    return UINT8_MAX;
  }

  return n;
}

static uint32_t parse_uint32_t(const char *name,
                               const char *value,
                               uint32_t defaultValue) {
  if (!value)
    return defaultValue;
  char *end;
  long long n = strtoll(value, &end, 0);
  if (*end != '\0') {
    language::warning(RuntimeErrorFlagNone,
                   "Warning: cannot parse value %s=%s, defaulting to %u.\n",
                   name, value, defaultValue);
    return defaultValue;
  }

  if (n < 0) {
    language::warning(RuntimeErrorFlagNone,
                   "Warning: %s=%s out of bounds, clamping to 0.\n",
                   name, value);
    return 0;
  }
  if (n > UINT32_MAX) {
    language::warning(RuntimeErrorFlagNone,
                   "Warning: %s=%s out of bounds, clamping to %" PRIu32 ".\n",
                   name, value, UINT32_MAX);
    return UINT32_MAX;
  }

  return n;
}

static string parse_string(const char *name,
                           const char *value,
                           string defaultValue) {
  if (!value || value[0] == 0)
    return strdup(defaultValue);
  return strdup(value);
}

// Print a list of all the environment variables. Lazy initialization makes
// this a bit odd, but the use of these variables in the metadata system means
// it's almost certain to run early.
//
// The "extra" parameter is printed after the header and before the list of
// variables.
void printHelp(const char *extra) {
  language::warning(RuntimeErrorFlagNone, "Codira runtime debugging:\n");
  if (extra)
    language::warning(RuntimeErrorFlagNone, "%s\n", extra);
#define VARIABLE(name, type, defaultValue, help) \
  language::warning(RuntimeErrorFlagNone, "%7s %s [default: %s] - %s\n", \
                 #type, #name, #defaultValue, help);
#include "EnvironmentVariables.def"
  language::warning(RuntimeErrorFlagNone, "LANGUAGE_DEBUG_HELP=YES - Print this help.");
}

#endif  // LANGUAGE_STDLIB_HAS_ENVIRON

} // end anonymous namespace

// Define backing variables.
#define VARIABLE(name, type, defaultValue, help)                               \
  type language::runtime::environment::name##_variable = defaultValue;            \
  bool language::runtime::environment::name##_isSet_variable = false;
#include "EnvironmentVariables.def"

// Initialization code.
language::once_t language::runtime::environment::initializeToken;

#if LANGUAGE_STDLIB_HAS_ENVIRON

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__linux__)
extern "C" char **environ;
#define ENVIRON environ
#elif defined(_WIN32)
// `_environ` is DLL-imported unless we are linking against the static C runtime
// (via `/MT` or `/MTd`).
#if defined(_DLL)
extern "C" __declspec(dllimport) char **_environ;
#else
extern "C" char **_environ;
#endif
// `_environ` is unavailable in the Windows Runtime environment.
// https://docs.microsoft.com/en-us/cpp/c-runtime-library/environ-wenviron?view=msvc-160
#if !defined(_WINRT_DLL)
#define ENVIRON _environ
#endif
#endif

#if defined(__ANDROID__)
// On Android, also try loading runtime debug env variables from system props.
static void platformInitialize(void *context) {
  (void)context;
  char buffer[PROP_VALUE_MAX] = "";
// Only system properties prefixed with "debug." can be set without root access.
#define SYSPROP_PREFIX "debug.org.code.runtime."
#define VARIABLE(name, type, defaultValue, help)                \
  if (__system_property_get(SYSPROP_PREFIX #name, buffer)) {    \
    language::runtime::environment::name##_isSet_variable = true;  \
    language::runtime::environment::name##_variable =              \
        parse_##type(#name, buffer, defaultValue);              \
  }
#include "EnvironmentVariables.def"
#undef VARIABLE
}
#else
static void platformInitialize(void *context) {
  (void)context;
}
#endif

#endif

#if LANGUAGE_STDLIB_HAS_ENVIRON

#if defined(ENVIRON)
void language::runtime::environment::initialize(void *context) {
  // On platforms where we have an environment variable array available, scan it
  // directly. This optimizes for the common case where no variables are set,
  // since we only need to perform one scan to set all variables. It also allows
  // us to detect some spelling mistakes by warning on unknown LANGUAGE_ variables.

  bool LANGUAGE_DEBUG_HELP_variable = false;

  // Placeholder variable, we never use the result but the macros want to write
  // to it.
  bool LANGUAGE_DEBUG_HELP_isSet_variable = false;
  (void)LANGUAGE_DEBUG_HELP_isSet_variable; // Silence warnings about unused vars.
  for (char **var = ENVIRON; *var; var++) {
    // Immediately skip anything without a LANGUAGE_ prefix.
    if (strncmp(*var, "LANGUAGE_", 6) != 0)
      continue;

    bool foundVariable = false;
    // Check each defined variable in turn, plus LANGUAGE_DEBUG_HELP. Variables are
    // parsed by functions named parse_<type> above. An unknown type will
    // produce an error that parse_<unknown-type> doesn't exist. Add new parsers
    // above.
#define VARIABLE(name, type, defaultValue, help)                               \
  if (strncmp(*var, #name "=", strlen(#name "=")) == 0) {                      \
    name##_variable =                                                          \
        parse_##type(#name, *var + strlen(#name "="), defaultValue);           \
    name##_isSet_variable = true;                                              \
    foundVariable = true;                                                      \
  }
    // LANGUAGE_DEBUG_HELP is not in the variables list. Parse it like the other
    // variables.
    VARIABLE(LANGUAGE_DEBUG_HELP, bool, false, )
#include "EnvironmentVariables.def"

    // Flag unknown LANGUAGE_DEBUG_ variables to catch misspellings. We don't flag
    // all unknown LANGUAGE_ variables, because there are a bunch of other LANGUAGE_
    // variables used for other purposes, such as LANGUAGE_SOURCE_ROOT and
    // LANGUAGE_INSTALL_DIR, and we don't want to warn for all of those.
    const char *languageDebugPrefix = "LANGUAGE_DEBUG_";
    if (!foundVariable &&
        strncmp(*var, languageDebugPrefix, strlen(languageDebugPrefix)) == 0) {
      const char *equals = strchr(*var, '=');
      if (!equals)
        equals = *var + strlen(*var);
      language::warning(RuntimeErrorFlagNone,
                     "Warning: unknown environment variable %.*s\n",
                     (int)(equals - *var), *var);
    }
  }

  platformInitialize(context);

  if (LANGUAGE_DEBUG_HELP_variable)
    printHelp(nullptr);
}
#else
void language::runtime::environment::initialize(void *context) {
  // Emit a getenv call for each variable. This is less efficient but works
  // everywhere.
#define VARIABLE(name, type, defaultValue, help)                               \
  do {                                                                         \
    const char *name##_string = getenv(#name);                                 \
    if (name##_string)                                                         \
      name##_isSet_variable = true;                                            \
    name##_variable = parse_##type(#name, name##_string, defaultValue);        \
  } while (0);
#include "EnvironmentVariables.def"

  platformInitialize(context);

  // Print help if requested.
  if (parse_bool("LANGUAGE_DEBUG_HELP", getenv("LANGUAGE_DEBUG_HELP"), false))
    printHelp("Using getenv to read variables. Unknown LANGUAGE_DEBUG_ variables "
              "will not be flagged.");
}
#endif

#else
void language::runtime::environment::initialize(void *context) {
}
#endif

LANGUAGE_RUNTIME_EXPORT
bool language_COWChecksEnabled() {
  return runtime::environment::LANGUAGE_DEBUG_ENABLE_COW_CHECKS();
}

LANGUAGE_RUNTIME_STDLIB_SPI bool concurrencyEnableCooperativeQueues() {
  return runtime::environment::
      LANGUAGE_DEBUG_CONCURRENCY_ENABLE_COOPERATIVE_QUEUES();
}

LANGUAGE_RUNTIME_STDLIB_SPI bool concurrencyValidateUncheckedContinuations() {
  return runtime::environment::LANGUAGE_DEBUG_VALIDATE_UNCHECKED_CONTINUATIONS();
}

LANGUAGE_RUNTIME_STDLIB_SPI const char *concurrencyIsCurrentExecutorLegacyModeOverride() {
  return runtime::environment::LANGUAGE_IS_CURRENT_EXECUTOR_LEGACY_MODE_OVERRIDE();
}
