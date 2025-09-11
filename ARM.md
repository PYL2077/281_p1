## ARM
- ADD,ADDI (adding numbers)
- SUB,SUBI (subbing numbers)

### Shifting
- LSR r6 r23 #2
    - r6=r23>>2
- LSL
### Psudo instruction
- MOV x1 x2

### Memory instructions
- word: a number of bytes
- LC2K is word addressable
    - all address refers to a word
- ARM is byte addressable
- LEG
    - register is 64bit
    - word is 32bit

### Loading
- LDUR (64bit)
- LDURH (16bit)
- LDURB (8bit)
- LDURSW (32bit)
    - set unused bits based on significant bit
    - maintains sign

### Store
- STUR (64)
- STURH (16)
- STURB (8)

- STURW (32)



## C to assembly
- An $n$ byte data should start at $A%n=0$
- padding is put between objects if needed
- start of a struct is aligned based on the largest data

### Control Flow
- CBZ X1 #25: go to PC+25*4 if X1 is zero
- CBNZ X1 #25: go to PC+25*4 if X1 is not zero

- FLAGS: NZVS record results of arithmetic operations
    - ADDS, SUBS, ADDIS
![hah](NVZS.png)
- CMP X1, X2 ()
    - b.ne
    - b.eq
