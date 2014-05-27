public _pciAvail
public _found595
public _pxl_loaded

_PROG  segment	byte	public	'code'
ASSUME cs:_prog

;*********************************************************
;                _pciAvail
;*********************************************************

_pciAvail proc far
	; Check and see if the PCI BIOS is available.
	mov 	ah, 0B1h
	mov 	al, 1
	int 	1Ah

	cmp		ax, 0
	je  	noPCI
	cmp 	dl, 'P'
	jne 	noPCI
	cmp 	dh, 'C'
	jne 	noPCI

	mov 	ax, 1
	jmp 	exit1

noPCI:
	clc
	mov 	ax, 0

exit1:
	ret

_pciAvail endp

;*********************************************************
;                _found595
;*********************************************************

_found595 proc far
	find_pci_rc			db	?
	bus_num				db	?
	dev_and_func_num	db	?
	read_config_rc		db	?
	int_line			db	?
	base_addr1			dw	?
	base_addr2			dw	?
	STATUS				dw	?
	INT_LEVEL			db	?

	; Search for a PXL-595 device
	mov		ah, 0b1h			; pci_function_id
	mov		al, 02				; find_pci_device
	mov		cx, 0595h			; device ID
	mov		dx, 15ffh			; vendor ID
	xor		si, si				; index
	int		1ah
	jnc		found				; found pci
	mov		ax, 0
	jmp		read_config_word_err

found:
	mov		ax, 1
	mov		find_pci_rc, ah
	mov		bus_num, bh
	mov		dev_and_func_num, bl

	; access configuration space of pci
	mov		ah, 0b1h
	mov		al, 9				; read config word
	mov		bh, bus_num
	mov		bl, dev_and_func_num
	mov		di, 14h				; register number for base address 1
	int		1ah
	mov		read_config_rc, ah
	jc		read_config_word_err
	and		cx, 0fffeh
	mov		base_addr1, cx

	mov		ah, 0b1h
	mov		al, 9				; read config word
	mov		bh, bus_num
	mov		bl, dev_and_func_num
	mov		di, 018h			; register number for base address 2
	int		1ah
	mov		read_config_rc, ah
	jc		read_config_word_err
	and		cx, 0fffeh
	mov		base_addr2, cx

	mov		ah, 0b1h
	mov		al, 9				; read config word
	mov		bh, bus_num
	mov		bl, dev_and_func_num
	mov		di, 3ch				; register number for interrupt line
	int		1ah
	mov		read_config_rc, ah
	jc		read_config_word_err
	mov		int_line, cl

	; enable plx chip
	mov		dx, base_addr1
	add		dx, 4ch
	mov		al, 43h
	out		dx, al

	mov		ax, base_addr2
	mov		STATUS, ax
	mov		al, int_line
	mov		INT_LEVEL, al

read_config_word_err:
	ret

_found595 endp

;*********************************************************
;                _pxl_loaded
;*********************************************************

_pxl_loaded proc far
	mov		ah, 4
	mov		bx, 'FT'
	mov		cx, 'GD'
	mov		dx, 'ta'		; return 'HI' in the ax register
	int		10H
	cmp		ax, 'HI'
	jne		not_loaded
	mov		ax, 1
	jmp		exit_pxl_loaded

not_loaded:
	mov		ax, 0

exit_pxl_loaded:
	ret

_pxl_loaded endp

_prog ends
end