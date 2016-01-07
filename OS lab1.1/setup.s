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
	push #11
	push #cursor
	call print_string
	pop ax
	pop ax
	
! Print cusor pos
	mov bp, #0x00
	call print_hex
	call print_nl

! Print "Memory SIZE"
	push #12
	push #memory
	call print_string
	pop	ax
	pop ax
	
!Print memory size
	mov bp, #0x02
	call print_hex
!Print KB
	push #2
	push #KB
	call print_string
	pop ax
	pop	ax
	call print_nl

! Print "Cyls:"
	push #5
	push #cyls
	call print_string
	pop ax
	pop ax
! Print cyls:
	mov bp, #0x04
	call print_hex
	call print_nl

! Print "Heads:"
	push #6
	push #heads
	call print_string
	pop ax
	pop ax
! Print heads:
	mov bp, #0x06
	call print_hex
	call print_nl

! Print "Sectors:"
	push #8
	push #_sectors
	call print_string
	pop ax
	pop ax
! Print sectors:
	mov bp, #0x12
	call print_hex
	call print_nl

inf_loop:
	jmp inf_loop

print_string:
	mov	ax,#SETUPSEG
	mov	es,ax
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	add esp, #4
	mov cx, (esp)
	mov	bx,#0x0007
	sub esp, #2
	mov bp, (esp)
	sub esp, #2		! ret PC
	mov ax,#0x1301
	int 0x10
	ret

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
