; naskfunc
; TAB=4

[FORMAT "WCOFF"]            ;object file producing mode
[BITS 32]                   ;produce in 32bit machine language

; infomation for object file

[FILE "naskfunc.nas"]       ;source file name

        GLOBAL _io_hlt      ;function name in this program

; functions

[SECTION .text]

_io_hlt: ;void io_hlt(void);
        HLT
        RET