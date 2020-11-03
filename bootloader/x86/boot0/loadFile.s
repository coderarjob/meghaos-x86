; ------------------------------------------- MACRO BLOCK BEGINS
; Reads a sector into a buffer
; Input:
;	Argument 1 - sector number
;	Argument 2 - buffer location
; Output:
;	The flags from INT 13 are preserved.
%macro readSector 2
	pusha		; I used push and pop a just to same some memory

	mov ax, %1
	call csh	; seetup the registers properly for INT 13H

	mov ah, 02	; sector read system call
	mov al, 01	; read one sector
	mov bx, %2
	int 0x13

	popa
%endmacro
; ------------------------------------------- MACRO BLOCK ENDS

; LOADS WHOLE FILE FROM THE FLOPPY DISK TO MEMORY.
; NOTE: CANNOT READ FILES THAT ARE LARGER THAN 64KB.
; Input: AX - Memory segment of the destination address
;	 BX - Memory offset in the segment
;	 DX - Memory offset with the filename
; Output: AX - 0 file could not be read.
;	       Or returns the file size if read was successful.

loadFile:
	push ds
	push es
	pusha
	
	xor cx, cx
	; Copy filename from the location pointed to by DS:DX to the local
	; memory segment and storage.
	mov si, dx			; DS:SI
	mov es, cx
	mov di, bootfilename		; ES:DI
	mov cx, 11
	rep movsb

	; Change the DS register to 0. Needed to address local memory.
	mov ds, cx

	; save the output memory location to local memory
	mov [osegment],ax
	mov [osegoffset],bx

	mov cx, RootDirSectors
	mov ax, 19		; root dir starts at sector 19
.readsector:
	readSector ax,buffer

	push cx
	xor bx, bx
.searchRootEntry:
	mov cx, 11
	lea si, [buffer + bx]
	mov di, bootfilename
	repe cmpsb
	je .filefound
	
	; not a match, we go to next entry
	;add bx, 64
	add bx, 32		; Goto the next entry.
	cmp bx, 512		; TODO: Is this really 512??
	jnz .searchRootEntry

.filenotfound:
	pop cx
	inc ax	; next sector
	loop .readsector
	jmp .failed

.filefound:
	pop cx
	; read the file start sector
	mov ax, word [buffer + bx + 0x1A]
	mov [filesector], ax

	; read file size a 32 bit number 
	; Endianess is preserved - Bytes are written to 'fileremsize' in the
	; same order as they appear in the disk.
	mov ax, word [buffer + bx + 0x1C]	; first 16 bits of file size
	mov [fileremsize], ax
	
	; Saves the file size. This value will be returned if success.
	mov [.ret], ax				

	; NOTE: We are only reading 16 bits of size, as we cannot load a file 
	; that is more than 64k in size anyways. This saves a little bit of 
	; memory.

	; mov ax, word [buffer + bx + 0x1E]	; second 16 bits of file size
	; mov [fileremsize+2], ax
.repeat:
	; setup the counter register
	cmp [fileremsize], word 0
	je .readFileEnd

	; read 512 bytes (one sector) if the file remaining size is > 512.
	mov cx, 512

	; check to see if the remaining bytes is > or < 512 (one sector size)
	cmp [fileremsize],word 512
	ja .readDataSector

	; read all the remaining bytes as it is < 512 (one sector size)
	; file remaining size >= 512
	mov cx, [fileremsize]
.readDataSector:
	mov ax, [filesector]
	add ax, 31			; sector = sector -2 + 33
	readSector ax, buffer		; read sector to internal buffer

	; we copy as many bytes in the CX register from the internal buffer to
	; the output buffer
	push es				; preserve the ES value before change
		mov dx, cx
		cld				; set direcection flag = 0 (increment)
		mov si, buffer
		mov ax, [osegment]		; set up destination address
		mov es, ax
		mov di, [osegoffset]
		rep movsb
	pop es				; restore the ES register
	; update remaining size variable.
	sub word [fileremsize], dx	; remaining = remaining - bytes read
	add word [osegoffset], dx	; osegoffset now points to the next
					; location to write to.
.getNextSector:
	; now we get the next sector to read
	mov ax, [filesector]
	mov bx, ax
	shr ax, 1
	add ax, bx			; [filesector] * 3/2

	; AX now has the byte to be read from the FAT. For filesector 3, the
	; byte to be read from FAT is 3 * 3/2 = 4.5.

	; we normalize the byte location in ax.
	; example: byte 513 in FAT table, is byte 1 of sector 2 of disk
	xor dx, dx
	div word [BytesPerSector]
	
	; dx contains the normalized byte to be read from sector in ax
	add ax, [ReservedSector]	; take into account reserved sector

	; read the sector (containing FAT entry)
	readSector ax, buffer

	; read the word located at DX location
	mov bx, dx			; DX cannot be used in effective
					; addtessing. So we use BX
	mov ax, [buffer + bx]

	; check if byte location is odd or even
	test word [filesector], 0x1
	jnz .odd
	
	; Byte location is even
	and ax, 0xFFF
	jmp .checkForLastSector
.odd:
	shr ax, 4
.checkForLastSector:
	cmp ax, 0xFFF
	mov [filesector], ax		; save the sector to the 'filesector'
					; variable, so that we read that sector
					; after we jump
	jnz .repeat
.readFileEnd:
	; file was found and read is complete.
	; Return value is the file size; this was already written when reading
	; the directory entry.
	jmp .end
.failed:
	; file was not found
	; Memory needs to written with 0 again.
	; This is becuause, it will hold the result from the previous read.
	mov [.ret], word 0
.end:
	popa
	mov ax, [.ret]		; AX must be set before we restore DS
	pop es
	pop ds
	iret

.ret dw 0
bootfilename:	resb	11
RootDirSectors:	equ 	14
filesector:	resw 	1
fileremsize	resw 	1 ; even though the file size in FAT12 is 32 bits, we
			  ; cannot load a file that is more than 64K 
			  ; (fits in one segment) with our current logic in 
			  ; this function, so we are allocating only 16 bits.
osegoffset	resw	1
osegment	resw	1

%include "csh.s"
