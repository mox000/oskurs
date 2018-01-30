# bootblock.s

# .equ symbol, expression
# These directive set the value of the symbol to the expression
  .equ    BOOT_SEGMENT,      0x07c0
  .equ    DISPLAY_SEGMENT,   0xb800
  .equ    KERNEL_SEGMENT,    0x0000
  .equ    KERNEL_OFFSET,     0x1000

# You need to decide where to put the stack
 .equ    STACK_SEGMENT,     0x07e00
 .equ    STACK_POINTER,     0x200    #tilsvarer 512 hex

.text                   # Code segment
.globl    _start        # The entry point must be global
.code16                 # Real mode
.org 0x0

#
# The first instruction to execute in a program is called the entry
# point. The linker expects to find the entry point in the "symbol" _start
# (with underscore).
#

_start:
  jmp beyondReservedSpace

kernelSize:
  .word 0               # bootimage will write size of kernel, in sectors


beyondReservedSpace: # samme som over: i eksempelet
# setup stack
  movw  $STACK_SEGMENT, %ax
  movw %ax, %ss
  movw $STACK_POINTER, %sp

# setup stack
  movw $BOOT_SEGMENT, %ax
  movw %ax, %ds

# sektorer, header, tracks, 
  movb (kernelSize), %al # numbers of sectors to read
  movb $2, %ah 
  movb $0, %ch # track
  movb $2, %cl # sector 
  movb $0, %dh # head

  movw $KERNEL_SEGMENT, %bx
  movw %bx, %es

  movw $KERNEL_OFFSET, %bx

  int $0x13

  movw $DISPLAY_SEGMENT, %bx
  movw %bx, %es

  movw $0, %ax
  movw %ax, %ds
/*
  # Clear screen
  movw $0x0a00, %ax     # Fill with black background / green foreground
  movw $2000, %cx       # Number of characters (80x25 screen in text mode = 2000)
  xorw %di, %di         # DI = 0
  rep stosw
  */
  movb $0x4b, %es:(0x0) # Write 'K' in the upper left corner of the screen

  ljmp $KERNEL_SEGMENT, $KERNEL_OFFSET

forever:
  jmp forever # Loop forever
/*
PrintCharacter:
  movw AH, 0x0E
  movw BH, 0x00
  movw bl, 0x07

  int 0x10
  ret 

PrintString
nextCharacter:
  mov al,    # Bite from the string and put it in al
*/
