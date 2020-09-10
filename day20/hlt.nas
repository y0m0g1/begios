[BITS 32]
    MOV     AL,'A'
    CALL    2*8:0x0b95 ;_asm_put_char
fin:
    HLT
    JMP     fin