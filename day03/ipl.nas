; hello-os
; TAB=4

CYLS    EQU     10

        ORG     0x7c00

; FAT12 format section

        JMP     entry
        DB      0x90
        DB      "HARIBOTE"      ;8byte
        DW      512
        DB      1
        DW      1
        DB      2
        DW      224
        DW      2880
        DB      0xf0
        DW      9
        DW      18
        DW      2
        DD      0
        DD      2880
        DB      0,0,0x29
        DD      0xffffffff
        DB      "HARIBOTE_OS"   ;11byte
        DB      "FAT12   "      ;8byte
        RESB    18

; program section

entry:
        MOV     AX,0            ;initialize registers 
        MOV     SS,AX
        MOV     SP,0x7c00
        MOV     DS,AX

; loading disk

        MOV     AX,0x0820    
        MOV     ES,AX
        MOV     CH,0            ;cylinder 0
        MOV     DH,0            ;head 0
        MOV     CL,2            ;sector 2
readloop:
        MOV     SI,0            ;register that counting failure
retry:
        MOV     AH,0x02         ;AH=0x02->loading disk
        MOV     AL,1            ;one sector
        MOV     BX,0
        MOV     DL,0x00         ;A drive
        INT     0x13            ;call disk bios
        JNC     fin             ;jump to fin if error
        ADD     SI,1            ;add 1 to SI
        CMP     SI,5
        JC      error           ;jump to error if SI >= 5
        MOV     AH,0x00
        MOV     DL,0x00         ;A drive
        INT     0x13            ;reset the drive
        JMP     retry
next:
        MOV     AX,ES           ;add 0x200 to address
        ADD     AX,0x0020
        MOV     ES,AX
        ADD     CL,1
        CMP     CL,18
        JBE     readloop        ;jump to readloop if CL <= 18
        MOV     CL,1
        ADD     DH,1
        CMP     DH,2
        JB      readloop        ;jump to readloop if DH < 2
        MOV     DH,0
        ADD     CH,1
        CMP     CL,CYLS
        JBE     readloop        ;jump to readloop if CH <= CYLS

;sleeping because there's not to do

fin:
        HLT
        JMP     fin             ;loop infinity
error:    
        MOV     SI,msg
putloop:
        MOV     AL,[SI]
        ADD     SI,1
        CMP     AL,0
        JE      fin
        MOV     AH,0x0e         ;display a character
        MOV     BX,15           ;color code
        INT     0x10            ;call video bios
        JMP     putloop

; message section
msg:
        DB      0x0a, 0x0a
        DB      "load error"
        DB      0x0a
        DB      0

        RESB    0x7dfe-$          ;fill ~0x001fe with 0x00

        DB      0x55, 0xaa

; apendix section (deleted)
