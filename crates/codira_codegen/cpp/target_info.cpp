//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Asks LLVM's own subtarget tables whether a CPU name is valid for a
//!   target triple, and which features that CPU implies. LLVM's C API can
//!   name the host CPU but cannot answer either question; without this, an
//!   unknown `--target-cpu` is a warning on stderr and a silently generic
//!   build.

#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Triple.h>

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

namespace {
char *codira_dup(const std::string &text) {
    char *out = static_cast<char *>(std::malloc(text.size() + 1));
    if (out != nullptr) {
        std::memcpy(out, text.c_str(), text.size() + 1);
    }
    return out;
}
}  // namespace

extern "C" {

/// Status codes returned by `codira_llvm_cpu_features`.
enum CodiraCpuStatus {
    CodiraCpuOk = 0,
    CodiraCpuUnknownTarget = 1,
    CodiraCpuUnknownCpu = 2,
};

/// Resolves `cpu` for `triple`. On success `*out` is the comma-separated
/// list of features the CPU enables (`+feat,+feat,...`, in LLVM's table
/// order). On `CodiraCpuUnknownCpu` `*out` is the comma-separated list of
/// CPU names LLVM knows for the target; on `CodiraCpuUnknownTarget` it is
/// LLVM's error message. `*out` is allocated with `malloc` and must be
/// released with `codira_llvm_free`. The target's MC layer must already be
/// initialized (`LLVMInitialize<Target>TargetMC`).
int codira_llvm_cpu_features(const char *triple, const char *cpu, char **out) {
    *out = nullptr;
    std::string error;
    const llvm::Triple the_triple(triple);
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(the_triple, error);
    if (target == nullptr) {
        *out = codira_dup(error);
        return CodiraCpuUnknownTarget;
    }
    // Ask a CPU-less subtarget first: creating one for an unknown CPU makes
    // LLVM print "not a recognized processor" on stderr, and reporting that
    // is this function's job.
    std::unique_ptr<llvm::MCSubtargetInfo> probe(
        target->createMCSubtargetInfo(the_triple, "", ""));
    if (!probe) {
        *out = codira_dup("the target has no subtarget information");
        return CodiraCpuUnknownTarget;
    }
    if (!probe->isCPUStringValid(cpu)) {
        std::string names;
        for (const auto &description : probe->getAllProcessorDescriptions()) {
            if (!names.empty()) {
                names += ",";
            }
            names += description.Key;
        }
        *out = codira_dup(names);
        return CodiraCpuUnknownCpu;
    }
    std::unique_ptr<llvm::MCSubtargetInfo> info(
        target->createMCSubtargetInfo(the_triple, cpu, ""));
    if (!info) {
        *out = codira_dup("the target has no subtarget information");
        return CodiraCpuUnknownTarget;
    }
    std::string features;
    const llvm::FeatureBitset &bits = info->getFeatureBits();
    for (const auto &feature : info->getAllProcessorFeatures()) {
        if (bits.test(feature.Value)) {
            if (!features.empty()) {
                features += ",";
            }
            features += "+";
            features += feature.Key;
        }
    }
    *out = codira_dup(features);
    return CodiraCpuOk;
}

void codira_llvm_free(char *text) { std::free(text); }
}
