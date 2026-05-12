org 0x7C00
bits 16


%define ENDL 0x0D, 0x0A

;
; FAT12 header
;
jmp short start
nop

bdb_oem:                        db 'MSWIN4.1'
bdb_bytes_per_sector:           dw 512
bdb_sectors_per_cluster:        db 1
bdb_reserved_sectors:           dw 1
bdb_fat_count:                  db 2
bdb_dir_entries_count:          dw 0E0h
bdb_sectors_count:              dw 2880                     ; 2880 *512 =1,44MB
bdb_media_descriptor_type:      db 0F0h
bdb_sectors_per_fat:            dw 9
bdb_sectors_per_track:          dw 18
bdb_heads:                      dw 2
bdb_hidden_sectors:             dd 0
bdb_large_sector_count:         dd 0

; extended boot record
ebr_drive_number:               db 0
                                db 0
ebr_signature:                  db 29h
ebr_volume_id:                  db 12h, 34h, 56h, 78h
ebr_volume_label:				db 'KnappOS    '		; 11 bytes padded with spaces
ebr_system_id:					db 'FAT12   '			; 8 bytes padded with spaces

;
; code goes here
;

start:
    jmp main


;
; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts:
    ; save registers we will modify
    push si
    push ax
    push bx

.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0x0E        ; call bios interrupt
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si    
    ret
    

main:
    ; setup data segments
    mov ax, 0           ; can't set ds/es directly
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00      ; stack grows downwards from where we are loaded in memory

    ; print hello world message
    mov si, msg_os_name
    call puts
    mov si, msg_os_vers
    call puts
    hlt

.halt:
    jmp .halt



msg_os_name: db 'KnappOS', ENDL, 0
msg_os_vers: db 'Version 0.01', ENDL, 0


times 510-($-$$) db 0
dw 0AA55h