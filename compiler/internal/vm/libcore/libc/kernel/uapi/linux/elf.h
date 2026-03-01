/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_ELF_H
#define _UAPI_LINUX_ELF_H
#include <linux/types.h>
#include <linux/elf-em.h>
typedef __u32 Elf32_Addr;
typedef __u16 Elf32_Half;
typedef __u32 Elf32_Off;
typedef __s32 Elf32_Sword;
typedef __u32 Elf32_Word;
typedef __u16 Elf32_Versym;
typedef __u64 Elf64_Addr;
typedef __u16 Elf64_Half;
typedef __s16 Elf64_SHalf;
typedef __u64 Elf64_Off;
typedef __s32 Elf64_Sword;
typedef __u32 Elf64_Word;
typedef __u64 Elf64_Xword;
typedef __s64 Elf64_Sxword;
typedef __u16 Elf64_Versym;
#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7
#define PT_LOOS 0x60000000
#define PT_HIOS 0x6fffffff
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff
#define PT_GNU_EH_FRAME (PT_LOOS + 0x474e550)
#define PT_GNU_STACK (PT_LOOS + 0x474e551)
#define PT_GNU_RELRO (PT_LOOS + 0x474e552)
#define PT_GNU_PROPERTY (PT_LOOS + 0x474e553)
#define PT_AARCH64_MEMTAG_MTE (PT_LOPROC + 0x2)
#define PN_XNUM 0xffff
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff
#define DT_NULL 0
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH 4
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_ENCODING 32
#define OLD_DT_LOOS 0x60000000
#define DT_LOOS 0x6000000d
#define DT_HIOS 0x6ffff000
#define DT_VALRNGLO 0x6ffffd00
#define DT_VALRNGHI 0x6ffffdff
#define DT_ADDRRNGLO 0x6ffffe00
#define DT_GNU_HASH 0x6ffffef5
#define DT_ADDRRNGHI 0x6ffffeff
#define DT_VERSYM 0x6ffffff0
#define DT_RELACOUNT 0x6ffffff9
#define DT_RELCOUNT 0x6ffffffa
#define DT_FLAGS_1 0x6ffffffb
#define DT_VERDEF 0x6ffffffc
#define DT_VERDEFNUM 0x6ffffffd
#define DT_VERNEED 0x6ffffffe
#define DT_VERNEEDNUM 0x6fffffff
#define OLD_DT_HIOS 0x6fffffff
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7fffffff
#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STN_UNDEF 0
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_COMMON 5
#define STT_TLS 6
#define VER_FLG_BASE 0x1
#define VER_FLG_WEAK 0x2
#define ELF_ST_BIND(x) ((x) >> 4)
#define ELF_ST_TYPE(x) ((x) & 0xf)
#define ELF32_ST_BIND(x) ELF_ST_BIND(x)
#define ELF32_ST_TYPE(x) ELF_ST_TYPE(x)
#define ELF64_ST_BIND(x) ELF_ST_BIND(x)
#define ELF64_ST_TYPE(x) ELF_ST_TYPE(x)
typedef struct {
  Elf32_Sword d_tag;
  union {
    Elf32_Sword d_val;
    Elf32_Addr d_ptr;
  } d_un;
} Elf32_Dyn;
typedef struct {
  Elf64_Sxword d_tag;
  union {
    Elf64_Xword d_val;
    Elf64_Addr d_ptr;
  } d_un;
} Elf64_Dyn;
#define ELF32_R_SYM(x) ((x) >> 8)
#define ELF32_R_TYPE(x) ((x) & 0xff)
#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)
typedef struct elf32_rel {
  Elf32_Addr r_offset;
  Elf32_Word r_info;
} Elf32_Rel;
typedef struct elf64_rel {
  Elf64_Addr r_offset;
  Elf64_Xword r_info;
} Elf64_Rel;
typedef struct elf32_rela {
  Elf32_Addr r_offset;
  Elf32_Word r_info;
  Elf32_Sword r_addend;
} Elf32_Rela;
typedef struct elf64_rela {
  Elf64_Addr r_offset;
  Elf64_Xword r_info;
  Elf64_Sxword r_addend;
} Elf64_Rela;
typedef struct elf32_sym {
  Elf32_Word st_name;
  Elf32_Addr st_value;
  Elf32_Word st_size;
  unsigned char st_info;
  unsigned char st_other;
  Elf32_Half st_shndx;
} Elf32_Sym;
typedef struct elf64_sym {
  Elf64_Word st_name;
  unsigned char st_info;
  unsigned char st_other;
  Elf64_Half st_shndx;
  Elf64_Addr st_value;
  Elf64_Xword st_size;
} Elf64_Sym;
#define EI_NIDENT 16
typedef struct elf32_hdr {
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry;
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
} Elf32_Ehdr;
typedef struct elf64_hdr {
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;
#define PF_R 0x4
#define PF_W 0x2
#define PF_X 0x1
typedef struct elf32_phdr {
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;
typedef struct elf64_phdr {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;
  Elf64_Addr p_vaddr;
  Elf64_Addr p_paddr;
  Elf64_Xword p_filesz;
  Elf64_Xword p_memsz;
  Elf64_Xword p_align;
} Elf64_Phdr;
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_NUM 12
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_LINK_ORDER 0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_RELA_LIVEPATCH 0x00100000
#define SHF_RO_AFTER_INIT 0x00200000
#define SHF_ORDERED 0x04000000
#define SHF_EXCLUDE 0x08000000
#define SHF_MASKOS 0x0ff00000
#define SHF_MASKPROC 0xf0000000
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_LIVEPATCH 0xff20
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff
typedef struct elf32_shdr {
  Elf32_Word sh_name;
  Elf32_Word sh_type;
  Elf32_Word sh_flags;
  Elf32_Addr sh_addr;
  Elf32_Off sh_offset;
  Elf32_Word sh_size;
  Elf32_Word sh_link;
  Elf32_Word sh_info;
  Elf32_Word sh_addralign;
  Elf32_Word sh_entsize;
} Elf32_Shdr;
typedef struct elf64_shdr {
  Elf64_Word sh_name;
  Elf64_Word sh_type;
  Elf64_Xword sh_flags;
  Elf64_Addr sh_addr;
  Elf64_Off sh_offset;
  Elf64_Xword sh_size;
  Elf64_Word sh_link;
  Elf64_Word sh_info;
  Elf64_Xword sh_addralign;
  Elf64_Xword sh_entsize;
} Elf64_Shdr;
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_PAD 8
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFMAG "\177ELF"
#define SELFMAG 4
#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2
#define ELFCLASSNUM 3
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2
#define EV_NONE 0
#define EV_CURRENT 1
#define EV_NUM 2
#define ELFOSABI_NONE 0
#define ELFOSABI_LINUX 3
#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif
#define NN_GNU_PROPERTY_TYPE_0 "GNU"
#define NT_GNU_PROPERTY_TYPE_0 5
#define NN_PRSTATUS "CORE"
#define NT_PRSTATUS 1
#define NN_PRFPREG "CORE"
#define NT_PRFPREG 2
#define NN_PRPSINFO "CORE"
#define NT_PRPSINFO 3
#define NN_TASKSTRUCT "CORE"
#define NT_TASKSTRUCT 4
#define NN_AUXV "CORE"
#define NT_AUXV 6
#define NN_SIGINFO "CORE"
#define NT_SIGINFO 0x53494749
#define NN_FILE "CORE"
#define NT_FILE 0x46494c45
#define NN_PRXFPREG "LINUX"
#define NT_PRXFPREG 0x46e62b7f
#define NN_PPC_VMX "LINUX"
#define NT_PPC_VMX 0x100
#define NN_PPC_SPE "LINUX"
#define NT_PPC_SPE 0x101
#define NN_PPC_VSX "LINUX"
#define NT_PPC_VSX 0x102
#define NN_PPC_TAR "LINUX"
#define NT_PPC_TAR 0x103
#define NN_PPC_PPR "LINUX"
#define NT_PPC_PPR 0x104
#define NN_PPC_DSCR "LINUX"
#define NT_PPC_DSCR 0x105
#define NN_PPC_EBB "LINUX"
#define NT_PPC_EBB 0x106
#define NN_PPC_PMU "LINUX"
#define NT_PPC_PMU 0x107
#define NN_PPC_TM_CGPR "LINUX"
#define NT_PPC_TM_CGPR 0x108
#define NN_PPC_TM_CFPR "LINUX"
#define NT_PPC_TM_CFPR 0x109
#define NN_PPC_TM_CVMX "LINUX"
#define NT_PPC_TM_CVMX 0x10a
#define NN_PPC_TM_CVSX "LINUX"
#define NT_PPC_TM_CVSX 0x10b
#define NN_PPC_TM_SPR "LINUX"
#define NT_PPC_TM_SPR 0x10c
#define NN_PPC_TM_CTAR "LINUX"
#define NT_PPC_TM_CTAR 0x10d
#define NN_PPC_TM_CPPR "LINUX"
#define NT_PPC_TM_CPPR 0x10e
#define NN_PPC_TM_CDSCR "LINUX"
#define NT_PPC_TM_CDSCR 0x10f
#define NN_PPC_PKEY "LINUX"
#define NT_PPC_PKEY 0x110
#define NN_PPC_DEXCR "LINUX"
#define NT_PPC_DEXCR 0x111
#define NN_PPC_HASHKEYR "LINUX"
#define NT_PPC_HASHKEYR 0x112
#define NN_386_TLS "LINUX"
#define NT_386_TLS 0x200
#define NN_386_IOPERM "LINUX"
#define NT_386_IOPERM 0x201
#define NN_X86_XSTATE "LINUX"
#define NT_X86_XSTATE 0x202
#define NN_X86_SHSTK "LINUX"
#define NT_X86_SHSTK 0x204
#define NN_X86_XSAVE_LAYOUT "LINUX"
#define NT_X86_XSAVE_LAYOUT 0x205
#define NN_S390_HIGH_GPRS "LINUX"
#define NT_S390_HIGH_GPRS 0x300
#define NN_S390_TIMER "LINUX"
#define NT_S390_TIMER 0x301
#define NN_S390_TODCMP "LINUX"
#define NT_S390_TODCMP 0x302
#define NN_S390_TODPREG "LINUX"
#define NT_S390_TODPREG 0x303
#define NN_S390_CTRS "LINUX"
#define NT_S390_CTRS 0x304
#define NN_S390_PREFIX "LINUX"
#define NT_S390_PREFIX 0x305
#define NN_S390_LAST_BREAK "LINUX"
#define NT_S390_LAST_BREAK 0x306
#define NN_S390_SYSTEM_CALL "LINUX"
#define NT_S390_SYSTEM_CALL 0x307
#define NN_S390_TDB "LINUX"
#define NT_S390_TDB 0x308
#define NN_S390_VXRS_LOW "LINUX"
#define NT_S390_VXRS_LOW 0x309
#define NN_S390_VXRS_HIGH "LINUX"
#define NT_S390_VXRS_HIGH 0x30a
#define NN_S390_GS_CB "LINUX"
#define NT_S390_GS_CB 0x30b
#define NN_S390_GS_BC "LINUX"
#define NT_S390_GS_BC 0x30c
#define NN_S390_RI_CB "LINUX"
#define NT_S390_RI_CB 0x30d
#define NN_S390_PV_CPU_DATA "LINUX"
#define NT_S390_PV_CPU_DATA 0x30e
#define NN_ARM_VFP "LINUX"
#define NT_ARM_VFP 0x400
#define NN_ARM_TLS "LINUX"
#define NT_ARM_TLS 0x401
#define NN_ARM_HW_BREAK "LINUX"
#define NT_ARM_HW_BREAK 0x402
#define NN_ARM_HW_WATCH "LINUX"
#define NT_ARM_HW_WATCH 0x403
#define NN_ARM_SYSTEM_CALL "LINUX"
#define NT_ARM_SYSTEM_CALL 0x404
#define NN_ARM_SVE "LINUX"
#define NT_ARM_SVE 0x405
#define NN_ARM_PAC_MASK "LINUX"
#define NT_ARM_PAC_MASK 0x406
#define NN_ARM_PACA_KEYS "LINUX"
#define NT_ARM_PACA_KEYS 0x407
#define NN_ARM_PACG_KEYS "LINUX"
#define NT_ARM_PACG_KEYS 0x408
#define NN_ARM_TAGGED_ADDR_CTRL "LINUX"
#define NT_ARM_TAGGED_ADDR_CTRL 0x409
#define NN_ARM_PAC_ENABLED_KEYS "LINUX"
#define NT_ARM_PAC_ENABLED_KEYS 0x40a
#define NN_ARM_SSVE "LINUX"
#define NT_ARM_SSVE 0x40b
#define NN_ARM_ZA "LINUX"
#define NT_ARM_ZA 0x40c
#define NN_ARM_ZT "LINUX"
#define NT_ARM_ZT 0x40d
#define NN_ARM_FPMR "LINUX"
#define NT_ARM_FPMR 0x40e
#define NN_ARM_POE "LINUX"
#define NT_ARM_POE 0x40f
#define NN_ARM_GCS "LINUX"
#define NT_ARM_GCS 0x410
#define NN_ARC_V2 "LINUX"
#define NT_ARC_V2 0x600
#define NN_VMCOREDD "LINUX"
#define NT_VMCOREDD 0x700
#define NN_MIPS_DSP "LINUX"
#define NT_MIPS_DSP 0x800
#define NN_MIPS_FP_MODE "LINUX"
#define NT_MIPS_FP_MODE 0x801
#define NN_MIPS_MSA "LINUX"
#define NT_MIPS_MSA 0x802
#define NN_RISCV_CSR "LINUX"
#define NT_RISCV_CSR 0x900
#define NN_RISCV_VECTOR "LINUX"
#define NT_RISCV_VECTOR 0x901
#define NN_RISCV_TAGGED_ADDR_CTRL "LINUX"
#define NT_RISCV_TAGGED_ADDR_CTRL 0x902
#define NN_LOONGARCH_CPUCFG "LINUX"
#define NT_LOONGARCH_CPUCFG 0xa00
#define NN_LOONGARCH_CSR "LINUX"
#define NT_LOONGARCH_CSR 0xa01
#define NN_LOONGARCH_LSX "LINUX"
#define NT_LOONGARCH_LSX 0xa02
#define NN_LOONGARCH_LASX "LINUX"
#define NT_LOONGARCH_LASX 0xa03
#define NN_LOONGARCH_LBT "LINUX"
#define NT_LOONGARCH_LBT 0xa04
#define NN_LOONGARCH_HW_BREAK "LINUX"
#define NT_LOONGARCH_HW_BREAK 0xa05
#define NN_LOONGARCH_HW_WATCH "LINUX"
#define NT_LOONGARCH_HW_WATCH 0xa06
typedef struct elf32_note {
  Elf32_Word n_namesz;
  Elf32_Word n_descsz;
  Elf32_Word n_type;
} Elf32_Nhdr;
typedef struct elf64_note {
  Elf64_Word n_namesz;
  Elf64_Word n_descsz;
  Elf64_Word n_type;
} Elf64_Nhdr;
#define GNU_PROPERTY_AARCH64_FEATURE_1_AND 0xc0000000
#define GNU_PROPERTY_AARCH64_FEATURE_1_BTI (1U << 0)
typedef struct {
  Elf32_Half vd_version;
  Elf32_Half vd_flags;
  Elf32_Half vd_ndx;
  Elf32_Half vd_cnt;
  Elf32_Word vd_hash;
  Elf32_Word vd_aux;
  Elf32_Word vd_next;
} Elf32_Verdef;
typedef struct {
  Elf64_Half vd_version;
  Elf64_Half vd_flags;
  Elf64_Half vd_ndx;
  Elf64_Half vd_cnt;
  Elf64_Word vd_hash;
  Elf64_Word vd_aux;
  Elf64_Word vd_next;
} Elf64_Verdef;
typedef struct {
  Elf32_Word vda_name;
  Elf32_Word vda_next;
} Elf32_Verdaux;
typedef struct {
  Elf64_Word vda_name;
  Elf64_Word vda_next;
} Elf64_Verdaux;
#endif
