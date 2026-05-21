#ifndef AGENT_UNIFY_H
#define AGENT_UNIFY_H

#include "type_core.h"

typedef enum {
  Z_UNIFY_BINDING_TYPE,
  Z_UNIFY_BINDING_STATIC
} ZUnifyBindingKind;

typedef struct {
  ZTypeBinderId binder;
  ZUnifyBindingKind kind;
  char *name;
  ZTypeId type;
  ZStaticValue static_value;
} ZUnifyBinding;

typedef struct {
  ZUnifyBinding *items;
  size_t len;
  size_t cap;
  char message[160];
} ZUnifyTrace;

void z_unify_trace_init(ZUnifyTrace *trace);
void z_unify_trace_free(ZUnifyTrace *trace);
const ZUnifyBinding *z_unify_trace_lookup(const ZUnifyTrace *trace, ZTypeBinderId binder, ZUnifyBindingKind kind);

bool z_type_occurs(const ZTypeArena *arena, ZTypeId type, ZTypeBinderId binder);
bool z_type_substitute(ZTypeArena *arena, ZTypeId source, const ZUnifyTrace *trace, ZTypeId *out);
bool z_type_unify(ZTypeArena *arena, ZTypeId pattern, ZTypeId actual, ZUnifyTrace *trace);

#endif
