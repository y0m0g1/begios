[BITS 32]
    MOV     AL,'H'
    CALL    2*8:0x0b9a ;_asm_put_char
    MOV     AL,'e'
    CALL    2*8:0x0b9a
    MOV     AL,'l'
    CALL    2*8:0x0b9a
    MOV     AL,'l'
    CALL    2*8:0x0b9a
    MOV     AL,' '
    CALL    2*8:0x0b9a
    MOV     AL,'w'
    CALL    2*8:0x0b9a
    MOV     AL,'o'
    CALL    2*8:0x0b9a
    MOV     AL,'r'
    CALL    2*8:0x0b9a
    MOV     AL,'l'
    CALL    2*8:0x0b9a
    MOV     AL,'d'
    CALL    2*8:0x0b9a
    MOV     AL,'!'
    CALL    2*8:0x0b9a
    RETF