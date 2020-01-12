;;-----------------_DEFINITIONS ONLY_-----------------------
;; IMPORT FUNCTIONS FROM C
%macro IMPORTFROMC 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    extern _%1
    %1 equ _%1
    %else
    extern %1
    %endif
%rotate 1 
%endrep
%endmacro

;; EXPORT TO C FUNCTIONS
%macro EXPORT2C 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    global _%1
    _%1 equ %1
    %else
    global %1
    %endif
%rotate 1 
%endrep
%endmacro

%define break xchg bx, bx


IMPORTFROMC KernelMain
IMPORTFROMC defaultInterruptHandler, timerHandler, keyHandler

TOP_OF_STACK                equ 0x200000
KERNEL_BASE_PHYSICAL        equ 0x200000

;Address of interrupt dumps
DUMPS_ADDR              equ 0X500000

;For paging
P4_ADDRESS              equ 0x8000
P3_ADDRESS              equ 0x9000
P2_ADDRESS              equ 0xA000

PRESENT_AND_WRITEABLE       equ 0x3
HUGE_PAGE                   equ 0x80 ;(1<<7)this bit tells to not sarch for next page

PSE_BIT                     equ 4

EOI_VAL                     equ 0x20;(1<<5) End of interrupt
PIC1_CMD_REG                equ 0x20
PIC2_CMD_REG                equ 0xA0


;;-----------------^DEFINITIONS ONLY^-----------------------

segment .text
[BITS 32]
ASMEntryPoint:
    cli
    MOV     DWORD [0x000B8000], 'O1S1'
%ifidn __OUTPUT_FORMAT__, win32
    MOV     DWORD [0x000B8004], '3121'                  ; 32 bit build marker
%else
    MOV     DWORD [0x000B8004], '6141'                  ; 64 bit build marker
%endif



    MOV     ESP, TOP_OF_STACK                           ; just below the kernel
    
    ;break

    ;TODO!!! define page tables; see https://wiki.osdev.org ,Intel's manual, http://www.brokenthorn.com/Resources/

    ;1 GBi Table no works we will use 2MB
    MOV     EBX, P3_ADDRESS     ;Addres of the second table
    OR      EBX, PRESENT_AND_WRITEABLE
    MOV     DWORD [P4_ADDRESS], EBX      ;Write entry in table

    MOV     EBX, P2_ADDRESS     ;Addres of the second table
    OR      EBX, PRESENT_AND_WRITEABLE
    MOV     DWORD [P3_ADDRESS], EBX      ;Write entry in table

    ;Map 6mb of virtual memory to physical memory
    XOR     EBX, EBX                ;Make map to 0x0
    OR      EBX, PRESENT_AND_WRITEABLE
    OR      EBX, HUGE_PAGE          ;Make page of 2MB size
    MOV     ECX, 256
    XOR     EDX, EDX
    XOR     EAX, EAX
l1: ;map virtual to physical memory 512Mb
    MOV     DWORD [P2_ADDRESS + EDX], EBX
    MOV     DWORD [P2_ADDRESS + EDX + 4], EAX
    ADD     EBX, 0x200000           ;next adress (2MB)
    ADD     EDX, 8
    loop l1

    ;set bits of cr3 with the PDBR
    MOV     EAX, P4_ADDRESS
    MOV     CR3, EAX

    ;ENable PAE paging
    MOV     EAX, CR4
    OR      EAX, 1 << 5
    MOV     CR4, EAX

    ;TODO!!! transition to 64bits-long mode
    ;Enter long mode
    MOV ECX, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    RDMSR                        ; Read from the model-specific register.
    OR EAX, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    WRMSR 

    ;TODO!!! activate pagging
    MOV     EAX, CR0
    OR      EAX, 0x80000000 ;set bit 31 in CR0 For enable pagin
    MOV     CR0, EAX
    jmp     40:.64biti
[bits 64]
.64biti:
    mov    ax,    48
    mov    ds,    ax
    mov    es,    ax      
    mov    gs,    ax      
    mov    ss,    ax      
    mov    fs,    ax
    
    mov    RAX,   DUMPS_ADDR ;Used in default interrupt
    mov    qword [RAX], 0
    
    ;break
    XOR     RAX, RAX
    MOV     EAX, KernelMain     ; after 64bits transition is implemented the kernel must be compiled on x64
    CALL    RAX
    
    CLI
    HLT

;;--------------------------------------------------------

__cli:
    CLI
    RET

__sti:
    STI
    RET

__magic:
    XCHG    BX,BX
    RET
    
__enableSSE:                ;; enable SSE instructions (CR4.OSFXSR = 1)  
    MOV     RAX, CR4
    OR      RAX, 0x00000200
    MOV     CR4, RAX
    RET
    
__launchInterrupt:
    CMP al, 0
    JNE n0
    INT 0
    ret
n0:
    CMP al, 1
    JNE n1
    INT 1
    ret
n1:
    CMP al, 2
    JNE n2
    INT 2
    ret
n2:
    CMP al, 3
    JNE n3
    INT 3
    ret
n3:
    CMP al, 4
    JNE n4
    INT 4
    ret
n4:
    CMP al, 5
    JNE n5
    INT 5
    ret
n5:
    CMP al, 6
    JNE n6
    INT 6
    ret
n6:
    CMP al, 7
    JNE n7
    INT 7
    ret
n7:
    CMP al, 8
    JNE n8
    INT 8
    ret
n8:
    CMP al, 9
    JNE n9
    INT 9
    ret
n9:
    CMP al, 10
    JNE n10
    INT 10
    ret
n10:
    CMP al, 11
    JNE n11
    INT 11
    ret
n11:
    CMP al, 12
    JNE n12
    INT 12
    ret
n12:
    CMP al, 13
    JNE n13
    INT 13
    ret
n13:
    CMP al, 14
    JNE n14
    INT 14
    ret
n14:
    CMP al, 15
    JNE end
    INT 15
    ret
end:
    ret
    

__defaultInterruptHandler:

    ;break
    ;Save registers an 24 bytes of stack in memory
    push RSI
    push RDI
    mov RSI, DUMPS_ADDR ;First 8 bytes will be a counter
    mov RDI, [RSI]
    add RSI, RDI
    
    ;kindof popad
    mov [RSI], RAX
    add RSI, 8
    mov [RSI], RBX
    add RSI, 8
    mov [RSI], RCX
    add RSI, 8
    mov [RSI], RDX
    add RSI, 8
    mov [RSI], RBP
    add RSI, 8
    mov RDI, [RSP] ; RDI
    mov [RSI], RDI
    add RSI, 8
    mov RDI, [RSP + 8] ; RSI
    mov [RSI], RDI
    add RSI, 8          
    ;RSP + q16 is return address

    MOV RDI, RSI
    LEA RSI, [RSP + 24]
    MOV RCX, 5 
    REP MOVSQ   ;move 5 qwords from stack to memory
    ;Save counter
    mov RSI, DUMPS_ADDR
    mov [RSI], RDI
                        
    ;Dump done
    ;pushad
    push RBX
    push RCX
    push RDX
    push RBP 
    push RAX  ;param -> 58

    mov RCX, RAX ;Param to function
    cld ; C code following the sysV ABI requires DF to be clear on function entry
    call defaultInterruptHandler
    
    ;popad
    pop RAX
    pop RBP
    pop RDX
    pop RCX
    pop RBX

    pop RDI
    pop RSI  
    POP RAX

    iretq

__Divide_by_0:
    PUSH RAX
    MOV RAX, 0
    JMP __defaultInterruptHandler
__Single_step:
    PUSH RAX
    MOV RAX, 1
    JMP __defaultInterruptHandler
__Non_Maskable_Interrupt_Pin:
    PUSH RAX
    MOV RAX, 2
    JMP __defaultInterruptHandler
__Breakpoint:
    PUSH RAX
    MOV RAX, 3
    JMP __defaultInterruptHandler
__Overflow:
    PUSH RAX
    MOV RAX, 4
    JMP __defaultInterruptHandler
__Bounds_check:
    PUSH RAX
    MOV RAX, 5
    JMP __defaultInterruptHandler
__Undefined_OPCode:
    PUSH RAX
    MOV RAX, 6
    JMP __defaultInterruptHandler
__No_coprocessor:
    PUSH RAX
    MOV RAX, 7
    JMP __defaultInterruptHandler
__Double_Fault:
    PUSH RAX
    MOV RAX, 8
    JMP __defaultInterruptHandler
__Coprocessor_Segment_Overrun:
    PUSH RAX
    MOV RAX, 9
    JMP __defaultInterruptHandler
__Invalid_Task_State_Segment:
    PUSH RAX
    MOV RAX, 10
    JMP __defaultInterruptHandler
__Segment_Not_Present:
    PUSH RAX
    MOV RAX, 11
    JMP __defaultInterruptHandler
__Stack_Segment_Overrun:
    PUSH RAX
    MOV RAX, 12
    JMP __defaultInterruptHandler
__General_Protection_Fault:
    PUSH RAX
    MOV RAX, 13
    JMP __defaultInterruptHandler
__Page_Fault:
    PUSH RAX
    MOV RAX, 14
    JMP __defaultInterruptHandler
__Coprocessor_error:
    PUSH RAX
    MOV RAX, 16
    JMP __defaultInterruptHandler
__Alignment_Check:
    PUSH RAX
    MOV RAX, 17
    JMP __defaultInterruptHandler
__Machine_Check:
    PUSH RAX
    MOV RAX, 18
    JMP __defaultInterruptHandler

__keyHandler:
    PUSH keyHandler
    jmp  __interruptHandler
__timerHandler:
    PUSH timerHandler
    jmp  __interruptHandler
__interruptHandler:
    push RAX
    mov  RAX, [RSP + 8]
    PUSH RSI  
    PUSH RDI
    push RBX
    push RCX
    push RDX
    push RBP  
    
    cld ; C code following the sysV ABI requires DF to be clear on function entry
    call RAX
    
    ;popad
    pop RBP
    pop RDX
    pop RCX
    pop RBX
    pop RDI
    pop RSI  
    pop RAX
    add RSP, 8
    jmp __iret_PIC1
__iret_PIC1:
    mov al, EOI_VAL
    out PIC1_CMD_REG, al
    iretq

    
__iret_PIC2:
    mov al, EOI_VAL
    out PIC2_CMD_REG, al ;Only if the interrupt was between 8-15
    out PIC1_CMD_REG, al
    iretq

__iret:
    iretq



EXPORT2C ASMEntryPoint, __cli, __sti, __magic, __enableSSE
EXPORT2C __iret_PIC1, __iret_PIC2, __iret, __launchInterrupt, __defaultInterruptHandler
EXPORT2C __timerHandler, __keyHandler
EXPORT2C __Divide_by_0
EXPORT2C __Single_step
EXPORT2C __Non_Maskable_Interrupt_Pin
EXPORT2C __Breakpoint
EXPORT2C __Overflow
EXPORT2C __Bounds_check
EXPORT2C __Undefined_OPCode
EXPORT2C __No_coprocessor
EXPORT2C __Double_Fault
EXPORT2C __Coprocessor_Segment_Overrun
EXPORT2C __Invalid_Task_State_Segment
EXPORT2C __Segment_Not_Present
EXPORT2C __Stack_Segment_Overrun
EXPORT2C __General_Protection_Fault
EXPORT2C __Page_Fault
EXPORT2C __Coprocessor_error
EXPORT2C __Alignment_Check
EXPORT2C __Machine_Check