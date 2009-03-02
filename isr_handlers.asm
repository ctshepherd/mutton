SECTION .text

%macro INTR_NOEC 1
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro INTR_EC 1
    nop
    nop
    push byte %1
    jmp isr_common_stub
%endmacro

global int_stubs
int_stubs:
INTR_NOEC 0
INTR_NOEC 1
INTR_NOEC 2
INTR_NOEC 3
INTR_NOEC 4
INTR_NOEC 5
INTR_NOEC 6
INTR_NOEC 7
INTR_EC 8
INTR_NOEC 9
INTR_EC 10
INTR_EC 11
INTR_EC 12
INTR_EC 13
INTR_EC 14
INTR_NOEC 15
INTR_NOEC 16
INTR_NOEC 17
INTR_NOEC 18
INTR_NOEC 19
INTR_NOEC 20
INTR_NOEC 21
INTR_NOEC 22
INTR_NOEC 23
INTR_NOEC 24
INTR_NOEC 25
INTR_NOEC 26
INTR_NOEC 27
INTR_NOEC 28
INTR_NOEC 29
INTR_NOEC 30
INTR_NOEC 31

extern fault_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10   ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   ; Push us the stack
    push eax
    mov eax, fault_handler
    call eax       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
