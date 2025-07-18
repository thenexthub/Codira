#ifndef REFERENCE_COUNTED_OBJC_PROPERTY_H
#define REFERENCE_COUNTED_OBJC_PROPERTY_H

#include "reference-counted.h"

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

@interface C0
@property (nonnull, readonly) NS::LocalCount *lc;
- (instancetype)init;
@end

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#endif // REFERENCE_COUNTED_OBJC_PROPERTY_H
