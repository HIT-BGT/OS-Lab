!
!	setup.s		(C) 1991 Linus Torvalds
!

.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

BOOTSEG  = 0x07c0			! original address of boot-sector
INITSEG  = 0x9000			! we move boot here - out of the way
SETUPSEG = 0x9020			! setup starts here

! ROOT_DEV:	0x000 - same type of floppy as boot.
!		0x301 - first partition on first drive etc
ROOT_DEV = 0x306

entry _start
_start:

! Print some inane message
	mov	ax,#SETUPSEG
	mov ds,ax
	mov	es,ax

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#25
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg1
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

!fetch the cursor location
	mov	ax,#INITSEG	! this is done in bootsect already, but...
	mov	ds,ax
	mov	ah,#0x03	! read cursor pos
	xor	bh,bh
	int	0x10		! save it in known place, con_init fetches
	mov	[0],dx		! it from 0x90000.

!fetch memory size
	mov	ah,#0x88
	int	0x15
	mov	[2],ax

!fetch hard drive info
	mov	ax,#0x0000
	mov	ds,ax
	lds	si,[4*0x41]
	mov	ax,#INITSEG
	mov	es,ax
	mov	di,#0x0004
	mov	cx,#0x10
	rep
	movsb
    call print_nl

! Print "Cursor POS:"
	mov	ax,#SETUPSEG
	mov	es,ax
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#11
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#cursor
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
! Print cusor pos
	mov bp, #0x00
	call print_hex
	call print_nl

! Print "Memory SIZE"
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#12
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#memory
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
!Print memory size
	mov bp, #0x02
	call print_hex
!Print KB
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#2
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#KB
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
	call print_nl

! Print "Cyls:"
	mov	ax,#SETUPSEG
	mov	es,ax
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#5
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#cyls
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
! Print cyls:
	mov bp, #0x04
	call print_hex
	call print_nl

! Print "Heads:"
	mov	ax,#SETUPSEG
	mov	es,ax
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#6
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#heads
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
! Print heads:
	mov bp, #0x06
	call print_hex
	call print_nl


! Print "Sectors:"
	mov	ax,#SETUPSEG
	mov	es,ax
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#8
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#_sectors
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
! Print sectors:
	mov bp, #0x12
	call print_hex
	call print_nl

inf_loop:
	jmp inf_loop

print_hex:
    mov	cx,#4 		! 4 hex numbers
    mov	dx,(bp) 	! move the number dp points to to dx
print_digit:
    rol	dx,#4		
    mov	ax,#0xe0f 	! ah = 0x0E, al = ASCII code of what is going to show
    and	al,dl
    add	al,#0x30 
    cmp	al,#0x3a
    jl	outp  		!if it is smaller than 10
    add	al,#0x07  	!plus 7 more
outp: 
    int	0x10
    loop	print_digit
    ret

print_nl:
    mov	ax,#0xe0d 	! CR
    int	0x10
    mov	al,#0xa 	! LF
    int	0x10
    ret

sectors:
	.word 0

msg1:
	.byte 13,10
	.ascii "Now we are in SETUP"
	.byte 13,10,13,10
cursor:
	.ascii "Cursor POS:"
memory:
	.ascii "Memory SIZE:"
KB:
	.ascii "KB"
cyls:
	.ascii "Cyls:"
heads:
	.ascii "Heads:"
_sectors:
	.ascii "Sectors:"

.org 508
root_dev:
	.word ROOT_DEV
boot_flag:
	.word 0xAA55

.text
endtext:
.data
enddata:
.bss
endbss:
