/*    $OpenBSD: arc4random_linux.h,v 1.7 2014/07/20 20:51:13 bcook Exp $    */

/*
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 * Copyright (c) 2013, Markus Friedl <markus@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <pthread.h>
#include <sys/mman.h>
#include <sys/prctl.h>

#include <async_safe/log.h>

static pthread_mutex_t g_arc4random_mutex = PTHREAD_MUTEX_INITIALIZER;

void arc4random_mutex_lock() {
  pthread_mutex_lock(&g_arc4random_mutex);
}

void arc4random_mutex_unlock() {
  pthread_mutex_unlock(&g_arc4random_mutex);
}

#define _ARC4_LOCK() arc4random_mutex_lock()
#define _ARC4_UNLOCK() arc4random_mutex_unlock()

static inline void _getentropy_fail(void) {
  async_safe_fatal("getentropy failed: %m");
}

static inline void _rs_forkdetect(void) {
  // Not needed thanks to the MADV_WIPEONFORK below.
}

static inline int _rs_allocate(struct _rs** rsp, struct _rsx** rsxp) {
  // OpenBSD's arc4random_linux.h allocates two separate mappings, but for
  // themselves they just allocate both structs into one mapping like this.
  struct data {
    struct _rs rs;
    struct _rsx rsx;
  };
  const size_t size = sizeof(struct data);

  struct data* p = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
  if (p == MAP_FAILED) {
    async_safe_fatal("arc4random data allocation failed: %m");
  }

  // Equivalent to OpenBSD's minherit(MAP_INHERIT_ZERO).
  if (madvise(p, size, MADV_WIPEONFORK) == -1) {
    async_safe_fatal("arc4random data MADV_WIPEONFORK failed: %m");
  }

  // Give the allocation a name to make tombstones more intelligible.
  prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, p, size, "arc4random data");

  *rsp = &p->rs;
  *rsxp = &p->rsx;
  return 0;
}
