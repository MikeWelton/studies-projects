# Diakrynizator

Transforms Unicode characters with values up to `0x10FFFF`, encoded on max. 4 bytes.
Characters with values from `0x00` to `0x7F` are staying unchanged.
Other characters of given range are transformed using polynomial:
```
w(x) = an * x^n + ... + a2 * x^2 + a1 * x + a0
```
whereas its coefficients are whole, non-negative numbers of 10 base.
Polynomial is calculated modulo `0x10FF80`. In the end we transform Unicode character `x` to `w(x - 0x80) + 0x80`.

Compile and link:
```
nasm -f elf64 -w+all -w+error -o diakrytynizator.o diakrytynizator.asm
ld --fatal-warnings -o diakrytynizator diakrytynizator.o
```
Run:
```
./diakrytynizator a0 a1 a2 ... an
```
