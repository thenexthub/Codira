#include <Foundation/Foundation.h>

// Note: 'Array' is declared 'id' instead of 'NSArray' to avoid automatic
// bridging to 'Codira.Array' when this API is imported to Codira.
void slurpFastEnumerationOfArrayFromObjCImpl(id Array, id<NSFastEnumeration> FE,
                                             NSMutableArray *Values);

// Note: 'Dictionary' is declared 'id' instead of 'NSDictionary' to avoid
// automatic bridging to 'Codira.Dictionary' when this API is imported to Codira.
void slurpFastEnumerationOfDictionaryFromObjCImpl(
    id Dictionary, id<NSFastEnumeration> FE, NSMutableArray *KeyValuePairs);

