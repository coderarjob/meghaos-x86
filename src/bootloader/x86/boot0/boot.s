; MEGHAOS V2 BOOT LOADER - x86
;
; Contains FAT12 driver, that reads a loader file from the disk to a buffer and
; and jumps to the loader code.
;
; -------------------------
; Changes in version 0.02 (4th July 2019)
; -------------------------
; * Removed the 'Welcome' message. Directly boots into the splashscreen now.
; * Removed 'filereqsize', 'filesize' (these were not really needed, we want to
;   load the whole file, so stating required size was useless).
; * Changed 'osegoffset' from EQU to RESW.
; * Resulted in reducing the file size from 503 bytes to 430 bytes in v0.02.
;
; -------------------------
; Changes in version 0.03 (3rd August 2019)
; -------------------------
; * Objective of file is now to load a 'loader' file and jump to it.
; * Loads a loader program into memory and jumps to it.
;
; * Sets up stack explicitly to be just before the bootloader.
;       Top of the stack is now 0x6C0:FFF
;       Stack is 4KiB in size
;
; * loadFlie and printString functions can now be accessed using INT 
;       loadFile    --> INT 30
;       printString --> INT 31
;   These two functions can be accessed from any segment and can be used by the
;   'loader' program to load rest of the OS and show some message on screen.
;
; * To save storage, error messages are now
;   |-----------------------|---------------------|
;   |       ERROR           |     MESSAGE         |
;   |-----------------------|---------------------|
;   |Floppy cannot be reset |       FL            |
;   |-----------------------|---------------------|
;   |Loader fie not found   |       LD            |
;   |-----------------------|---------------------|
; 
; * INT 30 is now used in the bootloader as well.
; * INT 31 is now used to display text in the bootloader.
; * INT 30 (loadFile) now preserves DS segment.
; * Bootloader sets just the DS register before jump to the loader. Previously
;   it set ES, FS, GS as well.
; * BootLoader file size is now 509 bytes.
;
; -------------------------
; Changes in version 0.31 (3rd August 2019)
; -------------------------
; * loadFile now do not take the segment number (CX register was used for this)
;   of the filename. It is now assumed to be same as DS of the callee.
; * Bootloader file size is 503 bytes.
; * Data section that is used by loadFile function was moved to loadFile.s
;
; -------------------------
; Changes in version 0.32 (11th August 2019)
; -------------------------
; * loadFile now returns the loaded file size in AX
;   If it failed to load the file, AX is 0
; * The 2nd Stage loader is loaded at location 0x800:0x100
;
; -------------------------
; Changes in version 20200830 (Using date as version number)
; -------------------------
; * Stack Segment is changed to 0
; * Stack Top is at 0x7C00
; * This makes DS = SS
; * OEMLabel changed from 'ARJOBBOOT' to 'MEGHAOS2'
; * VolumeLabel changed from 'ARJOBOS' to 'MEGHAOS2'
;
; -------------------------
; Changes in version 20200919 (Using date as version number)
; -------------------------
; * Data and other included files taken from the end of the file to between BPB
;   and MAIN CODE BLOCK.
; * Method printstr is moved from printstr.s file is boot0.s
; * printstr.s file is removed from source tree.
; * 2nd Stage loader file is renamed from 'LOADER' to 'BOOT1.FLT'.

	org 0x7C00
; ******************************************************
; BIOS PARAMETER BLOCK
; ******************************************************

	jmp near boot_main

	OEMLabel		    db "MEGHAOS2"	; Disk label
	BytesPerSector		dw 512		    ; Bytes per sector
	SectorsPerCluster	db 1		    ; Sectors per cluster
	ReservedSector		dw 1		    ; Reserved sectors for boot record
	NumberOfFats		db 2		    ; Number of copies of the FAT
	RootEntries		    dw 224		    ; Number of entries in root dir
						                ; (224 * 32 = 7168 = 14 sectors to read)
	LogicalSectors		dw 2880		    ; Number of logical sectors
	MediumByte		    db 0F0h		    ; Medium descriptor byte
	SectorsPerFat		dw 9		    ; Sectors per FAT
	SectorsPerTrack		dw 18		    ; Sectors per track (36/cylinder)
	HeadCount		    dw 2		    ; Number of sides/heads
	HiddenSectors		dd 0		    ; Number of hidden sectors
	LargeSectors		dd 0		    ; Number of LBA sectors
	DriveNo		        dw 0		    ; Drive No: 0
	Signature	        db 41		    ; Drive signature: 41 for floppy
	VolumeID	        dd 00000000h	; Volume ID: any number
	VolumeLabel	        db "MEGHAOS    "; Volume Label: any 11 chars
	FileSystem	        db "FAT12   "	; File system type: don't change!

; ******************************************************
; MACRO BLOCK
; ******************************************************

%macro printString 1
	push si
	mov si, %1
	int 0x31
	pop si
%endmacro

; ******************************************************
; DATA BLOCK
; ******************************************************
%include "mos.inc"
%include "loadFile.s"

drivefailedstr:  db	 'D',0
filenotfoundstr: db  'F',0
bootfile:        db  BOOT1_FILE

; ******************************************************
; CODE BLOCK
; ******************************************************

; Prints a string zascii to screen
; Input: Pointer to string in DS:SI
; Output: None
printstr:
	push ax

.repeat:
	lodsb
	cmp al, 0
    je .end

	mov ah, 0xE
	int 0x10
    jmp .repeat

.end:
	pop ax
iret

; ******************************************************
; MAIN CODE BLOCK
; ******************************************************
boot_main:	

	; Setup the Stack
	; The Stack is 4k in size and starts at location 0x7BFF (0x7C00 -1)
	cli		                            ; disable interrupts
    xor ax, ax
	mov ss, ax
	mov sp, 0x7C00
	sti		                            ; enable interrupts

	; reset the floppy drive
	mov ah, 0
	mov dl, 0
	int 0x13
	jc failed_drive_error 	            ; drive error

	; install loadFile to IVT
	xor ax, ax
	mov gs, ax
	mov [gs:0x30*4], word loadFile
	mov [gs:0x30*4+2], cs

	; install printstr to IVT
	mov [gs:0x31*4], word printstr
	mov [gs:0x31*4+2], cs

	; Read the directory and search for file

	mov ax, LOADER_SEG
	mov bx, LOADER_OFF
	mov dx, bootfile

	int 0x30
	cmp ax, 0			            ; Check if read was successful
	je failed_file_not_found	    ; Show error message if read failed.

	; -------------------- JUMP TO LOADER
	; Read was a success, we prepare the segment registers and jump.
	mov ax, LOADER_SEG
	mov ds, ax
	mov ss, ax
	jmp LOADER_SEG:LOADER_OFF
	;--------------------- 

failed_drive_error:
	printString drivefailedstr
	jmp exit
failed_file_not_found:
	printString filenotfoundstr
exit:	
	jmp $

; ******************************************************
; END OF BOOT LOADER
; ******************************************************
    times 510 - ($-$$) db 0
		dw 	0xAA55

; ******************************************************
; FILE IO BUFFER
; ******************************************************
buffer:
