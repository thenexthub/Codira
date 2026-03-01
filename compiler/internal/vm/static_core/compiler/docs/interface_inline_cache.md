# Interface Inline Cache
## Overview

`Interface Inline Cache` is an optimization for speeding up find method in interface method call.
Ark Runtime allows multiple interfaces to be inherited, and  uses the itable table to dynamically query the address of the target method.
`Interface Inline Cache` will stores the parsing result of the previous method address as the cache and directly uses the cache in subsequent call.

## Rationality

`Interface Inline Cache`'s reason for optimization:  
*  There is a 90% - 95% chance that the same method will be used in real-world application test.  
*  The speed of querying the itable table is very slow.  

`Interface Inline Cache` include `Fast Path` and `Slow Path`  
*  `Slow path`: Call runtime RESOLVE_VIRTUAL_CALL_AOT to get method address  and save method addr and class to cache  
*  `Fast Path`: Check whether the class is the same as the last cached class,if equal: use cache; if no equal: goto `Slow Path`  

## Dependence

*  must be 64bit system

## Algorithm

### Schematic Drawing 

```
    Cache structure:（offset addr)/(class addr) 32bit/32bit
    -----------------------------------------------
    (.aot_got)
        ...
        cache:offset/class       ---------->| <-|
        ...                                 |   |
    (.text)                                 |   |
    interface call start                    |   |
        --> call runtime irtoc function     |   |
        read cache  <-----------------------|   |
        if call class == cache.class            |
           use cache.offset(method)             |
        else                                    |
           call RESOLVE_VIRTUAL_CALL_AOT        |
           save method‘s offset to cache >------|
        <-- return to (.text)
    call method
    -----------------------------------------------
```
`cache`
1. one cache is 64bit
2. cache's high 32 save the `offset addr`,cache's low 32 save the `class addr`
3. cache's address is in an file's aot_got section
4. number of cache equal "interface call" number in this an file
5. why use 64bit?  
because read and write 64bit is atomic, same `cache` maybe use at same time

`class addr`
1. `class addr` is just the point to a class, in ark runtime, class point is 32bit
2. because `class address` is no move in ark, so we do not need to worry that the saved class will become invalid addresses.
3. if class address is move, it is still save, but maybe the hit rate will be lower.

`offset addr`
1. The purpose of `offset addr` is to obtain the `method addr` through calculation.
2. `offset addr` = ((`method addr`) - ([`class addr`].`methods_` addr)) / 2 ^ 3
3. why can not save `method addr` directly?  
    because method* is 64bit, it must be indirectly stored,cache only have 32bit to use
```
if we want to save ClassA.method_n*
-----------
| Class A  |
| ...      |     Array of Method*
| ...      |    --------------
| methods_*| -->| method_1*   |
| ...      |    | ...         |
-----------     | ->method_n* |
                | ...         |
we just need to save method_1* - method_n*

Note: Class A records only its own methods, if the implementation of the target method is in the parent class B of Class A.

-----------
| Class A  |
| ...      |     Array of Method*
| ...      |    --------------
| methods_*| -->| method_1*   |
| ...      |    | ...         |
-----------

-----------
| Class B  |
| ...      |     Array of Method*
| ...      |    --------------
| methods_*| -->| method_1*   |
| ...      |    | ...         |
-----------     | ->method_n* |
                | ...         |

we still save ClassA.method_1* - ClassB.method_n*, so if  
(ClassA.method_1* - ClassB.method_n*) > 2 ^ 32  
this optimization will be abandoned.This condition is quite rare.
```
4. why `offset addr` divided by 2 ^ 3 at the end of the formula？  
because Method* is 8bit align, so Method* can be compressed. So the actual range we can store is:   
((ClassA.method_1* - ClassB.method_n*) / 2 ^ 3) < 2 ^ 32

## Expected effect
1. `Interface Inline Cache` will reduce `RESOLVE_VIRTUAL_CALL_AOT` calls by more than 90%  
2. benchmark test case: ImtConflicts100 -- runing time will accelerate by more than 300%
3. Application startup: 10ms - 100ms. The value depends on the number of interfaces call.

## Cost
1. an file bloat < 1%
2. Rss：No significant impact