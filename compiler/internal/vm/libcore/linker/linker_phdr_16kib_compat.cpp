/*
 * Copyright (C) 2012 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "linker_phdr.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "linker_debug.h"
#include "linker_dlwarning.h"
#include "linker_globals.h"

#include "platform/bionic/macros.h"
#include "platform/bionic/page.h"

#include <android-base/stringprintf.h>

#include <algorithm>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

static bool g_enable_16kb_app_compat;

static inline bool segment_contains_prefix(const ElfW(Phdr)* segment, const ElfW(Phdr)* prefix) {
  return segment && prefix && segment->p_vaddr == prefix->p_vaddr;
}

void set_16kb_appcompat_mode(bool enable_app_compat) {
  g_enable_16kb_app_compat = enable_app_compat;
}

bool get_16kb_appcompat_mode() {
  return g_enable_16kb_app_compat;
}

/*
 * Returns true if the ELF contains at most 1 RELRO segment; and populates @relro_phdr
 * with the relro phdr or nullptr if none.
 *
 * Returns false if more than 1 RELRO segments are found.
 */
bool ElfReader::HasAtMostOneRelroSegment(const ElfW(Phdr)** relro_phdr) {
  const ElfW(Phdr)* relro = nullptr;
  for (size_t i = 0; i < phdr_num_; ++i) {
    const ElfW(Phdr)* phdr = &phdr_table_[i];

    if (phdr->p_type != PT_GNU_RELRO) {
      continue;
    }

    if (relro == nullptr) {
      relro = phdr;
    } else {
      return false;
    }
  }

  *relro_phdr = relro;

  return true;
}

/*
 * In 16KiB compatibility mode ELFs with the following segment layout
 * can be loaded successfully:
 *
 *         ┌────────────┬─────────────────────────┬────────────┐
 *         │            │                         │            │
 *         │  (RO|RX)*  │   (RW - RELRO prefix)?  │    (RW)*   │
 *         │            │                         │            │
 *         └────────────┴─────────────────────────┴────────────┘
 *
 * In other words, compatible layouts have:
 *         - zero or more RO or RX segments;
 *         - followed by zero or one RELRO prefix;
 *         - followed by zero or more RW segments (this can include the RW
 *           suffix from the segment containing the RELRO prefix, if any)
 *
 * In 16KiB compat mode, after relocation, the ELF is layout in virtual
 * memory is as shown below:
 *         ┌──────────────────────────────────────┬────────────┐
 *         │                                      │            │
 *         │                (RX)?                 │    (RW)?   │
 *         │                                      │            │
 *         └──────────────────────────────────────┴────────────┘
 *
 * In compat mode:
 *         - the RO and RX segments along with the RELRO prefix are protected
 *           as RX;
 *         - and the RW segments along with RW suffix from the relro segment,
 *           if any; are RW protected.
 *
 * This allows for the single RX|RW permission boundary to be aligned with
 * a 16KiB page boundary; since a single page cannot share multiple
 * permissions.
 *
 * IsEligibleForRXRWAppCompat() identifies compatible ELFs and populates @vaddr
 * with the boundary between RX|RW portions.
 *
 * Returns true if the ELF can be loaded in compat mode, else false.
 */
bool ElfReader::IsEligibleForRXRWAppCompat(ElfW(Addr)* vaddr) {
  const ElfW(Phdr)* relro_phdr = nullptr;
  if (!HasAtMostOneRelroSegment(&relro_phdr)) {
    DL_WARN("\"%s\": RX|RW compat loading failed: Multiple RELRO segments found", name_.c_str());
    return false;
  }

  const ElfW(Phdr)* last_rx = nullptr;
  const ElfW(Phdr)* last_rw = nullptr;
  const ElfW(Phdr)* first_rw = nullptr;

  for (size_t i = 0; i < phdr_num_; ++i) {
    const ElfW(Phdr)* curr = &phdr_table_[i];
    const ElfW(Phdr)* prev = (i > 0) ? &phdr_table_[i - 1] : nullptr;

    if (curr->p_type != PT_LOAD) {
      continue;
    }

    int prot = PFLAGS_TO_PROT(curr->p_flags);

    if ((prot & PROT_WRITE) && (prot & PROT_READ)) {
      if (!first_rw) {
        first_rw = curr;
      }

      if (last_rw && last_rw != prev) {
        DL_WARN("\"%s\": RX|RW compat loading failed: ELF contains non-adjacent RW segments",
                name_.c_str());
        return false;
      }

      last_rw = curr;
    } else if ((prot & PROT_EXEC) && (prot & PROT_READ)) {
      if (!last_rx || last_rx > last_rw) {
        last_rx = curr;
      } else  {
        DL_WARN(
            "\"%s\": RX|RW compat loading failed: ELF contains RX segments "
            "separated by RW segments",
            name_.c_str());
        return false;
      }
    }
  }

  if (!relro_phdr) {
    *vaddr = __builtin_align_down(first_rw->p_vaddr, kCompatPageSize);
    return true;
  }

  // The RELRO segment is present, it must be the prefix of the first RW segment.
  if (!segment_contains_prefix(first_rw, relro_phdr)) {
    DL_WARN("\"%s\": RX|RW compat loading failed: RELRO is not in the first RW segment",
            name_.c_str());
    return false;
  }

  uint64_t end;
  if (__builtin_add_overflow(relro_phdr->p_vaddr, relro_phdr->p_memsz, &end)) {
    DL_WARN("\"%s\": RX|RW compat loading failed: relro vaddr + memsz overflowed", name_.c_str());
    return false;
  }

  *vaddr = __builtin_align_up(end, kCompatPageSize);
  return true;
}

/*
 * Returns the offset/shift needed to align @vaddr to a page boundary
 * for RX|RW compat loading.
 */
static inline ElfW(Addr) perm_boundary_offset(const ElfW(Addr) addr) {
  ElfW(Addr) offset = page_offset(addr);

  return offset ? page_size() - offset : 0;
}

enum relro_pos_t {
  NONE,    // No RELRO in the LOAD segment
  PREFIX,  // RELRO is a prefix of the LOAD segment
  MIDDLE,  // RELRO is contained in the middle of the LOAD segment
  SUFFIX,  // RELRO is a suffix of the LOAD segment
  ENTIRE,  // RELRO is the entire LOAD segment
  ERROR,   // The relro size invalid (spans multiple segments?)
};

struct segment {
  const ElfW(Phdr)* phdr;
  relro_pos_t relro_pos;
};

static inline relro_pos_t relro_pos(const ElfW(Phdr)* phdr, const ElfW(Phdr)* relro) {
  // For checking the relro boundaries we use instead the LOAD segment's p_align
  // instead of the system or compat page size.
  uint64_t align = phdr->p_align;
  uint64_t seg_start = __builtin_align_down(phdr->p_vaddr, align);
  uint64_t seg_end = __builtin_align_up(phdr->p_vaddr + phdr->p_memsz, align);
  uint64_t relro_start = __builtin_align_down(relro->p_vaddr, align);
  uint64_t relro_end = __builtin_align_up(relro->p_vaddr + relro->p_memsz, align);

  if (relro_end <= seg_start || relro_start >= seg_end) return NONE;

  // Spans multiple LOAD segments?
  if (relro_start < seg_start || relro_end > seg_end) return ERROR;

  // Prefix or entire?
  if (relro_start == seg_start) return (relro_end < seg_end) ? PREFIX : ENTIRE;

  // Must be suffix or middle
  return (relro_end == seg_end) ? SUFFIX : MIDDLE;
}

static std::vector<struct segment> elf_segments(const ElfW(Phdr)* phdr_table, size_t phdr_count) {
  std::vector<struct segment> segments;

  for (size_t index = 0; index < phdr_count; ++index) {
    const ElfW(Phdr)* phdr = &phdr_table[index];

    if (phdr->p_type != PT_LOAD) continue;

    struct segment segment = {
        .phdr = phdr,
        .relro_pos = NONE,
    };

    segments.emplace_back(segment);
  }

  for (size_t index = 0; index < phdr_count; ++index) {
    const ElfW(Phdr)* relro = &phdr_table[index];

    if (relro->p_type != PT_GNU_RELRO) continue;

    for (struct segment& segment : segments) {
      if (segment.relro_pos != NONE) continue;

      segment.relro_pos = relro_pos(segment.phdr, relro);
    }
  }

  // Sort by vaddr
  std::sort(segments.begin(), segments.end(), [](const struct segment& a, const struct segment& b) {
    return a.phdr->p_vaddr < b.phdr->p_vaddr;
  });

  return segments;
}

static inline std::string prot_str(const struct segment& segment) {
  int prot = PFLAGS_TO_PROT(segment.phdr->p_flags);
  std::string str;

  if (prot & PROT_READ) str += "R";
  if (prot & PROT_WRITE) str += "W";
  if (prot & PROT_EXEC) str += "X";

  return str;
}

static inline std::string relro_pos_str(const struct segment& segment) {
  relro_pos_t relro_pos = segment.relro_pos;

  switch (relro_pos) {
    case NONE:
      return "";
    case PREFIX:
      return "(PREFIX)";
    case MIDDLE:
      return "(MIDDLE)";
    case SUFFIX:
      return "(SUFFIX)";
    case ENTIRE:
      return "(ENTIRE)";
    case ERROR:
      return "(ERROR)";
  }

  // Unreachable
  abort();
}

static inline std::string segment_format(const struct segment& segment) {
  uint64_t align_kbytes = segment.phdr->p_align / 1024;
  std::string format = prot_str(segment);

  if (segment.relro_pos != NONE) format += " " + relro_pos_str(segment);

  return format + " " + std::to_string(align_kbytes) + "K";
}

/*
 * Returns a string representing the ELF's load segment layout.
 *
 * Each segment has the format: <permissions> [(<relro position>)] <p_align>
 *
 *   e.g. "RX 4K|RW (ENTIRE) 4K|RW 4K|RW 16K|RX 16K|R 16K|RW 16K"
 */
static inline std::string elf_layout(const ElfW(Phdr)* phdr_table, size_t phdr_count) {
  std::vector<struct segment> segments = elf_segments(phdr_table, phdr_count);
  std::vector<std::string> layout;

  for (struct segment& segment : segments) {
    layout.emplace_back(segment_format(segment));
  }

  if (layout.empty()) return "";

  return std::accumulate(std::next(layout.begin()), layout.end(), layout[0],
                         [](std::string a, std::string b) { return std::move(a) + "," + b; });
}

void ElfReader::LabelCompatVma() {
  // Label the ELF VMA, since compat mode uses anonymous mappings, and some applications may rely on
  // them having their name set to the ELF's path.
  // Since kernel 5.10 it is safe to use non-global storage for the VMA name because it will be
  // copied into the kernel. 16KiB pages require a minimum kernel version of 6.1 so we can safely
  // use a stack-allocated buffer here.
  char vma_name_buffer[kVmaNameLimit] = {};
  format_left_truncated_vma_anon_name(vma_name_buffer, sizeof(vma_name_buffer),
                                      "16k:", name_.c_str(), "");
  if (prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, load_start_, load_size_, vma_name_buffer) != 0) {
    DL_WARN("\"%s\": Failed to rename 16KiB compat segment: %m", name_.c_str());
  }
}

void ElfReader::SetupRXRWAppCompat(ElfW(Addr) rx_rw_boundary) {
  // Adjust the load_bias to position the RX|RW boundary on a page boundary
  load_bias_ += perm_boundary_offset(rx_rw_boundary);

  // RW region (.data, .bss ...)
  ElfW(Addr) rw_start = load_bias_ + rx_rw_boundary;
  CHECK(rw_start % page_size() == 0);

  // Compat code and RELRO (RX) region (.text, .data.relro, ...)
  compat_code_start_ = load_start();
  compat_code_size_ = rw_start - load_start();
}

void ElfReader::SetupRWXAppCompat() {
  // Warn and fallback to RWX mapping
  const std::string layout = elf_layout(phdr_table_, phdr_num_);
  DL_WARN("\"%s\": RX|RW compat loading failed, falling back to RWX compat: load segments [%s]",
          name_.c_str(), layout.c_str());
  compat_code_start_ = load_start();
  compat_code_size_ = load_size();
}

bool ElfReader::Setup16KiBAppCompat() {
  if (!should_use_16kib_app_compat_) {
    return true;
  }

  ElfW(Addr) rx_rw_boundary;  // Permission boundary for RX|RW compat mode
  if (IsEligibleForRXRWAppCompat(&rx_rw_boundary)) {
    SetupRXRWAppCompat(rx_rw_boundary);
  } else {
    should_16kib_app_compat_use_rwx_ = true;
    SetupRWXAppCompat();
  }

  LabelCompatVma();
  return true;
}

bool ElfReader::CompatMapSegment(size_t seg_idx, size_t len) {
  const ElfW(Phdr)* phdr = &phdr_table_[seg_idx];

  // NOTE: The compat(legacy) page size (4096) must be used when aligning
  // the 4KiB segments for loading (reading). The larger 16KiB page size
  // will lead to overwriting adjacent segments since the ELF's segment(s)
  // are not 16KiB aligned.

  void* start = reinterpret_cast<void*>(__builtin_align_down(phdr->p_vaddr + load_bias_, kCompatPageSize));

  // The ELF could be being loaded directly from a zipped APK,
  // the zip offset must be added to find the segment offset.
  const ElfW(Addr) offset = file_offset_ + __builtin_align_down(phdr->p_offset, kCompatPageSize);

  CHECK(should_use_16kib_app_compat_);

  // Since the 4KiB max-page-size ELF is not properly aligned, loading it by
  // directly mmapping the ELF file is not feasible.
  // Instead, read the ELF contents into the anonymous RW mapping.
  if (TEMP_FAILURE_RETRY(pread64(fd_, start, len, offset)) == -1) {
    DL_ERR("Compat loading: \"%s\" failed to read LOAD segment %zu: %m", name_.c_str(), seg_idx);
    return false;
  }

  return true;
}

static size_t phdr_table_get_relro_min_align(const ElfW(Phdr)* relro_phdr,
                                             const ElfW(Phdr)* phdr_table, size_t phdr_count) {
  for (size_t index = 0; index < phdr_count; ++index) {
    const ElfW(Phdr)* phdr = &phdr_table[index];

    if (phdr->p_type != PT_LOAD) {
      continue;
    }

    // Only check for the case, where the relro segment is a prefix of a load segment. Conventional
    // linkers will only generate binaries where the relro segment is either the prefix of the first
    // RW load segment, or is entirely contained in the first RW segment.
    if (phdr->p_vaddr == relro_phdr->p_vaddr) {
      // No extra alignment checks needed if the whole load segment is relro.
      if (phdr->p_memsz <= relro_phdr->p_memsz) {
        return 0;
      }

      ElfW(Addr) relro_end = relro_phdr->p_vaddr + relro_phdr->p_memsz;
      // Alignments must be powers of two, so the RELRO segment's alignment can be determined by
      // calculating its lowest set bit with (n & -n).
      size_t relro_align = static_cast<size_t>(relro_end & -relro_end);
      // We only care about relro segments that are aligned to at least 4KiB. This is always
      // expected for outputs of a conventional linker.
      return relro_align >= kCompatPageSize ? relro_align : 0;
    }
  }
  return 0;
}

/*
 * In the base page size is 16KiB and the RELRO's end alignment is less than min_align_;
 *  override min_align_ with the relro's end alignment. This ensures that the ELF is
 * loaded in compat mode even if the LOAD segments are 16KB aligned.
 * Linker bug: https://sourceware.org/bugzilla/show_bug.cgi?id=28824
 */
void ElfReader::FixMinAlignFor16KiB() {
  // A binary with LOAD segment alignments of at least 16KiB can still be incompatible with 16KiB
  // page sizes if the first RW segment has a RELRO prefix ending at a non-16KiB-aligned address. We
  // need to check for this possibility here and adjust min_align_ accordingly.
  // We only check if the ELF file contains a single RELRO segment, because that's what the 16KiB
  // compatibility loader can handle.
  const ElfW(Phdr)* relro_phdr = nullptr;
  if (HasAtMostOneRelroSegment(&relro_phdr) && relro_phdr != nullptr) {
    size_t relro_min_align = phdr_table_get_relro_min_align(relro_phdr, phdr_table_, phdr_num_);
    if (relro_min_align) {
      min_align_ = std::min(min_align_, relro_min_align);
    }
  }
}

/*
 * Apply RX or RWX protection to the code region of the ELF being loaded in
 * 16KiB compat mode.
 *
 * Input:
 *   start                            -> start address of the compat code region.
 *   size                             -> size of the compat code region in bytes.
 *   should_16kib_app_compat_use_rwx  -> use RWX or RX permission.
 *   note_gnu_property                -> AArch64-only: use PROT_BTI if the ELF is BTI-compatible.
 * Return:
 *   0 on success, -1 on failure (error code in errno).
 */
int phdr_table_protect_16kib_app_compat_code(ElfW(Addr) start, ElfW(Addr) size,
                                             bool should_16kib_app_compat_use_rwx,
                                             const GnuPropertySection* note_gnu_property __unused) {
  int prot = PROT_READ | PROT_EXEC;
  if (should_16kib_app_compat_use_rwx) {
    prot |= PROT_WRITE;
  }
#ifdef __aarch64__
  if (note_gnu_property != nullptr && note_gnu_property->IsBTICompatible()) {
    prot |= PROT_BTI;
  }
#endif
  return mprotect(reinterpret_cast<void*>(start), size, prot);
}
