# arktarget_options

## Overview
The **`arktarget_options`** library provides a mechanism to select runtime parameters based on the **device** and **platform** on which **ArkTS** is running.  
It enables adaptive configuration by determining the current execution target and fetching corresponding parameter values.

---

## Features
- Retrieve the current execution target name at runtime.  
- Obtain target-specific configuration values defined in `options.yaml`.  
- Support for flexible mapping through user-provided model maps.

---

## API Reference

### 1. `std::string GetTargetString()`
Determines and returns the name of the current execution target.


### 2. `template <class ModelMap> static typename ModelMap::mapped_type GetTargetSpecificOptionValue(const ModelMap &modelMap, TargetGetterCallback getTargetString)`

Retrieves a value corresponding to the target-specific parameters defined under the  
`default_target_specific` section in `static_core/runtime/options.yaml`.

**Parameters:**
- `modelMap` — A mapping of target names to parameter values.  
- `getTargetString` — A callback used to determine the current target string.

---

### Configuration

The relevant configuration options should be defined in:

static_core/runtime/options.yaml

See: https://gitcode.com/openharmony/arkcompiler_runtime_core/blob/master/static_core/runtime/options.yaml

---

### Usage example

Add an option to [options.yaml](https://gitcode.com/openharmony/arkcompiler_runtime_core/blob/master/static_core/runtime/options.yaml), use the default_target_specific label instead of the default label, then add sub-labels where each label string corresponds to a device model or other target, and associate a value with that label:

```yaml
- name: gc-workers-count
  type: uint32_t
  # default value depends on a specific device model
  default_target_specific:
    HL1CMSM: 2  # mate 60 pro
    ohos: 2 # devboard
    HN1NOAHM: 2 # mate 40 pro
    default: 2
  description: Set number of additional GC helper threads. GC uses these threads to speed up collection doing work in parallel.
```

  The automatically generated runtime_options_gen.h will use the default_target_specific options specified in options.yaml to provide the correct parameters for the target at runtime, utilizing the API provided by the arktarget_options library.

```cpp
  explicit Options(const std::string &exe_path)
    : exe_dir_(GetExeDir(exe_path))
    , gc_workers_count_{
        "gc-workers-count",
        ark::default_target_options::GetTargetSpecificOptionValue<std::map<std::string, uint32_t>>(
            {
                {R"(HL1CMSM)", 2},
                {R"(ohos)", 2},
                {R"(HN1NOAHM)", 2},
                {R"(default)", 2},
            },
            ark::default_target_options::GetTargetString
        ),
        R"(Set number of additional GC helper threads.
        GC uses these threads to speed up collection by doing work in parallel.
        Default (target specific):
          on 'HL1CMSM': 2
          on 'ohos': 2
          on 'HN1NOAHM': 2
          on 'default': 2
        )"
      }
{}
```