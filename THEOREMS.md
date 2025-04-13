## Theorem T1
_(Once a cell is turned on, it doesn't turn off again)_  
If `A` is an n×n Configuration Matrix and if `(A_i){x,y}=1` for any `i >= 0`, then `(A_j){x,y}=1` for all `j > i`.

#### Proof
The rules only allow for cells to be turned on, not off. Obvious.

## Theorem T2
`f(n + 1) > f(n)` for all `n`.

#### Proof
You can replicate the pattern used to create `f(n)` in the `n+1`-sized matrix and achieve `f(n)`.

## Theorem T3
`f(n) >= n` for all `n`.

#### Proof
We achieve a length of exactly `n` by setting `A_{x,y}={1 if x=y, 0 else}`.

## Theorem T4
`13/18 × (n²) - O(n) ≤ f(n) ≤ n²`

#### Proof
TODO.
