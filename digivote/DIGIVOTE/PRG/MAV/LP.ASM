;;<?xml version="1.0" encoding="ISO-8859-1"?>
;;<source name="lp.asm">
;;<pvcs>
  ;;<!-- these tags are filled out by version control system during check in -->
  ;;<revision>$Revision: 1.2 $</revision>
  ;;<archiveTimeStamp>$Date: 2005/07/15 10:04:20 $</archiveTimeStamp>
  ;;<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
;;</pvcs>
;;<comments><!-- xml fragment from source code -->
  ;;<fileName>LP.ASM</fileName>
  ;;<creationDate/>
  ;;<description>
    ;;<fr>Fonctions crayon optique</fr>
    ;;<nl>Lichtpen functies</nl></description>
  ;;<comment/>
;;</comments>

;;<header>
;;<prototypes>
public _detection
public _lightpenconnected
public _g_pen

public _p_ri
public _p_ko
;;</prototypes>

dgroup	group	datasg
datasg	segment	'data'

; externals from video.asm
extrn vidmod : BYTE
extrn bytes_lijn : BYTE
extrn r_eclic : BYTE

tekeng		db	0
zaglicht	db	0
wissel		db	0
param1		db	?
p_kol		dw	?
p_rij		dw	?
hold		db	0
p_kon		dw	0
p_rin		dw	0
toetst		db	0

penpos		dw	offset	dgroup:penp
penp		db	400		dup(?)

rest		dw	0


datasg ends

AbsPen SEGMENT byte public 'CODE'
    ASSUME cs:AbsPen,ds:DGROUP

;;</header>
;;<body>

;;<function id="detection">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;=========================================================
_detection          proc far
;=========================================================
	push bp
	mov  bp,sp
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     
    mov  ah,04                  
    mov  bx,0                   ;initialize BX
    mov  cx,0                   ;initialize CX
    int  10h                    
    mov  ah,0                   
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_detection          endp 
;;</code></function>

;;<function id="lightpenconnected">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;=========================================================
_lightpenconnected  proc far
;=========================================================
	push bp
	mov  bp,sp
	push sp
	push ds
    push es                     
    push si                     
    push di                     
    call lightintern            
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_lightpenconnected  endp 
;;</code></function>

;;<function id="g_pen">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;=========================================================
_g_pen              proc far    ;start coordinaten,tekst,attribuut,
;=========================================================
	ARG  mode:word,onderbr:word,pijl:word
	push bp
	mov  bp,sp
	push sp
	push ds
	push es
	push si
	push di
	mov  tekeng,0               ;er mag getekend worden

niet:
	call lightintern            ;lichtpen aangesloten
	cmp  ax,0
	je   eindspr

	call _p_eclic               ;p_rij en p_kol ophalen
	and  al,3                   ;remove bits (only bit 0 and 1 have meaning)
	mov  r_eclic,al             ;toestand muis of pen in alcmp pijl,0
;***********************************************************************
	cmp  p_kol,635
	jl   bn1
	mov  p_kol,635

bn1:
	cmp  p_kol,6
	jg   bn
	mov  p_kol,6
;***********************************************************************

bn:
	cmp  hold,0
	je   doeit                  ;altijd doen als hold = 0 =losgelaten
	cmp  onderbr,1              ;DIAGNOST only
	je   doeit                  ;DIAGNOST only
	cmp  r_eclic,1              ;muis=1 lichtpen
	jg   niet                   ;ingedrukt en niet losgelaten

doeit:
	mov  hold,0
	cmp  pijl,0
	jne  met_pijl
	jmp  gedguy

met_pijl:
	cmp  r_eclic,1
	je   wel_licht
	cmp  r_eclic,3
	je   wel_licht
	jmp  gn_licht

eindspr:
	jmp  eindprog

wel_licht:
	mov  ax,p_rin
	cmp  ax,p_rij
	jne  bewoog
	mov  ax,p_kon
	cmp  ax,p_kol
	jne  bewoog
	cmp  onderbr,1              ;DIAGNOST
	je   bewoog                 ;DIAGNOST
	jmp  gedguy

bewoog:
	cmp  tekeng,1               ;er moet nog getekend worden
	jne  ovgu
	call _retek
	mov  tekeng,0

ovgu:
	cmp  tekeng,0
	jne  nol
	call _tek
	mov  tekeng,1

nol:
	mov  zaglicht,1
	jmp  gedguy

gn_licht:
	cmp  tekeng,1
	jne  gedguy1
	jmp  overov

mniet:
	cmp  onderbr,1
	je   diagnost
	jmp  niet

diagnost:
	cmp  pijl,0
	je   initvar
	mov  hold,0
	cmp  tekeng,1
	jne  initvar
	call _retek

initvar:
	mov  al,r_eclic
	mov  hold,1
	mov  toetst,0               ;stel altijd=0
	jmp  eindprog

overov:
	call _retek
	mov  tekeng,0

gedguy1:


gedguy:
	mov  hold,0
	mov  ax,p_rij
	mov  p_rin,ax
	mov  ax,p_kol
	mov  p_kon,ax
	cmp  r_eclic,3
	je   wverder
	jmp  mniet

wverder:
	cmp  pijl,0
	je   xverder
	call _retek                 ;ri,ko

xverder:
	mov  hold,1
	xor  ah,ah
	mov  al,0
	mov  toetst,0               ;stel altijd=0
	mov  ax,1                   ;lightpen ok

eindprog:
	pop  di
	pop  si
	pop  es
	pop  ds
	pop  sp
	pop  bp
	ret
_g_pen              endp
;;</code></function>

;;<function id="p_ko">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;=========================================================
_p_ko               proc far    
;=========================================================
    push bp                     
    push ds                     
    push es                     
    push si                     
    push di                     
    mov  ax,p_kol               
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  bp                     
    ret                         
_p_ko               endp 
;;</code></function>

;;<function id="p_ri">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;=========================================================
_p_ri               proc far    
;=========================================================
    push bp                     
    push ds                     
    push es                     
    push si                     
    push di                     
    mov  ax,p_rij               
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  bp                     
    ret                         
_p_ri               endp
;;</code></function>

;;<function id="tek">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;=========================================================
;                _tek
; Theorie    : tekenen van de pijl
; Oproep     : _g_pen
; Voorwaarde : tekeng moet op 0 staan, en moet nadien
;              geforceerd worden op 1 (zie : _retek)
;=========================================================

_tek                proc near   ;start coordinaten,tekst,attribuut,
    mov  ax,p_rij               
    mov  bx,p_kol               

    mov  cl,bytes_lijn          ;80
    xor  ch,ch                  
    mul  cx                     ;ax=ax*80
    mov  cx,bx                  ;cx=kol
    shr  cx,1                   
    shr  cx,1                   
    shr  cx,1                   ;cx=kol/8
	add  ax,cx                  ;ax=ax*80+kol/8
    mov  di,ax                  ;di=ax*80+kol/8

    mov  cx,bx                  ;kolom
    and  cl,07                  ;enkel 3 laagste bits
    mov  bl,80h                 ;bl=128
    shr  bl,cl                  ;schuif aantal posities

    mov  param1,0               
    mov  ax,0a000h              ;pagina 0
    mov  es,ax                  ;es wijst naar videobuffer
    mov  ax,dgroup              
    mov  ds,ax                  
    mov  si,penpos              ;adres van tabel
;*****************************************************************************
;resultaat in bl geeft bit
;di geeft byte
;*****************************************************************************
    mov  dx,3ceh                
    mov  al,5                   
    out  dx,al                  
    inc  dx                     
    mov  al,0                   
    out  dx,al                  ;mode ingesteld
    mov  dx,3ceh                
    mov  al,3                   
    out  dx,al                  
    inc  dx                     
    mov  al,0                   
    out  dx,al                  
    cmp  wissel,0               
    je   opm                    
	call _tek12
    jmp  opm1                   

opm:
	call _tek11

opm1:
    ret                         
_tek                endp
;;</code></function>

;;<function id="retek">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;=========================================================
;                _retek
;
; Theorie    : niet hertekenen, maar het verwijderen van de
;              pijl op positie p_rin en p_kon
; Oproep     : _g_pen
; Voorwaarde : tekeng moet op 1 staan (moet geforceerd worden
;                                      na een _tek = teken pijl)
;=========================================================
_retek              proc near   ;start coordinaten,tekst,attribuut,
	mov  ax,p_rin
	mov  bx,p_kon

	mov  cl,bytes_lijn          ;80
	xor  ch,ch
	mul  cx                     ;ax=ax*80
	mov  cx,bx                  ;cx=kol
	shr  cx,1
	shr  cx,1
	shr  cx,1                   ;cx=kol/8
	add  ax,cx                  ;ax=ax*80+kol/8
	mov  di,ax                  ;di=ax*80+kol/8
		
	mov  cx,bx                  ;kolom
	and  cl,07                  ;enkel 3 laagste bits
	mov  bl,80h                 ;bl=128
	shr  bl,cl                  ;schuif aantal posities

	mov  ax,0a000h              ;pagina 0
	mov  es,ax                  ;es wijst naar videobuffer
    mov  param1,2               

    mov  ax,dgroup              
    mov  ds,ax                  
	mov  si,penpos              ;adres van tabel

								;resultaat in bl geeft bit
								;di geeft byte

    mov  dx,3ceh                
    mov  al,5                   
    out  dx,al                  
    inc  dx                     
    mov  al,0                   
	out  dx,al                  ;mode ingesteld
    mov  dx,3ceh                
    mov  al,3                   
    out  dx,al                  
    inc  dx                     
    mov  al,0                   
    out  dx,al                  
    cmp  wissel,0               
    je   rt1                    
	call _tek22
    jmp  rt3                    

rt1:
	call _tek21

rt3:
    call _alles2                
    call _deinit2               
    ret                         
_retek              endp
;;</code></function>

;;<function id="lightintern">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;---------------------------------------------------------
lightintern         proc near
;---------------------------------------------------------
	mov  ah,4
	mov  bx,'FT'
	mov  cx,'GD'
	mov  dx,'PN'
	int  10h                    ;test of lightpen nog geconnecteerd is
	cmp  ax,'HI'
	jne  req_not_supported
	or   bx,bx
	jz   pen_still_there
	xor  ax,ax                  ;pen niet geconnecteerd
	ret

pen_still_there:
req_not_supported:
	mov  ax,1                   ;pen geconnecteerd
	ret
lightintern         endp
;;</code></function>

;;<function id="tek12">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;---------------------------------------------------------
_tek12              proc near   
;---------------------------------------------------------
    mov  cx,17                  
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,16                  
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,13                  
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _linoni                
    mov  cx,14                  
    call _lini                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,9                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
	mov  cx,9
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,6                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
	mov  cx,2
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
	mov  cx,1
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
	mov  cx,1
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _reci                  
    mov  cx,5                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _reci                  
	mov  cx,9
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,4                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,4                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _linksi                
	mov  cx,1
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
	mov  cx,1
    call _reconi                
    mov  cx,14                  
    call _lini                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _reci                  
    mov  cx,13                  
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,16                  
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,16                  
    call _reci                  

    mov  param1,1               
    mov  cx,2                   
    call _boveni                
;   sub  di,160                 
    mov  cx,1                   
    call _linksi                
    mov  cx,13                  
    call _lini                  
    mov  cx,2                   
    call _boveni                
;   sub  di,160                 
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
	mov  cx,1
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
	mov  cx,1
    call _rechtsi               
    mov  cx,3                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,3                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,4                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,8                   
    call _reci                  
    mov  cx,1                   
    call _linboi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,3                   
    call _lini                  
    mov  cx,1                   
    call _recboi                
    mov  cx,4                   
    call _reci                  
    mov  cx,1                   
    call _linboi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _recboi                
    mov  cx,2                   
    call _bovi                  
    mov  cx,1                   
    call _recboi                
	mov  cx,2
    call _lini                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _recboi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
	mov  cx,1
    call _bovi                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,5                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,8                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,8                   
    call _reci                  
    mov  cx,3                   
    call _rechtsi               
    mov  cx,2                   
    call _boveni                
;   sub  di,160
    mov  cx,13                  
    call _lini                  
    ret                         
_tek12              endp 
;;</code></function>

;;<function id="tek11">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _tek11
;*********************************************************

_tek11              proc near   
    mov  cx,1                   
    call _boveni                
    mov  cx,14                  
    call _ondi                  
    mov  cx,3                   
    call _recboi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _recboi                
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
	call _bovi
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linboi                
    mov  cx,2                   
    call _bovi                  
    mov  cx,4                   
    call _reci                  
    mov  cx,9                   
    call _linboi                

    mov  param1,1               

    mov  cx,11                  
    call _ondi                  
    mov  cx,1                   
    call _recboi                
    mov  cx,8                   
    call _bovi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,6                   
    call _ondi                  
    mov  cx,2                   
    call _onderi                
;   add  di,160                 
    mov  cx,1                   
    call _reci                  
    mov  cx,7                   
    call _bovi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,8                   
    call _ondi                  
    mov  cx,2                   
	call _onderi
;   add  di,160                 
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _bovi                  
    mov  cx,2                   
    call _onderi                
;   add  di,160                 
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,7                   
    call _boveni                
;   sub  di,560                 
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _bovi                  
    ret                         
_tek11              endp 
;;</code></function>

;;<function id="tek21">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _tek21
;*********************************************************

_tek21              proc near   
	mov  cx,1
    call _boveni                
    mov  cx,14                  
    call _ondi                  
    mov  cx,3                   
    call _recboi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _recboi                
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linboi                
	mov  cx,2
    call _bovi                  
    mov  cx,4                   
    call _reci                  
    mov  cx,9                   
    call _linboi                


    mov  cx,11                  
    call _ondi                  
    mov  cx,1                   
    call _recboi                
    mov  cx,8                   
    call _bovi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,6                   
    call _ondi                  
    mov  cx,2                   
    call _onderi                
;   add  di,160                 
    mov  cx,1                   
    call _reci                  
    mov  cx,7                   
    call _bovi                  
    mov  cx,1                   
    call _reconi                
    mov  cx,8                   
    call _ondi                  
    mov  cx,2                   
    call _onderi                
;   add  di,160                 
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _bovi                  
    mov  cx,2                   
    call _onderi                
;   add  di,160
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,7                   
    call _boveni                
;   sub  di,560                 
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _bovi                  
    ret                         
_tek21              endp 
;;</code></function>

;;<function id="tek22">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _tek22
;*********************************************************

_tek22              proc near   
    mov  cx,17                  
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,16                  
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,13                  
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _linoni                
    mov  cx,14                  
    call _lini                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,9                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
	call _ondi
    mov  cx,1                   
    call _lini                  
    mov  cx,9                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,6                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
	call _lini
    mov  cx,1                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
	call _linksi
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,2                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,2                   
	call _lini
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _reci                  
    mov  cx,5                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _reconi                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,3                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
	call _linoni
    mov  cx,1                   
    call _reci                  
    mov  cx,9                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,4                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,4                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,3                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
	call _lini
    mov  cx,2                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,2                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
	call _rechtsi
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _reconi                
    mov  cx,14                  
    call _lini                  
    mov  cx,1                   
    call _linoni                
    mov  cx,1                   
    call _reci                  
    mov  cx,13                  
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,16                  
    call _lini                  
    mov  cx,1                   
    call _ondi                  
    mov  cx,16                  
    call _reci                  

    mov  cx,2                   
    call _boveni                
;   sub  di,160                 
    mov  cx,1                   
    call _linksi                
    mov  cx,13                  
    call _lini                  
    mov  cx,2                   
    call _boveni                
    sub  di,160                 
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
	mov  cx,1
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
	mov  cx,1
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,3                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,3                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,4                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,8                   
    call _reci                  
    mov  cx,1                   
    call _linboi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,3                   
    call _lini                  
    mov  cx,1                   
    call _recboi                
    mov  cx,4                   
    call _reci                  
    mov  cx,1                   
    call _linboi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _recboi                
    mov  cx,2                   
    call _bovi                  
	mov  cx,1
    call _recboi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _recboi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,2                   
    call _lini                  
    mov  cx,1                   
    call _linboi                
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,1                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
	mov  cx,2
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _linksi                
    mov  cx,1                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,5                   
    call _reci                  
    mov  cx,1                   
    call _rechtsi               
    mov  cx,2                   
    call _reci                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,8                   
    call _lini                  
    mov  cx,1                   
    call _bovi                  
    mov  cx,8                   
    call _reci                  
    mov  cx,3                   
    call _rechtsi               
	mov  cx,2
    call _boveni                
;   sub  di,160                 
    mov  cx,13                  
    call _lini                  
    ret                         
_tek22              endp 
;;</code></function>

;;<function id="reci">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _reci
;*********************************************************

_reci               proc near   

recli:
    shr  bl,1                   ;schuif een bit op
    jnc  reczi                  
    rcr  bl,1                   
    inc  di                     

reczi:
    call _pixi                  
    loop recli                  
    ret                         
_reci               endp 
;;</code></function>

;;<function id="rechtsi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _rechtsi
;*********************************************************

_rechtsi            proc near   

itr3:
    shr  bl,1                   
	jnc  ier1
    rcr  bl,1                   
    inc  di                     

ier1:
    loop itr3                   
    ret                         
_rechtsi            endp 
;;</code></function>

;;<function id="linksi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _linksi
;*********************************************************

_linksi             proc near   

itr4:
    shl  bl,1                   
    jnc  ier2                   
    rcl  bl,1                   
    dec  di                     

ier2:
    loop itr4                   
    ret                         
_linksi             endp 
;;</code></function>

;;<function id="lini">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _lini
;*********************************************************

_lini               proc near   

ilinl:
    shl  bl,1                   
    jnc  ilinz                  
    rcl  bl,1                   
    dec  di                     

ilinz:
    call _pixi                  
    loop ilinl                  
    ret                         
_lini               endp 
;;</code></function>

;;<function id="recboi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _recboi
;*********************************************************

_recboi             proc near   

irecbol:
    shr  bl,1                   
    jnc  irecboz                
    rcr  bl,1                   
    inc  di                     

irecboz:
    push ax                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    sub  di,ax                  
    pop  ax                     
    call _pixi                  
    loop irecbol                
    ret                         
_recboi             endp 
;;</code></function>

;;<function id="linboi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _linboi
;*********************************************************

_linboi             proc near   

ilinbol:
    shl  bl,1                   
    jnc  ilinboz                
    rcl  bl,1                   
    dec  di                     

ilinboz:
    push ax                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    sub  di,ax                  
    pop  ax                     
    call _pixi                  
    loop ilinbol                
    ret                         
_linboi             endp 
;;</code></function>

;;<function id="ondi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _ondi
;*********************************************************

_ondi               proc near   

uondli:
    push ax                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    add  di,ax                  
    pop  ax                     
    call _pixi                  
    loop uondli                 
    ret                         
_ondi               endp 
;;</code></function>

;;<function id="bovi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _bovi
;*********************************************************

_bovi               proc near   

ibovl:
    push ax                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    sub  di,ax                  
    pop  ax                     
    call _pixi                  
    loop ibovl                  
    ret                         
_bovi               endp 
;;</code></function>

;;<function id="onderi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _onderi
;*********************************************************

_onderi             proc near   

ondli:
    push ax                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    add  di,ax                  
    pop  ax                     
    loop ondli                  
    ret                         
_onderi             endp 
;;</code></function>

;;<function id="boveni">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _boveni
;*********************************************************
_boveni             proc near   

uibovl:
    push ax                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    sub  di,ax                  
    pop  ax                     
    loop uibovl                 
    ret                         
_boveni             endp 
;;</code></function>

;;<function id="linoni">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _linoni
;*********************************************************

_linoni             proc near   

ilinonl:
    shl  bl,1                   
    jnc  ilinonz                
    rcl  bl,1                   
    dec  di                     

ilinonz:
    push ax                     
    mov  al,bytes_lijn          
	xor  ah,ah
    add  di,ax                  
    pop  ax                     
    call _pixi                  
    loop ilinonl                
    ret                         
_linoni             endp 
;;</code></function>

;;<function id="reconi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _reconi
;*********************************************************

_reconi             proc near   

ireconl:
	shr  bl,1
    jnc  ireconz                
	rcr  bl,1
    inc  di                     

ireconz:
    push ax                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    add  di,ax                  
    pop  ax                     
    call _pixi                  
    loop ireconl                
    ret                         
_reconi             endp 
;;</code></function>

;;<function id="p_eclic">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _p_eclic
;*********************************************************

_p_eclic            proc near   ;start coordinaten,tekst,attribuut,
    mov  ah,04                  
    mov  bx,0                   
    mov  cx,0                   
    int  10h                    
    mov  ah,0                   ;;enkel al bekijken
    cmp  vidmod,14              
    jne  gop                    
    xchg ch,cl                  
    xor  ch,ch                  

gop:
    mov  p_rij,cx               
    mov  p_kol,bx               

mgy:
    ret                         
_p_eclic            endp
;;</code></function>

;;<function id="alles2">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _alles2
;*********************************************************

_alles2             proc near   
    mov  dx,3ceh                
    mov  al,8                   
    out  dx,al                  
    inc  dx                     
    mov  al,11111111b           ;te diplayen bitmap
    out  dx,al                  

    mov  dx,3c4h                
    mov  al,2                   
    inc  dx                     
    mov  al,0ffh                ;alle bitmappen bereikbaar
    out  dx,al                  
	ret
_alles2             endp 
;;</code></function>

;;<function id="deinit2">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _deinit2
;*********************************************************

_deinit2            proc near   
    mov  dx,03c4h               
    mov  al,02                  
    out  dx,al                  
    inc  dx                     
    mov  al,0ffh                
    out  dx,al                  
    mov  dl,0ceh                
    mov  al,03h                 
    out  dx,al                  
	inc  dx
    xor  al,al                  
    out  dx,al                  
    dec  dx                     
    mov  al,08h                 
    out  dx,al                  
    inc  dx                     
    mov  al,0ffh                
    out  dx,al                  
    mov  dl,0c8h                
    mov  al,0bh                 
    out  dx,al                  
    inc  dx                     
    in   al,dx                  
    mov  ah,al                  
    dec  dx                     
    mov  al,00                  
    out  dx,al                  
    ret                         
_deinit2            endp 
;;</code></function>

;;<function id="pixi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _pixi
;*********************************************************

_pixi               proc near   

    cmp  param1,2               
    je   fagh                   
    call _lees                  ;lees pixelkleur resultaat in bh
    mov  ds:[si],bh             
    add  si,1                   ;volgend adres

    cmp  param1,0               
    je   fag3                   
    mov  bh,15                  
    jmp  fag2                   

fag3:
    mov  bh,0                   ;dl             ;neem seconds complement
    jmp  fag2                   

fagh:
    mov  bh,ds:[si]             ;uitlezen
    add  si,1                   

fag2:
    mov  dx,3ceh                
    mov  al,8                   
    out  dx,al                  
    inc  dx                     
    push ax                     
    push bx                     
    push dx                     
    xor  dx,dx                  
	mov  ax,di
    mov  bx,80                  
    div  bx                     ;dx rest
    mov  rest,dx                
    pop  dx                     
    pop  bx                     
    pop  ax                     
    mov  al,bl                  ;00000100b
    cmp  rest,0                 
    jne  gverder                
    and  al,00000011b           

gverder:
    out  dx,al                  

    mov  dx,3c4h                ;adresregister
    mov  al,2                   ;map maskeer
    out  dx,al                  
    inc  dx                     
    mov  al,0ffh                ;mapmask op ff zetten
    out  dx,al                  

    mov  al,es:[di]             ;uitlezen
    mov  al,0                   ;
    mov  es:[di],al             ;0 wegschrijven
                                ;overal kleur nul schrijven

    mov  dx,3c4h                
    mov  al,2                   
    out  dx,al                  
    inc  dx                     
    mov  al,bh                  
    out  dx,al                  
    mov  al,es:[di]             
    mov  al,0ffh                
    mov  es:[di],al             
    ret                         
_pixi               endp 
;;</code></function>

;;<function id="lees">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _lees
;*********************************************************

_lees               proc near   ;start coordinaten,tekst,attribuut,
    mov  ah,0                   ;teller=0
    mov  bh,0                   ;resultaat=0

beg:
    mov  dx,3ceh                
    mov  al,5                   
    out  dx,al                  
    inc  dx                     
    mov  al,0                   
    out  dx,al                  
    mov  dx,3ceh                
    mov  al,4                   
    out  dx,al                  
    inc  dx                     
    mov  al,ah                  ;bitmap teller 0 1 2 3 naargelang pass
    out  dx,al                  
    mov  al,es:[di]             ;lees bitmap uit,hele byte
    and  al,bl                  ;and met bitdefinitie, 1bit
    cmp  al,0                   ;is het betreffende bit=0
    je   beg1                   
    mov  dx,1                   ;zet 1
    push cx                     ;hou cx even bij
	mov  cl,ah                  ;bitmapteller  bijhouden in cl
    shl  dx,cl                  ;shift naar juiste pos 0 1 2 3 4    16
    pop  cx                     
    or   bh,dl                  ;bh geeft uiteindelijk kleur

beg1:
    cmp  ah,3                   
    je   endr                   
	inc  ah
    jmp  beg                    

endr:
    ret                         ;bh resultaat kleur
_lees               endp        ;ah teller
;;</code></function>

AbsPen ENDS
END
;*****************************************************************************
;;</body>
;;</source>
;;<!-- eof -->

