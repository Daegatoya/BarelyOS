bits 64
section .text
global Inb
Inb:
	mov dx, di
	in al, dx
	ret