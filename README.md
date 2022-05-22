# GEC: Generic Elliptic Curve Library

## TODO

- [x] crtp
- [ ] divide `sequence.hpp` into smaller pieces
- [ ] bigint
  - [x] bit operations
    - [ ] inplace bit operations
  - [ ] add
    - [x] `add_with_carry`
      - [x] with intrinsics
    - [x] `add_with_carry` inplace
      - [ ] with intrinsics
    - [x] `add`: $c = a + b \pmod{\mathrm{LIMB_BITS}}$
    - [x] `add` inplace: $a = a + b \pmod{\mathrm{LIMB_BITS}}$
    - [x] add group `add`: $c = a + b \pmod{M}$
    - [x] add group `add` inplace: $a = a + b \pmod{M}$
  - [ ] sub
    - [x] `sub_with_borrow`
      - [ ] with intrinsics
    - [x] `sub_with_borrow` inplace
      - [ ] with intrinsics
    - [x] `sub`: $a = b - c \pmod{\mathrm{LIMB_BITS}}$
    - [x] `sub` inplace: $a = a - b \pmod{\mathrm{LIMB_BITS}}$
    - [x] add group `sub`: $a = b - c \pmod{M}$
    - [x] add group `sub` inplace: $a = a - b \pmod{M}$
  - [ ] mul
    - [ ] Interleaved Multiplication-reduction Method
    - [x] Montgomery Multiplication
      - [ ] Q: What about $N$ that is not coprime with $b$?
        - [x] Q: Is mon-mul capable of handling this?\
              A: No
          - [x] Q: If not, what's a good alternative?\
                A: A possible alternative is to use Interleaved Multiplication-reduction Method instead
          - [ ] Q: Is mon-arthmetic + $\mathbb{F}_{2^q}$ arthmetic + CRT a probable or even a good solution?
  - [ ] inv
    - [x] Montgomery Inversion
- [ ] replace `array` based interface with `sequence` based (Maybe 2.0?)
- [ ] refactor operations for heterogeneous data structures (Maybe 2.0?)
- [ ] fix segment fault under Clang in release mode
