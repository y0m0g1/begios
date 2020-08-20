; naskfunc
; TAB=4

[FORMAT "WCOFF"]            ;object file producing mode
[INSTRSET "i486p"]          ;to use 486 instructions set
[BITS 32]                   ;produce in 32bit machine language

; infomation for object file

[FILE "naskfunc.nas"]       ;source file name

        GLOBAL _io_hlt,_write_mem8      ;function name in this program

; functions

[SECTION .text]

_io_hlt: ;void io_hlt(void);
        HLT
        RET

_write_mem8:    ;void write_mem8(int addr, int data);
        MOV     ECX,[ESP+4]     ;load addr on [ESP+4] to ECX
        MOV     AL,[ESP+8]      ;load addr on [ESP+8] to AL
        MOV     [ECX],AL
        RET