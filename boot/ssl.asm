%define break xchg bx, bx

KERNEL_SECTORS equ 50

[org 0x7E00]
[bits 16]
SSL: 
    mov    ah,    02h               ; parameters for calling int13 (ReadSectors) - read the Kernel
    mov    al,    KERNEL_SECTORS    ; read KERNEL_SECTORS sectors (hardcoded space for Kernel)
    mov    ch,    00h     
    mov    cl,    0Bh               ; starting from sector 11 - skip first 10 sectors (the MBR + SSL)
    mov    dh,    00h     
    mov    bx,    0x9200            ; memory from 0x7E00 - 0x9200 used by SLL;  0x9200 - 0x9FFFF is unused
    int    13h             
    jc    .fail
    ;one copy dont work
    mov    ah,    02h               ; parameters for calling int13 (ReadSectors) - read the Kernel
    mov    al,    5    ; read KERNEL_SECTORS sectors (hardcoded space for Kernel)
    mov    ch,    00h     
    mov    cl,    0Bh               ; starting from sector 11 - skip first 10 sectors (the MBR + SSL)
    mov    dh,    00h     
    mov    bx,    0xF600            ; memory from 0x7E00 - 0x9200 used by SLL;  0x9200 - 0x9FFFF is unused
    int    13h             
    jnc    .success 
.fail:
    break;
    cli                    ; we should reset drive and retry, but we hlt
    hlt                     
 
.success:                      
    ;break
    cli                    ; starting RM to PM32 transition
    lgdt   [GDT]
    mov    eax,    cr0
    or     al,     1
    mov    cr0,    eax     ; we are in protected mode but we need to set the CS register  
    jmp    8:.bits32       ; we change the CS to 8 (index of FLAT_DESCRIPTOR_CODE32 entry)

.bits32:
[bits 32]
    mov    ax,    16       ; index of FLAT_DESCRIPTOR_DATA32 entry
    mov    ds,    ax
    mov    es,    ax      
    mov    gs,    ax      
    mov    ss,    ax      
    mov    fs,    ax 

    cld
    mov    ecx,    KERNEL_SECTORS*512  ; KERNEL_SECTORS * sector_size
    mov    esi,    0x9200              ; source
    mov    edi,    0x200000            ; destination
    rep    movsb                       ; copy the kernel from 0x9200 to 0x200000
    
    mov    [ds:0xb8000], BYTE 'O' ;Ceva byte este pentru culare, jumate e background si foreground
    mov    [ds:0xb8002], BYTE 'K'
    
    ;break
    
is_A20_on?:   ;Verificam ca e pornit feater-ul de 20 de linii de adresa. E ceva legacy, e ca sa mearga alte programe
    pushad
    mov edi,0x112345  ;odd megabyte address.
    mov esi,0x012345  ;even megabyte address.
    mov [esi],esi     ;making sure that both addresses contain diffrent values.
    mov [edi],edi     ;(if A20 line is cleared the two pointers would point to the address 0x012345 that would contain 0x112345 (edi)) 
    cmpsd             ;compare addresses to see if the're equivalent.
    popad
    jne A20_on        ;if not equivalent , A20 line is set.

    cli               ;if equivalent , the A20 line is cleared.
    hlt
 
A20_on:
    
    ;break
    
    mov    eax, 0x201000    ; the hardcoded ASMEntryPoint of the Kernel
						    ; e incarcat un .exe si primul sector e incarcat la 1000.
    call   eax
    
    cli                     ; this should not be reached 
    hlt

;
; Data
;
GDT:
    .limit  dw  GDTTable.end - GDTTable - 1
    .base   dd  GDTTable


FLAT_DESCRIPTOR_CODE64  equ 0x00AF9A000000FFFF  ; Code: Execute/Read
FLAT_DESCRIPTOR_DATA64  equ 0x00AF92000000FFFF  ; Data: Read/Write
FLAT_DESCRIPTOR_CODE32  equ 0x00CF9A000000FFFF  ; Code: Execute/Read
FLAT_DESCRIPTOR_DATA32  equ 0x00CF92000000FFFF  ; Data: Read/Write
FLAT_DESCRIPTOR_CODE16  equ 0x00009B000000FFFF  ; Code: Execute/Read, accessed
FLAT_DESCRIPTOR_DATA16  equ 0x000093000000FFFF  ; Data: Read/Write, accessed
    
GDTTable:
    .null     dq 0                         ;  0
    .code32   dq FLAT_DESCRIPTOR_CODE32    ;  8
    .data32   dq FLAT_DESCRIPTOR_DATA32    ; 16
    .code16   dq FLAT_DESCRIPTOR_CODE16    ; 24
    .data16   dq FLAT_DESCRIPTOR_DATA16    ; 32 ;TODO!!! add 64-bits descriptors
    .code64   dq FLAT_DESCRIPTOR_CODE64    ; 40
    .data64   dq FLAT_DESCRIPTOR_DATA64    ; 48 ;TODO!!! add 64-bits descriptors
    .end: