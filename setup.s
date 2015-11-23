!
!	setup.s		(C) 1991 Linus Torvalds
!
! setup.s is responsible for getting the system data from the BIOS,
! and putting them into the appropriate places in system memory.
! both setup.s and system has been loaded by the bootblock.
!
! This code asks the bios for memory/disk/other parameters, and
! puts them in a "safe" place: 0x90000-0x901FF, ie where the
! boot-block used to be. It is then up to the protected mode
! system to read them from there before the area is overwritten
! for buffer-blocks.
!

! NOTE! These had better be the same as in bootsect.s!

INITSEG  = 0x9000	! we move boot here - out of the way
SYSSEG   = 0x1000	! system loaded at 0x10000 (65536).
SETUPSEG = 0x9020	! this is the current segment

.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

entry start
start:

go:	mov	ax,cs
	mov	ds,ax
	mov	es,ax
! Print some inane message

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#25
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg2
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

!death_fuck:
	!jmp death_fuck

! ok, the read went well so we get current cursor position and save it for
! posterity.

	mov	ax,#INITSEG	! this is done in bootsect already, but...
	mov	ds,ax
	mov	ah,#0x03	! read cursor pos
	xor	bh,bh
	int	0x10		! save it in known place, con_init fetches
	mov	[0],dx		! it from 0x90000.
! Get memory size (extended mem, kB)

	mov	ah,#0x88
	int	0x15
	mov	[2],ax

! Get video-card data:

	mov	ah,#0x0f
	int	0x10
	mov	[4],bx		! bh = display page
	mov	[6],ax		! al = video mode, ah = window width

! check for EGA/VGA and some config parameters

	mov	ah,#0x12
	mov	bl,#0x10
	int	0x10
	mov	[8],ax
	mov	[10],bx
	mov	[12],cx

! Get hd0 data

	mov	ax,#0x0000
	mov	ds,ax
	lds	si,[4*0x41]
	mov	ax,#INITSEG
	mov	es,ax
	mov	di,#0x0080
	mov	cx,#0x10
	rep
	movsb

! Get hd1 data

	mov	ax,#0x0000
	mov	ds,ax
	lds	si,[4*0x46]
	mov	ax,#INITSEG
	mov	es,ax
	mov	di,#0x0090
	mov	cx,#0x10
	rep
	movsb

	!mov ax, #INITSEG
	!mov ds, ax
	!mov ax, #SETUPSEG
	!mov es, ax

print_cur:
  mov ah,#0x03
  xor bh,bh
  int 0x10        
  mov ax,#SETUPSEG
  mov es,ax
  mov cx,#16
  mov bx,#0x0007
  mov bp,#msg3
  mov ax,#0x1301
  int 0x10
  xor di,di
  mov ax,#INITSEG
  mov ds,ax
  mov bx,(di)
  call print_bx
  call print_nl

print_mem:
  mov ah,#0x03
  xor bh,bh
  int 0x10        
  mov cx,#23
  mov bx,#0x0007
  mov bp,#msg4
  mov ax,#0x1301
  int 0x10
  
  mov di,#0x02
  mov bx,(di)
  call print_bx
  call print_nl

  !loop3:
	!jmp loop3

print_hd:
  mov ah,#0x03
  xor bh,bh
  int 0x10        
  mov cx,#8
  mov bx,#0x0007
  mov bp,#msg5
  mov ax,#0x1301
  int 0x10 
  mov di,#0x80
  mov bx,(di)
  call print_bx
  call print_nl

  mov ah,#0x03
  xor bh,bh
  int 0x10        
  mov cx,#9
  mov bx,#0x0007
  mov bp,#msg6
  mov ax,#0x1301
  int 0x10 
  mov di,#0x82
  mov bx,(di)
  xor bh,bh
  call print_bx
  call print_nl
  
  mov ah,#0x03
  xor bh,bh
  int 0x10        
  mov cx,#11
  mov bx,#0x0007
  mov bp,#msg7
  mov ax,#0x1301
  int 0x10 
  mov di,#0x8e
  mov bx,(di)
  xor bh,bh
  call print_bx
  call print_nl

death_loop:
	jmp death_loop

print_bx:
   mov cx,#4
   mov dx,bx
print_digit:
   rol dx,#4
   mov ax,#0x0e0f
   and al,dl
   add al,#0x30
   cmp al,#0x3a
   jb outp
   add al,#0x07
outp:
   int 0x10
   loop print_digit
   ret

print_nl:
   mov ax,#0x0e0d
   int 0x10
   mov al,#0x0a
   int 0x10
   ret

msg2:
	.byte 13,10
	.ascii "Now we are in SETUP"
	.byte 13,10,13,10	

msg3:
    .ascii "Cursor Position:"
	 
msg4:
    .ascii "Extend Memory Size(KB):"
	 
msg5:
    .ascii "HD cyls:"	 
msg6:
    .ascii "HD heads:"
msg7:
    .ascii "HD Sectors:"

.text
endtext:
.data
enddata:
.bss
endbss:
