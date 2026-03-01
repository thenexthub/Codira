# Expression balancing

## Overview 
**Expression balancing** - optimization that reorganises computation of algebraic expressions.  
The optimization is applied to expressions of the form of a long chain of the same binary associative and commutative operator, such as `ADD`, `MUL`, `AND`, `OR`, and etc. It calculates expression's critical path and, if it can be decreased, reorganises expression so it would be optimal. For example: critical path of `(((a + b) + c) + d)` is 3, whereas critical path of `((a + b) + (c + d))` is 2.

## Rationality
Increases instruction-level parallelism.

## Dependence 
* RPO analyze.

## Algorithm

Visit all basic blocks in PRO order.

For each block iterate over instructions in reverse order looking for suitable instruction (i.e. operator).

If such instruction is found, it is the last one in an expression and it is necessary to determine the whole chain and its critical path by recursive [analysis](#operator-analysis) of operator inputs (`lhs` and `rhs`).

If the critical path isn't [optimal](#optimal-critical-path), delete expression's operators from the basic block, allocate them to expression's terms in an [optimal way](#operators-allocation) and insert new operators in the basic block. 


### Note
#### Operator analysis
`Analysis of operator inputs` is a check if `lhs`(`rhs`) has the same opcode and has the only user (the operator itself). If so, the input is an operator of the expression, the analysis is called for it too and it should be saved to array of operators (`operators_`); otherwise it is an expression's term and it should be saved to array of terms (`sources_`).  
If inputs belong to different basic blocks but satisfy the conditions above (have single user and are arithmetic operations), they should be moved to a single (dominatee) basic block (similiar to CodeSink).

#### Optimal critical path
The `optimal critical path` of an expression is `ceil[log2(n_terms)]` (`ceil[x]` is rounding `x` up).

#### Operators allocation
`Allocation in an optimal way` is an algorithm that creates expression and guarantees that it would have an [optimal](#optimal-critical-path) critical path:  
Assume `terms[]` is an array of expression terms, algorithm called for two arguments `first_idx` and `last_idx` creates expression with terms in range from `terms[first_idx]` to `terms[last_idx]`.  
The algorithm is:  
- If range `first_idx:last_idx` covers `1` element then return this element.
- If range `first_idx:last_idx` covers `2` elements then create and return operator with `lhs` and `rhs` equal to `terms[first_idx]` and `terms[last_idx]`.
- Else calculate `split_idx` so that `split_idx` is strictly less than `last_idx` and size of `first_idx:split_idx` is the greatest possible power of 2, create and return operator with `lhs` and `rhs` equal to results of recursive calls  `allocate(first_idx, split_idx)` and `allocate(split_idx + 1, last_idx)`.


## Pseudocode
```
    for (auto basic_block : GetGraph()->GetBlocksRPO()) {
        for (auto instruction : basic_block->InstsReverse()) {
            if (instruction is a suitable operator) {

                // Recursively check and save inputs, determine critical_path_length;
                ...

                // Calculate optimal_critical_path_length;
                ...

                if (optimal < current) {
                    AllocateSources(0, size(terms[]) - 1);
                    insert new expression to the basic block;
                }
            }
        }
    }

    auto AllocateSources(size_t first_idx, size_t last_idx) {
        if (first_idx == last_idx) {
            return terms[first_idx];
        }
        if (first_idx == last_idx + 1) {
            auto operator = operators[free_op_idx++];
            operator->GetBasicBlock()->RemoveInst(operator);

            operator.lhs = terms[first_idx];
            operator.rhs = terms[last_idx];

            basic_block.Insert(operator);
            return operator;
        }
        else {
            size_t split_idx = calculate split_idx;
            auto lhs = AllocateSources(first_idx, split_idx);
            auto rhs = AllocateSources(split_idx + 1, last_idx);

            auto operator = operators[free_op_idx++];
            basic_block->RemoveInst(operator);

            operator.lhs = lhs;
            operator.rhs = rhs;

            basic_block.Insert(operator);
            return operator;
        }
    }

```
## Examples
Before expressions balancing:
```
0.i64 Parameter  -> v8      // a
1.i64 Parameter  -> v8      // b
2.i64 Parameter  -> v9      // c
3.i64 Parameter  -> v10     // d
4.i64 Parameter  -> v11     // e
5.i64 Parameter  -> v12     // f
6.i64 Parameter  -> v12     // g

8. Add v0, v1 -> v9         // a + b
9. Add v2, v8 -> v10        // c + (a + b)
// As soon as v10 has more than one users it has side-effects, so the algorithm considers it as a term:
10. Add v3, v9 -> v11, v14  // s10 = d + (c + (a + b))

11. Add v4, v10 -> v13      // e + s10
12. Add v5, v6 -> v13       // f + g
13. Add v11, v12 -> v14     // (e + s10) + (f + g)
14. Add v10, v13 -> v15     // s10 + ((e + s10) + (f + g))

15. Return v14
```
The code above contains two expressions: `v8-v10` (critical path is 3) and `v11-v14` (critical path is 3). Moreover, `v11-v14` is [optimal](#optimal-critical-path).  
After expressions balancing:
```
0.i64 Parameter  -> v8      // a
1.i64 Parameter  -> v8      // b
2.i64 Parameter  -> v9      // c
3.i64 Parameter  -> v10     // d
4.i64 Parameter  -> v11     // e
5.i64 Parameter  -> v12     // f
6.i64 Parameter  -> v12     // g

8. Add v3, v2 -> v10        // d + c
9. Add v0, v1 -> v10        // a + b
// As soon as v10 has more than one users it has side-effects, so the algorithm considers it as a term:
10. Add v8, v9 -> v11, v14  // s10 = ((d + c) + (a + b))

11. Add v4, v10 -> v13      // e + s10
12. Add v5, v6 -> v13       // f + g
13. Add v11, v12 -> v14     // (e + s10) + (f + g)
14. Add v10, v13 -> v15     // s10 + ((e + s10) + (f + g))

15. Return v14
```
## Links
Source code:   
[balance_expressions.cpp](../optimizer/optimizations/balance_expressions.cpp)  
[balance_expressions.h](../optimizer/optimizations/balance_expressions.h)  

Tests:  
[balance_expressions_test.cpp](../tests/balance_expressions_test.cpp)

