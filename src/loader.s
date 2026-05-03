.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1 | 1<<2)
.set CHECKSUM, -(MAGIC + FLAGS)
.set HEADER_ADDR, 0
.set LOAD_ADDR, 0
.set LOAD_END_ADDR, 0
.set BSS_END_ADDR, 0
.set ENTRY_ADDR, 0
.set MODE_TYPE, 1
.set WIDTH, 0
.set HEIGHT, 0
.set DEPTH, 0
#.set WIDTH, 640
#.set HEIGHT, 480
#.set DEPTH, 8

.section .multiboot
	.long MAGIC
	.long FLAGS
	.long CHECKSUM
	.long HEADER_ADDR
	.long LOAD_ADDR
	.long LOAD_END_ADDR
	.long BSS_END_ADDR
	.long ENTRY_ADDR
	.long MODE_TYPE
	.long WIDTH
	.long HEIGHT
	.long DEPTH


.section .text
.extern kernelMain
.extern callConstructors
.global loader

loader:
	mov $kernel_stack, %esp
	
	call callConstructors
	

	push %eax
	push %ebx
	
	#comment these registers out if you have to
	#push %ecx
	#push %edx
	
	
	call kernelMain
_stop:
	cli
	hlt
	jmp _stop

.section .bss
.space 4*1024*1024	; # 4 MiB
kernel_stack:
