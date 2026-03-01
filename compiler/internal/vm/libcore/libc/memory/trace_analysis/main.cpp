/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <iostream>
#include <memory>
#include <string_view>

#include <memory_trace/TraceInfo.h>

#include "OpGenAnalysis.h"
#include "OpOverlapAnalysis.h"

namespace {

class OptionBase;

class OptionRegistry {
 public:
  static OptionRegistry& get() {
    static OptionRegistry registry;
    return registry;
  }

  void RegisterOption(OptionBase* opt) { options_.push_back(opt); }

  auto begin() { return options_.begin(); }
  auto end() { return options_.end(); }

  void Stream(std::ostream& os, std::string_view indent = "\t");

 private:
  OptionRegistry() {}

  std::vector<OptionBase*> options_;
};

class OptionBase {
 public:
  template <typename... Analyses>
  OptionBase(std::string_view name, std::string_view desc) : name_(name), desc_(desc) {
    OptionRegistry::get().RegisterOption(this);
  }

  std::string_view name() const { return name_; }
  std::string_view desc() const { return desc_; }

  void GetAnalyses(std::vector<Analysis*>& analyses) {
    for (auto& analysis : analyses_) analyses.push_back(analysis.get());
  }

 protected:
  template <typename Analysis, typename... Analyses,
            std::enable_if_t<(sizeof...(Analyses) > 0), bool> = true>
  void AppendAnalyses() {
    AppendAnalyses<Analysis>();
    AppendAnalyses<Analyses...>();
  }
  template <typename Analysis>
  void AppendAnalyses() {
    analyses_.push_back(std::move(std::make_unique<Analysis>()));
  }

  std::string_view name_;
  std::string_view desc_;
  std::vector<std::unique_ptr<Analysis>> analyses_;
};

template <typename... Analyses>
class Option : public OptionBase {
 public:
  Option(std::string_view name, std::string_view desc) : OptionBase(name, desc) {
    AppendAnalyses<Analyses...>();
  }
};

}  // namespace

void OptionRegistry::Stream(std::ostream& os, std::string_view indent) {
  for (OptionBase* opt : options_) {
    os << indent << "--" << opt->name() << ": " << opt->desc() << std::endl;
  }
}

static bool ParseArgs(int argc, char** argv, std::vector<Analysis*>& analyses,
                      std::vector<std::string>& traces) {
  auto& registry = OptionRegistry::get();
  for (int i = 0; i < argc; ++i) {
    if (argv[i][0] != '-') {
      traces.push_back(argv[i]);
      struct stat st;
      if (stat(traces.back().c_str(), &st) != 0) {
        if (errno == ENOENT) {
          std::cerr << traces.back() << " does not exist." << std::endl;
        } else {
          std::cerr << "stat of file " << traces.back() << " failed: " << strerror(errno)
                    << std::endl;
        }
        return false;
      }
      if (!S_ISREG(st.st_mode)) {
        std::cerr << traces.back() << " is not a regular file." << std::endl;
        return false;
      }
      continue;
    } else if (argv[i][1] != '-') {
      std::cerr << "Unknown option: " << argv[i] << std::endl;
      return false;
    }

    bool found = false;
    std::string_view arg(&argv[i][2]);
    for (auto* option : registry) {
      if (option->name() == arg) {
        option->GetAnalyses(analyses);
        found = true;
        break;
      }
    }
    if (!found) {
      std::cerr << "Unknown option: " << argv[i] << std::endl;
      return false;
    }
  }
  return true;
}

void Usage() {
  std::cerr << "Usage: memory_trace_analysis TRACE_FILE1 TRACE_FILE2 ... [Analyses...]\n";
  std::cerr << "Analyses:\n";
  OptionRegistry::get().Stream(std::cerr);
}

int main(int argc, char** argv) {
  static Option<OpMinMaxAnalysis, OpAverageAnalysis> gen(
      "op-gen-stats", "get the min/max/avg of each kind of alloc operation");
  static Option<OpOverlapAnalysis> overlap(
      "op-overlap", "get the amount of overlap in between each kind of alloc operation");

  if (argc < 2) {
    Usage();
    return 1;
  }

  std::vector<Analysis*> analyses;
  std::vector<std::string> traces;
  if (!ParseArgs(argc - 1, argv + 1, analyses, traces)) {
    Usage();
    return 1;
  }
  if (analyses.empty()) {
    std::cerr << "No analyses chosen.\n";
    Usage();
    return 1;
  }
  if (traces.empty()) {
    std::cerr << "No trace files.\n";
    Usage();
    return 1;
  }

  for (auto& trace : traces) {
    std::cout << "Analyzing trace " << trace << std::endl;
    memory_trace::TraceInfo info;
    info.Init(trace.c_str());
    for (auto& analysis : analyses) {
      std::cout << "  " << analysis->Name() << std::endl;
      analysis->Gather(info);
      analysis->StreamResult(std::cout, "    ");
    }
  }
  return 0;
}
