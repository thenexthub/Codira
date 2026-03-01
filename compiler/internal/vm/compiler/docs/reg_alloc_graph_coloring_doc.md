# AOT Register allocator

## Overview
`Register allocator` assigns program variables to target CPU registers

## Rationality
Minimize load/store operations with memory

## Dependence
* LivenessAnalysis
* DominatorsTree
* Reverse Post Order (RPO)

# Register allocation on Chordal-graphs with advanced register coalescing
Register allocation algorithm by Evgeny Erokhin.

This paper describes register allocation algorithm that is based on ideas of Chordal-graph coloring with integrated coalescing.
Base method of register allocation on Chordal-graphs is described in works of Sebastian Hack and Fernando Pereira. It is based on observation that programs in a strict SSA form produce a Chordal interference graphs. Main properties of Chordal graph is that: its max-clique value equals to Chromatic one; there are exist Perfect Elimination Order (PEO) - order in which graph deconstruction always keeps chordality-property. And using PEO for coloring lets the standard greedy algorithm to color a graph by optimal way (max number of registers equal to Chromatic number).
In works of Hack ("Register Allocation for Programs in SSA Form") and Pereira ("Register Allocation via Chordal Graphs") to construct PEO is used Max Cardinality Search (MCS) algorithm. In this implementation was used alternative algorithm that is LexBFS. It uses little bit more compact data structures, that gives some performance advantages other MCS.
Before coloring, available registers are collected from Architecture and remapped (to sequential numbering) in order <caller-saved, callee-saved>. In fact because these values with call-site crossing will interfere with fixed-intervals (special intervals that are placeholders of convention registers) of call-site, it will shift coloring to callee-saved registers.

## Build interference graph
Graph is built by liveness-intervals, algorithm under the hood is the same as in Linear-scan from works of Wimmer ("Linear Scan Register Allocation on SSA Form"). But output is an Interference Graph (IG). Additionally for nodes is collected number or fixed-intervals intersections that tell us that this interval is crossed by call-sites.

## Precoloring
If interval is pre-colored, this value is set as pre-color for node. And input parameters is marked for split if it has intersection with call-site. They will be split later, to let algorithm make a decision about caller/callee register placement.
Here additionally to the built graph with regular edges that represent interference, it is built affinity edges of the graph. Affinity edges represent move-relation between values. Currently affinity edges are added for inputs of PHI-function and input of Return instructions.
Output graph will be interference-graph G = {Nodes, IEdges, AEdges} – graph with 2 types of edges: interference, affinity.

## How basic chordal coloring works
Using ordering of nodes by MCS or LexBFS (that is reverse PEO), one by one is colored by a Greedy coloring algorithm. Greedy coloring is an algorithm that is collecting colors of neighbor nodes, and chose color for processed node by selecting first not occupied color.
PEO on chordal graph always builds a clique, so it always color consequently nodes of the same clique, or start another one. This way we don’t have situations when different cliques are colored in parallel that at the end collides, that may cause extra register for coloring. This property let us optimal coloring.

## Bias construction
This algorithm extends coalescing abilities of original algorithms of Chordal-RA with integrated coalescing.
There are two classes of approaches for coalescing, that is:
-	pre/post coalescing that happens before or after coloring respectively. Pre-coalescing is known to potentially can increase register pressure, and make graph G’ not K-colorable as original graph G. Post coalescing (like in Pereira algorithm) is trying to coalesce after coloring, that makes work when major decisions already done, so it decrease ability for successful coalescing. Or alternatively on post-coalescing potentially big amount of re-coloring may be required.
-	Biasing, is a way when in front of coloring nodes are “hinted” with preferred colors. And on stage of coloring it is tried to satisfy these hints if possible. This approach is more flexible, and gives an ability to make decision on coloring having information about neighbors and specially prepared information for biasing.
Presented algorithm leverages second approach. After Pre-coloring stage we have colors assigned for nodes such as input parameters of function itself and arguments for call-sites, and affinity subgraph has been added.
Each affinity edge means connection of two values by move-relation. That means that in worst-case scenario there will be additional move presented. Value maybe transitively passed through many Phi-function or any other kind of move-relations (such as return value or Spill-Fills between registers).
Affinity edges of IG form components known as Affinity-components. In best-case scenario all nodes in affinity-component should be colored with the same color, which will remove all move-instructions on its edges. Let’s call such components of graph “Bias”.
To find biases it is used component search algorithm which is recursive call to DFS, until all nodes having affine edges are visited. Marker of visited node here is assigned bias-number. And each bias additionally holds cumulative number of call-sites intersections of underlying nodes. That value will be later to prevent situation, when first bias’s colored node doesn’t have call-site intersection, which will “hint” all nodes of bias to caller-save register. So if other nodes in bias have intersections, it will turn to multiple spill-fills.
Biases are collected to array of tuples <bias color, call-sites count>, where color value initially set to “Undefined” value. Bias-array is used on assignment (Coloring) phase.

## Coloring
Base for this register assignment algorithm is a greedy-coloring algorithm, but with modification to support biasing. 
This algorithm traversing nodes of IG one by one in order provided by LexBFS algorithm (that is reversed PEO). On each step processes only nodes that have no colors (not pre-colored). On each step we need two sets of values: neighbor colors of current node, and neighbor bias-colors (if any). To account this information two bit-sets are used respectively. 
The next step is to collect busy neighbor colors and bias colors bitsets. For each node it is checked if it is a neighbor and it has a color, this value is set in bitset. Else, if it is affine neighbor presence of bias is checked. Respective color collected to neighbor bias-color bitset.
Having “Busy”-bitmaps for current node, assignment phase is started. If node has bias and color set in bias-array and this color is not busy (checked by neighbor bitset), this color is set for node. Else the new color will be allocated: bias bitset is OR-combined with neighbor busy-bitset, that creates “Full”-busy bitset. Satisfying it provide us the best case allocation, because it’s not only doesn’t touch colors of already colored neighbors, but takes in account biases of not colored yet ones. Additionally based on bias-array it is made decision about offset for search of free-color. If call-sites count in bias-array isn’t zero, search of free color should start from calle-saved range.
As for now search of free color is implemented as a remainder of division by number of colors, to wrap-around the number of colors if offset was set to start of callee-saved range and it is fully occupied. Search is done by two phases. On first phase neighbor busy bitset is scanned in loop for first not-set bit. On second phase search continues from found position on 1st phase, but now on “Full”-busy bitset, to try to satisfy neighbor bias preferences by tying to improve choice from 1st phase. This 2 phase scheme helps in case if colored node take part in multiple cliques, and potentially neighbors from different cliques may occupy all available colors by their biases. In such case choice made by 1st phase won’t be changed.
After color for node has been chosen, if node has bias and in bias-array color is in “Undefined” state, this color is set in bias-array and will be used for other nodes of this bias.
And at the end, color chosen from previous bias color or chosen by 2-phase scheme is set for node.

## Remapping back
Finally, if node was marked to split, real split on corresponding interval is performed. All nodes at the moment are assigned by colors, these colors are remapped back to physical register numbers and assigned to intervals.

## Current implementation limitations
Current implementation does not support spilling and rematerialisation. So in case of high register pressure, fallback to LinearScan is performed. Because entire allocation is done in algorithm-own structures, and implementation of results happen after allocation is successfully done, in case of fallback all analyses are preserved.

# TBD
Meantime information about call-site convention-registers is not passed to allocator. When it is available, it’s worth to add pre-coloring of inputs of arguments, in case the interval of input is not intersected by a call-site. The same thing should be done for return value, meantime there’s no way to get architected return register number.

