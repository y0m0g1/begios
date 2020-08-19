; haribote_os
; TAB=4

        ORG 0xc200

        MOV AL,0x13     ;VGA graphics 320x200,8bit color
        MOV AH,0x00
        INT 0x10

fin:
        HLT
        JMP fin