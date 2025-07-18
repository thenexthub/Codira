#import <objc/runtime.h>

#import "Cast_Blocks.h"

BOOL ObjCThinksObjectIsCodiraValue(id obj) {
  Class cls = object_getClass(obj);
  const char *name = class_getName(cls);
  if (strcmp(name, "__CodiraValue") == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

void ObjCCanCallBlock(id block_as_id) {
  typedef void(^blockType)(void);
  blockType block = (blockType)block_as_id;
  block();
}
