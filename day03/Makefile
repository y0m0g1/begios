# variables

TOOLPATH = ../z_tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

# default

default :
	$(MAKE) img

# file producing rules

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.pst

helloos.img : ipl.bin Makefile
	$(EDIMG) imgin:../z_tools/fdimg0at.tek \
	wbinimg src:ipl.bin len:512 from:0 to:0 imgout:helloos.img

# commands

img :
	$(MAKE) helloos.img

asm :
	$(MAKE) ipl.bin

run :
	$(MAKE) img
	$(COPY) helloos.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: helloos.img

clean :
	-$(DEL) ipl.bin
	-$(DEL) ipl.lst

src_only :
	$(MAKE) clean
	-$(DEL) helloos.img