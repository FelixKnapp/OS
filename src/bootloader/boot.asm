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

	; read sth from floppy
	mov [ebr_drive_number], dl

	mov ax, 1
	mov cl, 1
	mov bx, 0x7E00
	call read_disk_sectors

    ; print OS name and version
    mov si, msg_os_name
    call puts
    mov si, msg_os_vers
    call puts

	cli
    hlt

;
; Errors
;

floppy_error:
	mov si, msg_read_failed
	call puts
	jmp wait_key_reboot

wait_key_reboot:
	mov ah, 0
	int 16h				; wait for keypress
	jmp 0FFFFh:0		; jmp to Bios beginning

.halt:
	cli					; disable interrupts, keep CPU in halt state 
    hlt

;
; Disk routines
;

;
; Converts an LBA adress to a CHS adress
; Parameters: 
; 	-ax: LBA adress
; Returns:
; 	-cx [bits 0-5]: sector number
; 	-cx [bits 6-15]: cylinder
; 	-dh: head 
; 
lba_to_chs:

	push ax
	push dx

	xor dx, dx							; dx = 0
	div word [bdb_sectors_per_track]	; ax = LBA / SectorsPerTrack
										; ax = LBA % SectorsPerTrack
	inc dx								; dx = (LBA % SectorsPerTrack)
	mov cx, dx							; cx = sector

	xor dx, dx							; dx = 0
	div word [bdb_heads]				; ax = (LBA / SectorsPerTrack) / Heads = cylinder
										; dx = (LBA / SectorsPerTrack) % Heads = head
	mov dh, dl							; dl = head
	mov ch, al							; ch = cylinder (lower 8 bits)
	shl ah, 6
	or cl, ah							; put upper 2 bits of cylinder in CL

	pop ax
	mov dl, al							; restore DL
	pop ax
	
	ret

;
; Reads sectors from disk
; Parameters: 
; 	-ax: LBA adress
;	-cl: number of sectors to read(max 128)
;	-dl: drive number
;	-es:bx: store read number
; 
read_disk_sectors:

	push ax								; safe to be modified registers
	push bx
	push cx
	push dx
	push di

	
	push cx								; save cl temporarily
	call lba_to_chs
	pop ax								; AL = numbers to read
	
	mov ah, 02h
	mov di, 3							; retry count

.retry:
	pusha
	stc									; set carry flag
	int 13h
	jnc .done							; jump if carry isnt set

	; read failed
	popa
	call disk_reset

	dec di
	test di, di
	jnz .retry

.fail:
	; all attempts exhausted
	jmp floppy_error

.done:
	popa

	pop di								; restore modified registers
	pop dx
	pop cx
	pop bx
	pop ax

	ret

;
; Resets disk controller
; Parameters
; 	dl: drive number
;
disk_reset:

	pusha
	mov ah, 0
	int 13h
	jc floppy_error
	popa

	ret

msg_os_name: db 'KnappOS', ENDL, 0
msg_os_vers: db 'Version 0.012', ENDL, 0

msg_read_failed: db 'Reading from Disk has failed!'

times 510-($-$$) db 0
dw 0AA55h