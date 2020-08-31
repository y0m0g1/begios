; haribote_os
; TAB=4

BOTPAK  EQU     0x00280000      ;load bootpack from
DSKCAC  EQU     0x00100000      ;disk cache
DSKCAC0 EQU     0x00008000      ;disk cache (real mode)

; BOOT_INFO
CYLS    EQU     0x0ff0
LEDS    EQU     0x0ff1
VMODE   EQU     0x0ff2
SCRNX   EQU     0x0ff4
SCRNY   EQU     0x0ff6
VRAM    EQU     0x0ff8

        ORG     0xc200

; set the graphics mode

        ; MOV     AL,0x13         ;VGA graphics 320x200,8bit color
        ; MOV     AH,0x00
        ; INT     0x10
        ; MOV     BYTE [VMODE],8
        ; MOV     WORD [SCRNX],320
        ; MOV     WORD [SCRNY],200
        ; MOV     DWORD [VRAM],0x000a0000

        ; MOV     BX,0x4101       ;VBE graphics 640x480,8bit color
        ; MOV     AX,0x4f02
        ; INT     0x10
        ; MOV     BYTE [VMODE],8
        ; MOV     WORD [SCRNX],640
        ; MOV     WORD [SCRNY],480
        ; MOV     DWORD [VRAM],0xe0000000

        MOV     BX,0x4105       ;VBE graphics 1024x768,8bit color
        MOV     AX,0x4f02
        INT     0x10
        MOV     BYTE [VMODE],8
        MOV     WORD [SCRNX],1024
        MOV     WORD [SCRNY],768
        MOV     DWORD [VRAM],0xe0000000

; get the keyboard's led status

        MOV     AH,0x02
        INT     0x16            ;keybord bios
        MOV     [LEDS],AL

; make pic deny all interruptions

        MOV     AL,0xff
        OUT     0x21,AL
        NOP
        OUT     0xa1,AL

        CLI                     ;prohibit interruptions on cpu level

; set A20GATE to allow cpu to access 1MB> memory

        CALL    waitkbdout
        MOV     AL,0xd1
        OUT     0x64,AL
        CALL    waitkbdout
        MOV     AL,0xdf         ;enable A20
        OUT     0x60,AL
        CALL    waitkbdout

; move to protect mode

[INSTRSET "i486p"]              ;to use 486 instructions

        LGDT    [GDTR0]         ;setting temporary GDT
        MOV     EAX,CR0
        AND     EAX,0x7fffffff  ;to restrict paging, make bit31 to 0
        OR      EAX,0x00000001  ;to move to protect mode, make bit0 to 1
        MOV     CR0,EAX
        JMP     pipelineflush
pipelineflush:
        MOV     AX,1*8          ;read-writable segment 32bit
        MOV     DS,AX
        MOV     ES,AX
        MOV     FS,AX
        MOV     GS,AX
        MOV     SS,AX

; transfer bootpack

        MOV     ESI,bootpack    ;transfer source
        MOV     EDI,BOTPAK      ;transfer destination
        MOV     ECX,512*1024/4
        CALL    memcpy

; transfer disk data

; boot sector

        MOV     ESI,0x7c00      ;transfer source
        MOV     EDI,DSKCAC      ;transfer destination
        MOV     ECX,512/4
        CALL    memcpy

; rest of all

        MOV     ESI,DSKCAC0+512 ;transfer source
        MOV     EDI,DSKCAC+512  ;transfer destination
        MOV     ECX,0
        MOV     CL,BYTE [CYLS]
        IMUL    ECX,512*18*2/4  ;convert from cylinder num to byte num / 4 
        SUB     ECX,512/4       ;subtract only ipl
        CALL    memcpy

; finished all task that asmhead have to do
; leave it to bootpack

; boot bootpack

        MOV     EBX,BOTPAK
        MOV     ECX,[EBX+16]
        ADD     ECX,3           ;ECX += 3
        SHR     ECX,2           ;ECX >> 2
        JZ      skip            ;nothing to transfer
        MOV     ESI,[EBX+20]    ;transfer source
        ADD     ESI,EBX
        MOV     EDI,[EBX+12]    ;transfer destination
        CALL    memcpy
skip:
        MOV     ESP,[EBX+12]    ;stack initial value
        JMP     DWORD 2*8:0x0000001b

waitkbdout:
        IN      AL,0x64
        AND     AL,0x02
        JNZ     waitkbdout      ;jump to w8t if result not zero
        RET

memcpy:
        MOV     EAX,[ESI]
        ADD     ESI,4
        MOV     [EDI],EAX
        ADD     EDI,4
        SUB     ECX,1
        JNZ     memcpy          ;jump to memcpy if result not zero
        RET

        ALIGNB  16

GDT0:
        RESB    8               ;null sector
        DW      0xffff,0x0000,0x9200,0x00cf     ;read-writable segment 32bit
        DW      0xffff,0x0000,0x9a28,0x0047     ;executable segment 32bit (for bootpack)

        DW      0
GDTR0:
        DW      8*3-1
        DD      GDT0

        ALIGNB  16
bootpack: