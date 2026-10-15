/*
 * Copyright (c) 2026 Omnira CJSC
 * Author: Tunjay Akbarli
 * Date: October 15, 2026
 *
 * Functionality:
 * - Loads an ELF shared object with dlopen and prints what it looks like once
 *   the dynamic linker has applied its relocations, in a form that does not
 *   depend on where the linker put things: every data symbol's words, with
 *   the words that point into the library written as symbol+offset, and the
 *   result of calling every exported function (in a child process, with fixed
 *   arguments, so a crash is reported instead of ending the run).
 *
 * The differential tests link the same objects with LLD and with Funis, run
 * this on both outputs on a Linux host (WSL on Windows), and require the two
 * listings to be identical.
 *
 *   cc -O1 -o loadcheck loadcheck.c -ldl && ./loadcheck lib.so
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

struct sym {
    const char *name;
    uint64_t value, size;
    int is_func, is_object, dynamic;
};

static struct sym *syms;
static size_t nsyms;

static int by_value(const void *a, const void *b) {
    const struct sym *x = a, *y = b;
    if (x->value != y->value) return x->value < y->value ? -1 : 1;
    return strcmp(x->name, y->name);
}

static void add_symbols(const unsigned char *file, const Elf64_Shdr *sh, int dynamic) {
    const Elf64_Sym *tab = (const Elf64_Sym *)(file + sh->sh_offset);
    const Elf64_Shdr *strsec = (const Elf64_Shdr *)(file + ((const Elf64_Ehdr *)file)->e_shoff) + sh->sh_link;
    const char *str = (const char *)(file + strsec->sh_offset);
    size_t n = sh->sh_size / sizeof(Elf64_Sym);
    for (size_t i = 1; i < n; i++) {
        const Elf64_Sym *s = &tab[i];
        int type = ELF64_ST_TYPE(s->st_info);
        if (s->st_shndx == SHN_UNDEF || s->st_shndx >= SHN_LORESERVE || s->st_name == 0) continue;
        if (type != STT_FUNC && type != STT_OBJECT) continue;
        if (dynamic == 0) {
            /* A symbol in both tables is listed once, from .dynsym. */
        }
        syms = realloc(syms, (nsyms + 1) * sizeof *syms);
        syms[nsyms++] = (struct sym){str + s->st_name, s->st_value, s->st_size, type == STT_FUNC,
                                     type == STT_OBJECT, dynamic};
    }
}

static const struct sym *find(uint64_t off, uint64_t *delta) {
    const struct sym *best = NULL;
    for (size_t i = 0; i < nsyms; i++) {
        if (!syms[i].dynamic) continue;
        if (syms[i].value <= off && off < syms[i].value + (syms[i].size ? syms[i].size : 1)) {
            best = &syms[i];
            break;
        }
    }
    if (best) *delta = off - best->value;
    return best;
}

static uint64_t image_extent(const unsigned char *file) {
    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)file;
    const Elf64_Phdr *ph = (const Elf64_Phdr *)(file + eh->e_phoff);
    uint64_t end = 0;
    for (int i = 0; i < eh->e_phnum; i++)
        if (ph[i].p_type == PT_LOAD && ph[i].p_vaddr + ph[i].p_memsz > end) end = ph[i].p_vaddr + ph[i].p_memsz;
    return end;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s lib.so\n", argv[0]);
        return 2;
    }
    int fd = open(argv[1], O_RDONLY);
    struct stat st;
    if (fd < 0 || fstat(fd, &st) < 0) {
        perror(argv[1]);
        return 2;
    }
    unsigned char *file = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)file;
    const Elf64_Shdr *sh = (const Elf64_Shdr *)(file + eh->e_shoff);
    for (int i = 0; i < eh->e_shnum; i++)
        if (sh[i].sh_type == SHT_DYNSYM) add_symbols(file, &sh[i], 1);
    qsort(syms, nsyms, sizeof *syms, by_value);

    void *handle = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        printf("dlopen failed: %s\n", dlerror());
        return 1;
    }
    struct link_map *lm;
    dlinfo(handle, RTLD_DI_LINKMAP, &lm);
    uintptr_t base = lm->l_addr;
    uint64_t extent = image_extent(file);
    printf("loaded; image extent is not compared\n");

    for (size_t i = 0; i < nsyms; i++) {
        const struct sym *s = &syms[i];
        if (s->is_object) {
            printf("data %s size=%llu:", s->name, (unsigned long long)s->size);
            const unsigned char *p = (const unsigned char *)(base + s->value);
            size_t k = 0;
            for (; k + 8 <= s->size; k += 8) {
                uint64_t w;
                memcpy(&w, p + k, 8);
                uint64_t delta;
                const struct sym *t = (w >= base && w - base < extent) ? find(w - base, &delta) : NULL;
                if (t) printf(" &%s+%llu", t->name, (unsigned long long)delta);
                else if (w >= base && w - base < extent) {
                    /* Not at a symbol (a string literal, say): show the first bytes it points at. */
                    const unsigned char *q = (const unsigned char *)w;
                    uint64_t room = extent - (w - base);
                    printf(" &[");
                    for (uint64_t j = 0; j < 12 && j < room; j++) printf("%02x", q[j]);
                    printf("]");
                }
                else printf(" %016llx", (unsigned long long)w);
            }
            for (; k < s->size; k++) printf(" %02x", p[k]);
            printf("\n");
        }
    }
    fflush(stdout);

    for (size_t i = 0; i < nsyms; i++) {
        const struct sym *s = &syms[i];
        if (!s->is_func || strcmp(s->name, "set_allocator_handle") == 0) continue;
        int pipefd[2];
        if (pipe(pipefd) < 0) return 2;
        pid_t pid = fork();
        if (pid == 0) {
            alarm(2);
            close(pipefd[0]);
            typedef long (*ifn)(long, long, long, long, long, long, double, double, double, double);
            typedef double (*dfn)(long, long, long, long, long, long, double, double, double, double);
            void *f = (void *)(base + s->value);
            long r = ((ifn)f)(3, 5, 7, 11, 13, 17, 1.5, 2.5, 3.5, 4.5);
            double d = ((dfn)f)(3, 5, 7, 11, 13, 17, 1.5, 2.5, 3.5, 4.5);
            uint64_t bits;
            memcpy(&bits, &d, 8);
            long out[2] = {r, (long)bits};
            if (write(pipefd[1], out, sizeof out) < 0) _exit(3);
            _exit(0);
        }
        close(pipefd[1]);
        long out[2] = {0, 0};
        ssize_t got = read(pipefd[0], out, sizeof out);
        close(pipefd[0]);
        int status = 0;
        waitpid(pid, &status, 0);
        if (got == (ssize_t)sizeof out && WIFEXITED(status))
            printf("call %s: int=%ld fp=%016lx\n", s->name, out[0], out[1]);
        else if (WIFSIGNALED(status))
            printf("call %s: signal %d\n", s->name, WTERMSIG(status));
        else
            printf("call %s: exit %d\n", s->name, WEXITSTATUS(status));
    }
    return 0;
}
