#### Definitions D1
- An initial starting position for an n×n grid can be described using an n×n *Configuration Matrix* `A` with binary entries (`A_{x,y} = 0` or `A_{x,y} = 1`).
- If `A` is an n×n Configuration Matrix, we denote `A_i` to be the state resulting from applying the rules to `A_{i - 1}` for all `i > 0` with `A_0 = A`.
- The matrix `A` has *length* `n` if `n` is the minimum integer (`n > 0`) such that `A_n=A_{n - 1}`.
- We define `f(n)` to be the maximum length achieved by an n×n matrix.
- We define `s(n)` to be the number of initial water placemens needed to achieve the maximum length in an n×n grid.
