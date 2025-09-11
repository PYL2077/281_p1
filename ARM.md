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