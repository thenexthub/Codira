/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

import {
    tsInt,
    Obj
} from 'main_js';

function checkOptionalOperatorWithValue(): boolean {
    return Obj?.name?.toString() == Obj.name?.toString();
}

function checkOptionalOperatorReturnUndefined(): boolean {
    const res: undefined | Object = Obj?.test as undefined | Object;
    
    return res == undefined;
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
// function check_optional_operator_with_value_by_string_index(): boolean {
//     return Obj?.['name'] as string == Obj.name as string;
// }

// function checkOptionalOperatorReturnUndefined_by_string_index(): boolean {
//     const res: undefined = Obj?.['test'] as undefined;

//     return res == undefined;
// }

function checkOptionalOperatorInsideObjectWithValue(): boolean {
    return Obj.inside?.name?.toString() == Obj.inside.name?.toString();
}

function checkOptionalOperatorInsideObjectReturnUndefined(): boolean {
    const res: undefined | Object = Obj.inside?.test as undefined | Object;

    return res == undefined;
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
// function checkOptionalOperatorInsideObjectWithValueByStringIndex(): boolean {
//     return Obj.inside?.['name'] as string == Obj.name as string;
// }

// function checkOptionalOperatorInsideReturnUndefinedByStringIndex(): boolean {
//     const res: undefined = Obj.inside?.['test'] as undefined;

//     return res == undefined;
// }
// NOTE  (alexanderpolenov) issue(18447) enable after fix call function with ?. operator
// function checkOptionalOperatorFunction(): boolean {
//     return Obj?.sum() as int == tsInt as int;
// }

function checkOptionalOperatorFunctionUndefined(): boolean {
    try{
        // Import Obj from 1.1. It is strict type checking in 1.2, and there is no test in obj.
        // Obj?.test();
        
        return false;
    }catch(e){
        return true;
    }
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
// function checkOptionalOperatorFunctionByStringIndex(): boolean {
//     return Obj?.['sum']() as int == tsInt as int;
// }

// function checkOptionalOperatorFunctionReturnUndefinedByStringIndex(): boolean {
//     try{
//         Obj?.['test']();
        
//         return false;
//     }catch(e){
//         return true;
//     }
// }
