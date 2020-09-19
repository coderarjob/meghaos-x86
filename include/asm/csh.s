

%ifndef __CSH_S__
	%define __CSH_S__

    ; Calculates the cylinder/track, circular sector and head for INT 0x14
    ; Input: 
    ;	AX - Sector number
    ; Output:
    ;	ch - Cylinder number
    ; 	dl - sector number (1 - 63)
    ;	dh - head number
    csh:
        push ax
        push bx

        ; disk sector (circular sector)
        xor dx, dx	; clear dx again, for second div
        div word [SectorsPerTrack]
        add dx, 1	; sector starts from 1 in INT 13
        mov bl, dl	; save the sector in BL

        ; track with more than one head
        xor dx, dx
        div word [HeadCount]
        mov ch, al	; cylinder
        mov cl, bl	; sector
        mov dh, dl	; head
        mov dl, 0	; disk 0
        
        pop bx
        pop ax
    ret
%endif ; __CSH_S__

