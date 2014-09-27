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
!print some message
	mov	ax,#SETUPSEG
	mov	es,ax
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#25
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg_setup
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

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


	mov ax,#SETUPSEG
	mov es,ax	! if not  will be unreadable codes	
!print the cursor position
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#13
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg_cursor
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

	mov ax,#INITSEG
	mov ds,ax
	mov bp,#0

	call print_hex	

!print the size of memory
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#14
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg_mem
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

	mov ax,#INITSEG
	mov ds,ax
	mov bp,#2

	call print_hex		

!print the hd0 data:
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#11
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg_hd0
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

	mov ax,#INITSEG
	mov ds,ax
	mov bp,#0x80
	mov bx,#16
	mov bp,#0x80
loop_hd:
	call print_hex
	add bp,#2
	sub bx,#2
	cmp bx,#0
	ja loop_hd

	jmp print_nl

print_hex:
    mov	cx,#4 		! 4个十六进制数字
    mov	dx,(bp) 	! 将(bp)所指的值放入dx中，如果bp是指向栈顶的话
print_digit:
    rol	dx,#4		! 循环以使低4比特用上 !! 取dx的高4比特移到低4比特处。
    mov	ax,#0xe0f 	! ah = 请求的功能值，al = 半字节(4个比特)掩码。
    and	al,dl 		! 取dl的低4比特值。
    add	al,#0x30 	! 给al数字加上十六进制0x30
    cmp	al,#0x3a
    jl	outp  		!是一个不大于十的数字
    add	al,#0x07  	!是a～f，要多加7
outp: 
    int	0x10
    loop	print_digit
    ret	

print_nl:
    mov	ax,#0xe0d 	! CR
    int	0x10
    mov	al,#0xa 	! LF
    int	0x10

msg_setup:
	.byte 13,10
	.ascii "Now we are in SETUP"
	.byte 13,10,13,10

msg_cursor:
	.byte 13,10
	.ascii "Cursor Pos:"
msg_mem:
	.byte 13,10
	.ascii "Memory Size:"
msg_hd0:
	.byte 13,10
	.ascii "HD0 data:"
.text
endtext:
.data
enddata:
.bss
endbss:
