;;<?xml version="1.0" encoding="ISO-8859-1"?>
;;<source name="video.asm">
;;<pvcs>
  ;;<!-- these tags are filled out by version control system during check in -->
  ;;<revision>$Revision: 1.3 $</revision>
  ;;<archiveTimeStamp>$Date: 2005/07/15 10:04:20 $</archiveTimeStamp>
  ;;<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
;;</pvcs>
;;<comments><!-- xml fragment from source code -->
  ;;<fileName>VIDEO.ASM</fileName>
  ;;<creationDate>01/04/93</creationDate>
  ;;<description>
    ;;<fr>Fonctions video</fr>
    ;;<nl>Video functies</nl></description>
  ;;<comment>
;
; functions accessible from C using mavasm.h
;
  ;;</comment>
;;</comments>

;;<header>
;;<prototypes>
public _videom				
public _achtersp
public _bladvul
public _lijnvert
public _lijnhor
public _kad
public _zegsp
public _knopsp
;;</prototypes>

dgroup	group	datasg
datasg	segment	'data'

	;
	; variables made accessible for lp.asm
	;
	vidmod		db	0
	public vidmod

	bytes_lijn	db	0
	public bytes_lijn

	r_eclic		db	0
	public r_eclic

blank		db	0
param		dw	?
getal		db	0
gtabel		dw	?
hulp		dw	?
parinhoud	dw	offset	dgroup:trab
trab		db	17		dup(?)
linen		db	0
tijdl		dw	?
totlijn		dw	?
segm		dw	?
offs		dw	?
lengte		db  	?
kleur1		db	?
kolom		db	?
kolom1		db	?
koloml		dw	?
koloml1		dw	?
rijl		dw	?
rijl1		dw	?
invers		db	0

datasg ends

_PROG	segment	byte	public	'code'
ASSUME  cs:_prog,ds:DGROUP

;;</header>
;;<body>

;;<function id="mode0">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;****************************************************************************
;*  procedure om een byte op het scherm te displayen in schrijfmode 0       *
;*  bron : PC-handboek voor programmeurs pagina 238                         *
;*  ah = bitmap van de te displayen byte                                    *
;*  bl = kleur van de te displayen byte                                     *
;*  Er wordt verondersteld dat men in schrijfmode 0 zit                     *
;*  verandert na oproep dx,al,                                              *
;*  opgelet : es moet naar videopagina wijzen   (0a000h)                    *
;*            di moet de offset bevatten in videobuffer                     *
;*            voor pagina 1 moet 8000h worden bijgeteld                     *
;****************************************************************************

;*********************************************************
;                _mode0
;*********************************************************

_mode0              proc near   
    mov  dx,3ceh                
    mov  al,8                   
    out  dx,al                  
    inc  dx                     
    mov  al,ah                  ;te diplayen bitmap doorgeven
    out  dx,al                  

    mov  dx,3c4h                
    mov  al,2                   
    out  dx,al                  
    inc  dx                     
    mov  al,0ffh                ;alle kleurmaps bereikbaar
    out  dx,al                  

    mov  al,es:[di]             ;leesoperatie
    mov  al,0                   
    mov  es:[di],al             ;schrijf overal 0

    mov  dx,3c4h                
    mov  al,2                   
    out  dx,al                  
    inc  dx                     
    mov  al,bl                  ;enkel geselecteerde maps bereikbaar
    out  dx,al                  
    mov  al,es:[di]             
    mov  al,0ffh                
    mov  es:[di],al             ;schrijf overal een 1
    ret                         
_mode0              endp 
;;</code></function>

;;<function id="achtersp">
;;<heading>
  ;;<description>
    ;;<fr>Colorier une partie de l'écran</fr>
    ;;<nl>Gedeelte vh scherm inkleuren</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>r1, kol1, r2, kol2</name>
      ;;<description>
        ;;<fr>Coordonnées du rectangle</fr>
        ;;<nl>Coördinaten vd rechthoek</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kleur</name>
      ;;<description>
        ;;<fr>Couleur du rectangle</fr>
        ;;<nl>Kleur vd rechthoek</nl>
      ;;</description>
    ;;</parameter>
  ;;</parameters>
;;</heading>
;;<code>
;*********************************************************
;                _achtersp
;*********************************************************

_achtersp           proc far    
    ARG  r1:word,kol1:word,r2:word,kol2:word,kleur:word
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     
    mov  ax,kleur               
    mov  kleur1,al              
    mov  ax,kol2                
    mov  koloml1,ax             
    mov  ax,r2                  
    mov  rijl1,ax               
    mov  ax,kol1                
    mov  koloml,ax              
    mov  ax,r1                  
    mov  rijl,ax                

    mov  ax,0a000h              
    mov  es,ax                  ;es wijst naar videobuffer blad 0

    mov  dx,3ceh                
    mov  al,8                   
    out  dx,al                  
    inc  dx                     
    mov  al,11111111b           ;te diplayen bitmap
    out  dx,al                  

    mov  dx,3c4h                
    mov  al,2                   
    out  dx,al                  

    inc  dx                     
    mov  al,0ffh                
    out  dx,al                  

    mov  ax,rijl                ;10
    mov  bl,bytes_lijn          
    xor  bh,bh                  
    mul  bx                     ;beginrij                    800
    mov  bx,ax                  ;bx is beginpointer rij

    mov  ax,koloml              ;10
    mov  cl,3                   
    shr  ax,cl                  ;delen door 8            	   1
    add  ax,bx                  ;di waarde begin byte  		 801
    mov  di,ax                  ;0
    mov  ax,koloml              ;0
    mov  bx,7                   
    and  ax,bx                  ;enkel 7 laagste bits    	   0
    mov  cl,al                  ;0
    mov  al,255                 ;11111111
    shr  al,cl                  ;schuif aantal enen weg
    mov  kolom,al               ;beginwaarde      		11111111

    mov  ax,koloml1             ;639
    mov  bx,7                   
    and  ax,bx                  ;7
    mov  cl,al                  ;0
    inc  cl                     
    mov  al,255                 ;11111111
    shr  al,cl                  ;juiste eindwaarde     	11111111
    mov  ah,11111111b           ;11111111
    xor  al,ah                  ;00000000 0
                                ;10000000 1
    mov  kolom1,al              ;11110000
    mov  ax,koloml              ;10
    mov  cl,3                   
    shr  ax,cl                  ;ax=koloml/8            	   1
    mov  bx,ax                  ;bx=koloml/8            	   1

    mov  ax,koloml1             ;500
    mov  cl,3                   ;3
    shr  ax,cl                  ;ax=koloml1/8           	  62
    sub  ax,bx                  ;ax=koloml1/8-koloml/8  	  61
    cmp  ax,0                   
    je   fhu                    
    dec  ax                     ;-1                     	  60
    jmp  fhu1                   

fhu:
    mov  al,255                 
    mov  ah,kolom1              
    and  kolom,ah               

fhu1:
    mov  lengte,al              ;lengte in bytes        	  60

    mov  bl,kleur1              
    mov  ax,rijl1               ;200
    sub  ax,rijl                ;190
    inc  ax                     ;191
    mov  cx,ax                  ;aantal rijen

hn1:
    push cx                     ;191
    push di                     ;pointer

    mov  ah,kolom               ;00111111
    call _mode0                 
    inc  di                     
    xor  ch,ch                  
    mov  cl,lengte              ;60 bytes
    cmp  cx,0                   
    je   ghp                    
    cmp  cx,255                 
    je   ghp1                   

hn:
    push cx                     
    mov  ah,11111111b           ;alle bytes
    call _mode0                 
    inc  di                     ;volgende byte
    pop  cx                     
    loop hn                     

ghp:
    mov  ah,kolom1              
    call _mode0                 

ghp1:
    pop  di                     
    pop  cx                     
    mov  al,bytes_lijn          
    xor  ah,ah                  
    add  di,ax                  
    loop hn1                    
    call _deinit                
    call _alles                 
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_achtersp           endp 
;;</code></function>

;;<function id="bladvul">
;;<heading>
  ;;<description>
    ;;<fr>Remplir l'écran complet d'une couleur</fr>
    ;;<nl>Volledig scherm opvullen in een bepaalde kleur</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kleur</name>
      ;;<description>
        ;;<fr>Couleur d'écran</fr>
        ;;<nl>Kleur vh scherm</nl>
      ;;</description>
    ;;</parameter>
  ;;</parameters>
  ;;<comment>99 = wit, sneller dan 15.
  ;;</comment>
;;</heading>
;;<code>
; ***************************************************************************
; * Procedure om een volledig blad te vullen in een bepaalde kleur          *
; * Er worden achtereenvolgens (28000 maal) een byte geschreven             *
; * Indien wit werkt de procedure veel sneller (kleur=99)                   *
; * gebruik bladvul (kleur,pagina)                                           *
; ***************************************************************************

;*********************************************************
;                _bladvul
;*********************************************************

_bladvul            proc far    
    ARG  kleur:word             
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     

    mov  ax,kleur               
    mov  kleur1,al              

    mov  cx,0a000h              
    mov  es,cx                  
    mov  di,0                   
    mov  al,bytes_lijn          
    xor  ah,ah                  
    mov  bx,totlijn             
    mul  bx                     
    mov  cx,ax                  

    mov  bl,kleur1              

    mov  ah,11111111b           ;alle bits in byte

bpo1:
    cmp  bl,99                  
    jne  brt                    
    mov  es:[di],ah             
    jmp  brt1                   

brt:
    call _mode0                 

brt1:
    add  di,1                   
    loop bpo1                   
    call _deinit                
    call _alles                 
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_bladvul            endp 

;;</code></function>

;;<function id="zegsp">
;;<heading>
  ;;<description>
    ;;<fr>Ecrire texte à l'écran</fr>
    ;;<nl>Schrijft tekst op het scherm</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>r1, kol</name>
      ;;<description>
        ;;<fr>Coordonnées position début du texte</fr>
        ;;<nl>Coördinaten beginpositie van de tekst</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>tekstofs</name>
      ;;<description>
        ;;<fr>texte à afficher</fr>
        ;;<nl>tekst</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>tekstofs</name>
      ;;<description>
        ;;<fr>texte à afficher</fr>
        ;;<nl>tekst</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>tekstseg</name>
      ;;<description/>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kleur</name>
      ;;<description>
        ;;<fr>couleur du texte</fr>
        ;;<nl>kleur vd tekst</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>leng</name>
      ;;<description>
        ;;<fr>longueur du texte</fr>
        ;;<nl>lengte vd tekst</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>inv</name>
      ;;<description>
        ;;<fr>inverser les couleurs</fr>
        ;;<nl>0 =  kleur op achtergrond, 1 = karakters in achtergrondkleur op kleur</nl>
      ;;</description>
    ;;</parameter>
  ;;</parameters>
;;</heading>
;;<code>
;*********************************************************
;                _zegsp
;*********************************************************

_zegsp              proc far    
    ARG  r1:word,kol:word,tekstofs:word,tekstseg:word,kleur:word,leng:word,inv:word
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     

    mov  ax,inv                 
    mov  invers,al              
    mov  ax,leng                
    mov  lengte,al              
    mov  ax,kleur               
    mov  kleur1,al              
    mov  ax,tekstofs            
    mov  offs,ax                
    mov  ax,tekstseg            ;!!!!!!!!!!!!!! was dx
    mov  segm,ax                
    mov  ax,r2                  
    mov  rijl1,ax               
    mov  ax,kol                 
    mov  koloml,ax              
    mov  ax,r1                  
    mov  rijl,ax                

    cmp  lengte,0               
    jne  verv2                  
    jmp  verv3                  

verv2:
    call _zegi1                 
    call _alles                 
    call _deinit                

verv3:
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_zegsp              endp 
;;</code></function>

;;<function id="zegi">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _zegi
;*********************************************************

_zegi               proc near   
    cmp  bytes_lijn,40          
    jne  bg                     
    mov  ax,rijl                
    push ax                     
    mov  cl,5                   
    shl  ax,cl                  
    mov  bx,ax                  
    pop  ax                     
    mov  cl,3                   
    shl  ax,cl                  
    add  bx,ax                  
    jmp  res                    

bg:
    cmp  bytes_lijn,80          
    jne  kl1                    
    mov  ax,rijl                ;ax=rij
    push ax                     
    mov  cl,6                   
	shl  ax,cl                  ;*64
    mov  bx,ax                  ;bx=ax*64

    pop  ax                     
    mov  cl,4                   
    shl  ax,cl                  
    add  bx,ax                  ;bx=ax*64+ax*16
    jmp  res                    

kl1:
    cmp  bytes_lijn,100         
    jne  kl2                    
    mov  ax,rijl                ;ax=rij
    push ax                     
    mov  cl,6                   
	shl  ax,cl                  ;*1024
    mov  bx,ax                  ;bx=ax*64
    pop  ax                     
    push ax                     
    mov  cl,5                   
    shl  ax,cl                  
    add  bx,ax                  ;bx=ax*64+ax*32
    pop  ax                     
    mov  cl,2                   
    shl  ax,cl                  
    add  bx,ax                  ;resultaat in bx

    jmp  res                    

kl2:
    mov  ax,rijl                ;ax=rij
    mov  cl,7                   
	shl  ax,cl                  ;*1024
    mov  bx,ax                  ;bx=ax*64

res:
    mov  ax,0                   
    mov  al,kolom               ;bx=begincoord 0->100
    add  ax,bx                  ;di wijst naar byte in videobuffer
                                ;0->59999
    mov  bx,0a000h              ;
    mov  es,bx                  ;es wijst naar segment buffer
    mov  di,ax                  ;es:[di] wijst naar videobuffer
                                ;0->59999

    mov  dx,segm                ;dx is segment
    mov  bx,offs                ;bx is offset te displayen
                                ;dx=segm
                                ;bx=offs
    mov  cx,0                   
    mov  cl,lengte              ;cx=aantal te displayen
    mov  ax,dgroup              
    mov  ds,ax                  

kuter1:
    mov  si,gtabel              ;begin bitmap
    push ds                     ;ds=dgroup op stack
    mov  ds,dx                  ;ds=segment
    mov  al,ds:[bx]             ;eerste byte te displayen,waarde ophalen
    pop  ds                     ;nu
    xor  ah,ah                  
    push cx                     ;bx op stack
    push bx                     
    cmp  linen,8                
    jne  fj1                    
    mov  cl,3                   
    shl  ax,cl                  ;bx=ax*8
    jmp  olm                    

fj1:
    cmp  linen,14               
    jne  fj2                    
    mov  bx,ax                  ;bx=ax
    mov  cl,4                   
    shl  bx,cl                  ;bx=ax*16
    mov  cx,bx                  ;cx=ax*16
    mov  bx,ax                  ;bx=ax
    shl  bx,1                   ;bx=ax*2
    sub  cx,bx                  ;cx=ax*14
    mov  ax,cx                  ;ax=ax*14
    jmp  olm                    

fj2:
    mov  cl,4                   
    shl  ax,cl                  ;bx=ax*16

olm:
    pop  bx                     
    pop  cx                     ;bx van stack
    jmp  overg                  

tusdoe:
    jmp  kuter1                 

overg:
    add  si,ax                  ;wijst naar eerste byte bitmap
    push bx                     
    push cx                     
    mov  cl,linen               
    xor  ch,ch                  
    push dx                     

    mov  bl,kleur1              
    mov  bh,invers              

kuterug:
    push ds                     
    mov  ds,hulp                
    mov  ah,ds:[si]             ;bitmap uitlezen
    pop  ds                     
    cmp  bl,99                  
    jne  zwwi1                  
    mov  al,ah                  
    jmp  zwwi                   

zwwi1:
    cmp  bh,0                   
    je   fag                    
    mov  al,11111111b           
    xor  ah,al                  

fag:
    call _mode0                 
    jmp  gef                    

zwwi:
    cmp  bh,0                   
    je   fgh                    
    mov  ah,11111111b           
    xor  al,ah                  

fgh:
    mov  es:[di],al             

gef:
    inc  si                     ;volgende byte bitmap
    mov  al,bytes_lijn          
    xor  ah,ah                  
    add  di,ax                  ;volgende scanlijn
    loop kuterug                
    mov  al,bytes_lijn          
    mov  bl,linen               
    mul  bl                     ;ax=bytes_lijn*lijnen
    pop  dx                     
    pop  cx                     
    pop  bx                     
    inc  bx                     ;volgende byte ophalen

    sub  di,ax                  ;terug naar begin
    add  di,1                   
    loop tusdoe                 

    call _deinit                
    ret                         
_zegi               endp 
;;</code></function>

;;<function id="zegi1">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _zegi1
;*********************************************************

_zegi1              proc near   
    cmp  bytes_lijn,40          
    jne  bg1                    
    mov  ax,rijl                
    push ax                     
    mov  cl,5                   
    shl  ax,cl                  
    mov  bx,ax                  
    pop  ax                     
    mov  cl,3                   
    shl  ax,cl                  
    add  bx,ax                  
    jmp  ures                   

bg1:
    cmp  bytes_lijn,80          
    jne  ukl1                   
    mov  ax,rijl                ;ax=rij
    push ax                     
    mov  cl,6                   
	shl  ax,cl                  ;*64
    mov  bx,ax                  ;bx=ax*64
    pop  ax                     
    mov  cl,4                   
    shl  ax,cl                  
    add  bx,ax                  ;bx=ax*64+ax*16
    jmp  ures                   

ukl1:
    cmp  bytes_lijn,100         
    jne  ukl2                   
    mov  ax,rijl                ;ax=rij
    push ax                     
    mov  cl,6                   
	shl  ax,cl                  ;*1024
    mov  bx,ax                  ;bx=ax*64
    pop  ax                     
    push ax                     
    mov  cl,5                   
    shl  ax,cl                  
    add  bx,ax                  ;bx=ax*64+ax*32
    pop  ax                     
    mov  cl,2                   
    shl  ax,cl                  
    add  bx,ax                  ;resultaat in bx
    jmp  ures                   

ukl2:
    mov  ax,rijl                ;ax=rij
    mov  cl,7                   
	shl  ax,cl                  ;*1024
    mov  bx,ax                  ;bx=ax*64

ures:
    mov  ax,koloml              ;0-799
    mov  cl,3                   
    shr  ax,cl                  ;ax=koloml/8
    add  ax,bx                  ;ax=rijl*100+koloml/8
    mov  bx,0a000h              ;bx=0a000h
    mov  es,bx                  ;es=0a000h
    mov  di,ax                  ;wijst naar eerste byte in buffer
                                ;di wijst naar eerste byte te displayen
                                ;in videobuffer
    mov  ax,koloml              
    mov  cx,7                   
    and  ax,cx                  ;getal van 0 to 7
    mov  getal,al               ;aantal keren te shiften

    mov  dx,segm                ;dx is segment
    mov  bx,offs                ;bx is offset te displayen
    inc  lengte                 ;1 byte meer dan lengte
    xor  ch,ch                  
    mov  cl,lengte              ;cx=aantal te displayen
                                ;cx = lengte in bytes
    mov  ax,dgroup              
    mov  ds,ax                  ;wijst naar dgroup

akuter1:
    mov  si,gtabel              ;si wijst naar begin bitmap
    mov  ax,parinhoud           
    mov  param,ax               ;param wijst naar houdtabel
    push ds                     ;ds=dgroup op stack
    mov  ds,dx                  ;ds=segment van te displayen bytes
    mov  al,ds:[bx]             ;eerste byte te displayen,waarde ophalen
    xor  ah,ah                  ;ax heeft asciwaarde
    pop  ds                     
    push cx                     ;cx op stack = nog te displayen leng
    push bx                     ;bx op stack = offset te displayen
    cmp  linen,8                
    jne  ufj1                   
    mov  cl,3                   
    shl  ax,cl                  ;bx=ax*8
    jmp  uolm                   
ufj1:
    cmp  linen,14               
    jne  ufj2                   
    mov  bx,ax                  ;bx=ax
    mov  cl,4                   
    shl  bx,cl                  ;bx=ax*16
    mov  cx,bx                  ;cx=ax*16
    mov  bx,ax                  ;bx=ax
    shl  bx,1                   ;bx=ax*2
    sub  cx,bx                  ;cx=ax*14
    mov  ax,cx                  ;ax=ax*14
    jmp  uolm                   

ufj2:
    mov  cl,4                   
    shl  ax,cl                  ;bx=ax*16

uolm:
    pop  bx                     ;bx=offset te displayen
    pop  cx                     ;nog te displayen leng
    add  si,ax                  ;wijst naar eerste byte bitmap
    push bx                     ;bijhouden offset
    push cx                     ;bijhouden aantal
    push dx                     ;segment bijhouden
    mov  dx,cx                  ;bijhouden aantal te displayen
    xor  ch,ch                  
    mov  cl,linen               ;14 lijnen
    jmp  akuterug               

akut:
    jmp  akuter1                

akuterug:
    push cx                     
    push dx                     ;aantal bytes te displayen
    push ds                     
    mov  ds,hulp                
    mov  ah,ds:[si]             ;bitmap uitlezen
    pop  ds                     
    cmp  invers,1               
    je   afag                   
    mov  al,11111111b           
    xor  ah,al                  ;ah eventueel inverteren

afag:
    xor  al,al                  ;ah is uitgelezen bitmap
    mov  cl,getal               ;aantal keren shiften
    shr  ax,cl                  ;ah-al ah voor nu al voor volgende
                                ;ah enkel de te displayen bits
                                ;al de volgende maal te displayen bits
    push si                     ;bijhouden pointer tabel
    mov  si,param               ;pointer naar bijhoudtabel
    cmp  dl,lengte              ;eerste byte ?
    je   agh1                   
    mov  bl,ds:[si]             ;ophalen overschot vorige keer eerste bits
    jmp  agh2                   

agh1:
    mov  bl,0                   ;er is geen overschot van vorige keer
                                ;bl houd bitmap vorige keer

agh2:
    cmp  dl,1                   ;laatste keer
    jne  aklo                   
    xor  ah,ah                  ;ah=0 nu te displayen gedeelte

aklo:
    or   ah,bl                  ;gedeelte voor nu met vorige keer
    mov  ds:[si],al             ;eerste overschot wegschrijven volgend
    pop  si                     ;pointer naar tabel
    cmp  kleur1,99              
    jne  hu                     
    mov  al,ah                  
    mov  es:[di],al             
    jmp  hu1                    

hu:
    mov  bl,kleur1              ;kleur
    call _mode0                 

hu1:
    inc  param                  ;volgende param pointer naar v adres
    inc  si                     ;volgende byte bitmap
    mov  al,bytes_lijn          
    xor  ah,ah                  
    add  di,ax                  ;volgende scanlijn
    pop  dx                     ;aantal bytes te displayen
    pop  cx                     ;aantal opnieuw poppen
    loop akuterug               
    mov  al,bytes_lijn          
    mov  bl,linen               
    mul  bl                     ;ax=bytes_lijn*lijnen
    pop  dx                     ;segment bijhouden
    pop  cx                     ;aantal
    pop  bx                     ;offset
    inc  bx                     ;volgende byte ophalen
    sub  di,ax                  ;teug naar begin
    inc  di                     
    loop akut                   
    ret                         
_zegi1              endp 
;;</code></function>

;;<function id="knopsp">
;;<heading>
  ;;<description>
    ;;<fr>Bouton de longueur variable</fr>
    ;;<nl>Knop met variabele lengte</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>r1</name>
      ;;<description>
        ;;<fr>Rangée de début du bouton</fr>
        ;;<nl>Beginrij vd knop</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kol1</name>
      ;;<description>
        ;;<fr>Colonne de début du bouton</fr>
        ;;<nl>Beginkolom vd knop</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kol2</name>
      ;;<description>
        ;;<fr>Colonne de fin du bouton</fr>
        ;;<nl>Eindkolom vd knop</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>tekstofs, tekstseg</name>
      ;;<description>
        ;;<fr>Pointeur vers texte</fr>
        ;;<nl>Pointer naar de tekst</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>leng</name>
      ;;<description>
        ;;<fr>Longueur du texte</fr>
        ;;<nl>Lengte vd tekst</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kleur</name>
      ;;<description>
        ;;<fr>Couleur du texte</fr>
        ;;<nl>kleur vd tekst</nl>
      ;;</description>
    ;;</parameter>
  ;;</parameters>
;;</heading>
;;<code>
;*********************************************************
;                _knopsp
;*********************************************************

_knopsp             proc far    
    ARG  r1:word,kol1:word,kol2:word,tekstofs:word,tekstseg:word,leng:word,kleur:word
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     

    mov  ax,kleur               
    mov  kleur1,al              
    mov  ax,leng                
    mov  lengte,al              
    mov  ax,tekstseg            
    mov  segm,ax                
    mov  ax,tekstofs            
    mov  offs,ax                
    mov  ax,kol2                
    mov  koloml1,ax             
    mov  ax,kol1                
    mov  koloml,ax              
    mov  ax,r1                  
    mov  rijl,ax                

    call _init                  ;es gedefinieerd
		
    mov  ax,rijl                ;ax=rij    210
    mov  cl,bytes_lijn          
    xor  ch,ch                  ;80
    mul  cx                     ;ax=di-waarde begin rij  16800

    mov  bx,koloml              ;bx=koloml =15
    mov  cl,3                   ;3
    shr  bx,cl                  ;bx=koloml/8   1
		
    add  ax,bx                  ;di waarde byte  168001
    mov  di,ax                  ;di waarde byte  168001 

    mov  al,bytes_lijn          
    xor  ah,ah                  
    sub  di,ax                  ;een rij hoger   167921
		
    mov  ax,koloml              ;cx=koloml     15
    mov  bx,7                   ;7
    and  ax,bx                  ;0-7           7
    mov  cx,ax                  
    mov  al,128                 
    shr  al,cl                  ;ah heeft de juiste waarde
    mov  ah,al                  
    mov  cx,1                   
    call _links                 ;een positie naar links
                                ;di en ah waarde aangepast
    push ax                     ;ax=2 
    mov  ax,koloml1             ;ax=koloml1          74 
    sub  ax,koloml              ;ax=koloml1-koloml   59
    inc  ax                     ;bitafstand gevraagd 60
    mov  dx,ax                  ;bijhouden bitafstand dx=60
    push dx                     
		
    xor  ah,ah                  
    mov  al,lengte              ;ax=lengte van woord   7
    mov  cl,3                   
    shl  ax,cl                  ;ax=bitlengte lengte * 8 in bits =56
				
    sub  dx,ax                  ;bitafstand -lengte in bits  4
    shr  dx,1                   ;delen door 2                2
    add  dx,koloml              ;beginwaarde voor tekst      17
    mov  koloml,dx              ;beginwaarde tekst           17
    pop  dx                     ;bitafstand                  60
    pop  ax                     ;ah waarde                    2

    mov  bl,0                   ;teken in het zwart het kader
    mov  cx,dx                  ;60
    push dx                     
    call _rec                   
    mov  cl,linen               
    xor  ch,ch                  
    add  cx,3                   
;	mov  cx,17
    call _ond                   
    pop  dx                     
    mov  cx,dx                  
    push dx                     
    call _lin                   
    mov  cl,linen               
    xor  ch,ch                  
    add  cx,3                   
;   mov  cx,17                  
    call _bov                   
    pop  dx                     
	mov  bl,15                  ;maak het inwendige wit
;	mov  bl,7
	mov  cx,1
	call _rechts
	dec  dx
	dec  dx
	mov  cl,linen
	xor  ch,ch
	shr  cx,1
;   mov  cx,7                   ;6 x achter elkaar

rstag:
	push cx
	push dx
	mov  cx,1
	call _ond
    pop  dx                     
    push dx                     
    mov  cx,dx                  
    call _rec                   
    mov  cx,1                   
    call _ond                   
    pop  dx                     
    mov  cx,dx                  
    push dx                     
    call _lin                   
    pop  dx                     
    pop  cx                     
    cmp  cx,0                   
    je   rfb                    
    dec  cx                     
    jmp  rstag                  

rfb:
    push bx                     
    push di                     
    push dx                     
    push ax                     
    mov  invers,1               
    call _zegi1                 
    pop  ax                     ;ah waarde ophalen
    pop  dx                     ;dx waarde ophalen
    pop  di                     ;di waarde ophalen
    pop  bx                     ;bx=kleurwaarde ophalen
    mov  cx,1                   
    call _links                 
    mov  bl,3                   
;   mov  bl,7                   ;teken grijze randen
    inc  dx                     
    mov  cx,dx                  
    push dx                     
    call _rec                   
    mov  cl,linen               
    xor  ch,ch                  
    inc  cx                     
;   mov  cx,15                  
    call _bov                   
    mov  cx,1                   
    call _linon                 
    mov  cl,linen               
    xor  ch,ch                  
    dec  cx                     
;   mov  cx,13                  
    call _ond                   
    pop  dx                     
    dec  dx                     
    dec  dx                     
    dec  dx                     
    mov  cx,dx                  
    call _lin                   

gedo:
    call _deinit                
    call _alles                 
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_knopsp             endp 
;;</code></function>

;;<function id="kad">
;;<heading>
  ;;<description>
    ;;<fr>Dessine un cadre</fr>
    ;;<nl>Tekent een kader</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>r1, kol1, r2, kol2</name>
      ;;<description>
        ;;<fr>Coordonnées du cadre</fr>
        ;;<nl>Coördinaten vd kader</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kleur</name>
      ;;<description>
        ;;<fr>Couleur du cadre</fr>
        ;;<nl>Kleur vd kader</nl>
      ;;</description>
    ;;</parameter>
  ;;</parameters>
;;</heading>
;;<code>
;***************************************************************************
; kad (rij1,kolom1,rij2,kolom2,kleur,pagina)
; rijen van 0 tot 349
; kolom van 0 tot 639
;**************************************************************************

;*********************************************************
;                _kad
;*********************************************************

_kad                proc far    
    ARG  r1:word,kol1:word,r2:word,kol2:word,kleur:word
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     
    mov  ax,dgroup              
    mov  ds,ax                  
    mov  ax,kleur               
    mov  kleur1,al              
    mov  ax,kol2                
    mov  koloml1,ax             
    mov  ax,r2                  
    mov  rijl1,ax               
    mov  ax,kol1                
    mov  koloml,ax              
    mov  ax,r1                  
    mov  rijl,ax                

    call _init                  ;es=0a000h

    mov  ax,rijl                ;ax=rijl
    mov  bl,bytes_lijn          
    xor  bh,bh                  
;   mov  bx,80                  ;bx=80                         
    mul  bx                     ;dx:ax=ax*bx=rijl*80
                                ;ax wijst naar eerste byte in rij
                                ;ax=0 tot 27920
    mov  bx,koloml              ;bx=koloml  0 - 649
    mov  cl,3                   ;delen door 8
    shr  bx,cl                  ;bx=koloml/8      0-79
    add  ax,bx                  ;ax=ax+bx=rijl*80+koloml/8 max 27999
    mov  di,ax                  ;0 pointer naar eerste byte

    mov  ax,rijl1               ;349
    sub  ax,rijl                ;0
    mov  rijl,ax                ;aantal pixels horizontaal
                                ;bepaling ah
    mov  ax,koloml              ;0-639
    mov  bx,7                   
    and  ax,bx                  ;enkel drie laatste bits 0-7
    mov  cx,ax                  ;cl van 0 tot 7
    mov  al,128                 
    shr  al,cl                  ;zet op juiste pos
    mov  ah,al                  
    push ax                     ;houd bij

    mov  ax,koloml1             
    sub  ax,koloml              
    mov  koloml,ax              

    pop  ax                     
;   mov  ah,1                   
    mov  bl,kleur1              
    mov  cx,koloml              
    call _rec                   
    mov  cx,rijl                
    call _ond                   
    mov  cx,koloml              
    call _lin                   
    mov  cx,rijl                
    call _bov                   
    call _deinit                
    call _alles                 
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_kad                endp 
;;</code></function>

;;<function id="lijnhor">
;;<heading>
  ;;<description>
    ;;<fr>Dessine une ligne horizontale</fr>
    ;;<nl>Tekent horizontale lijn</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>r1</name>
      ;;<description>
        ;;<fr>Rangée de la ligne</fr>
        ;;<nl>Rij waar de lijn getrokken moet worden</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kol1</name>
      ;;<description>
        ;;<fr>Colonne à laquelle débute la ligne</fr>
        ;;<nl>Beginkolom vd lijn</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kol2</name>
      ;;<description>
        ;;<fr>Colonne à laquelle termine la ligne</fr>
        ;;<nl>Eindkolom vd lijn</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kleur</name>
      ;;<description>
        ;;<fr>Couleur de la ligne</fr>
        ;;<nl>Kleur vd lijn</nl>
      ;;</description>
    ;;</parameter>
  ;;</parameters>
;;</heading>
;;<code>
;****************************************************************************
; lijnhor (rij,kolom,einkolom,kleur)
;****************************************************************************

;*********************************************************
;                _lijnhor
;*********************************************************

_lijnhor            proc far    
    ARG  r1:word,kol1:word,kol2:word,kleur:word
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     
    mov  ax,dgroup              
    mov  ds,ax                  

    mov  ax,kleur               
    mov  kleur1,al              
    mov  ax,kol2                
    mov  koloml1,ax             
    mov  ax,kol1                
    mov  koloml,ax              
    mov  ax,r1                  
    mov  rijl,ax                
		
    call _init                  

    mov  ax,rijl                ;ax=rijl
    mov  bl,bytes_lijn          ;bx=100                        
    mov  bh,0                   
    mul  bx                     ;dx:ax=ax*bx=rijl*100=0 
    mov  bx,koloml              ;bx=koloml  0
    mov  cl,3                   ;delen door 8
    shr  bx,cl                  ;bx=koloml/8      0-79
    add  ax,bx                  ;0
    mov  di,ax                  ;0 pointer naar eerste byte

    mov  ax,koloml              ;0
    mov  bx,7                   
    and  ax,bx                  ;enkel drie laatste bits 0-7
    mov  cx,ax                  ;cl van 0 tot 7
    mov  al,128                 
    shr  al,cl                  ;10000000
    mov  ah,al                  
    push ax                     ;houd bij

    mov  ax,koloml1             
    sub  ax,koloml              
    mov  koloml,ax              ;799-0=799

    pop  ax                     
    mov  bl,kleur1              

    mov  cx,koloml              
    call _rec                   
    call _deinit                
    call _alles                 
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_lijnhor            endp 
;;</code></function>

;;<function id="lijnvert">
;;<heading>
  ;;<description>
    ;;<fr>Dessine une ligne verticale</fr>
    ;;<nl>Tekent verticale lijn</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>r1</name>
      ;;<description>
        ;;<fr>Rangée à laquelle débute la ligne</fr>
        ;;<nl>Beginrij vd lijn</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kol1</name>
      ;;<description>
        ;;<fr>Colonne de début</fr>
        ;;<nl>Kolom vd lijn</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>r2</name>
      ;;<description>
        ;;<fr>Rangée à laquelle termine la ligne</fr>
        ;;<nl>Eindrij vd lijn</nl>
      ;;</description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>kleur</name>
      ;;<description>
        ;;<fr>Couleur de la ligne</fr>
        ;;<nl>Kleur vd lijn</nl>
      ;;</description>
    ;;</parameter>
  ;;</parameters>
;;</heading>
;;<code>
;*****************************************************************************
;lijnvert (rij,kolom,eindrij,kleur,pagina)
; rij van 0 tot 349
; kolom van 0 tot 639
;****************************************************************************

;*********************************************************
;                _lijnvert
;*********************************************************

_lijnvert           proc far    
    ARG  r1:word,kol1:word,r2:word,kleur:word
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     

    mov  ax,dgroup              
    mov  ds,ax                  

    mov  kleur1,0               
    mov  ax,kleur               
    mov  kleur1,al              
    mov  ax,0                   
    mov  ax,r2                  
    mov  rijl1,ax               
    mov  ax,0                   
    mov  ax,kol1                
    mov  koloml,ax              
    mov  ax,0                   
    mov  ax,r1                  
    mov  rijl,ax                

    call _init                  
    mov  ax,rijl                ;ax=rijl
    mov  bl,bytes_lijn          ;bx=80
    mov  bh,0                   
    mul  bx                     ;dx:ax=ax*bx=rijl*80
                                ;ax wijst naar eerste byte in rij
                                ;ax=0 tot 27920
    mov  bx,koloml              ;bx=koloml  0 - 649
    mov  cl,3                   ;delen door 8
    shr  bx,cl                  ;bx=koloml/8      0-79
    add  ax,bx                  ;ax=ax+bx=rijl*80+koloml/8 max 27999
    mov  di,ax                  ;0 pointer naar eerste byte

    mov  ax,rijl1               ;349
    sub  ax,rijl                ;0
    mov  rijl,ax                ;aantal pixels horizontaal

                                ;bepaling ah

    mov  ax,koloml              ;0-639
    mov  bx,7                   
    and  ax,bx                  ;enkel drie laatste bits 0-7
    mov  cx,ax                  ;cl van 0 tot 7
    mov  al,128                 
    shr  al,cl                  ;zet op juiste pos
    mov  ah,al                  
    mov  bl,kleur1              
    mov  cx,rijl                
    call _ond                   
    call _deinit                
    call _alles                 
    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_lijnvert           endp 
;;</code></function>

;;<function id="alles">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _alles
;*********************************************************

_alles              proc near   
    mov  dx,3ceh                
    mov  al,8                   
    out  dx,al                  
    inc  dx                     
    mov  al,11111111b           ;te diplayen bitmap
    out  dx,al                  

    mov  dx,3c4h                
    mov  al,2                   
    out  dx,al                  
    inc  dx                     
    out  dx,al                  
    mov  al,0ffh                ;alle bitmappen bereikbaar
    out  dx,al                  
    ret                         
_alles              endp 
;;</code></function>

;;<function id="videom">
;;<heading>
  ;;<description>
    ;;<fr>Adapter le mode video/set de caractères</fr>
    ;;<nl>Videomode en/of karakterset veranderen</nl>
  ;;</description>
  ;;<parameters>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>mode</name>
      ;;<description><en>Video mode</en></description>
    ;;</parameter>
    ;;<parameter type="arg">
      ;;<datatype>word</datatype><name>karset</name>
      ;;<description><en>Character set</en></description>
      ;;<defaultValue/>
    ;;</parameter>
  ;;</parameters>
  ;;<comment>mode = 14 : CGA (640*200)
  ;;mode = 16 : EGA ( 640 * 350)
  ;;mode = 18 : VGA ( 640 * 480)
  ;;mode = 91 : 800 * 600
  ;;mode = 95 : SVGA ( 1024 * 768)
  ;;
  ;;karak =  8 : caracters 8 * 8 pixels
  ;;karak = 14 : caracters 14 * 8 pixels
  ;;karak = 16 : caracters 16 * 8 pixels
  ;;</comment>
;;</heading>
;;<code>
;*********************************************************
;                _videom
;*********************************************************

_videom             proc far    
    ARG  mode:word,karset:word  
    push bp                     
    mov  bp,sp                  
    push sp                     
    push ds                     
    push es                     
    push si                     
    push di                     

    mov  ax,dgroup              
    mov  ds,ax                  

    mov  ax,karset              
    mov  linen,al               
    mov  ax,mode                
    mov  tijdl,ax               

    cmp  al,vidmod              ;vergelijken met vorige waarde
    je   ty                     ;eindigen indien gelijk
    cmp  al,13                  
    jne  gk11                   
    jmp  ok1                    

gk11:
    cmp  al,14                  
    jne  gk12                   
    jmp  ok1                    

gk12:
    cmp  al,16                  
    jne  gk1                    
    jmp  ok1                    ;mode 16

gk1:
    cmp  al,18                  
    jne  gk2                    
    jmp  ok1                    ;mode 18

gk2:
    cmp  al,91                  
    jne  gk3                    
    jmp  ok1                    ;mode 91

gk3:
    cmp  al,95                  
    jne  gk4                    
    jmp  ok1                    ;mode 95

gk4:
    cmp  al,3                   
    jne  gk5                    
    jmp  ok1                    

gk5:
    cmp  al,7                   
    jne  kgedg                  

ok1:
    mov  vidmod,al              ;wegschrijven mode
    int  10h                    ;veranderen mode
    mov  al,vidmod              ;terug ophalen
    cmp  al,13                  
    jne  gko11                  
    mov  bytes_lijn,40          
    mov  totlijn,200            
    jmp  kgedg                  

gko11:
    cmp  al,14                  
    jne  gko12                  
    mov  bytes_lijn,80          
    mov  totlijn,200            

ty:
    jmp  kgedg                  

gko12:
    cmp  al,16                  
    jne  gko1                   
	mov  bytes_lijn,80
    mov  totlijn,350            
    jmp  kgedg                  

gko1:
    cmp  al,18                  
    jne  gko2                   
    mov  bytes_lijn,80          
    mov  totlijn,480            
    jmp  kgedg                  

gko2:
    cmp  al,91                  
    jne  gko3                   
    mov  bytes_lijn,100         
    mov  totlijn,600            
    jmp  kgedg                  

gko3:
    mov  bytes_lijn,128         
    mov  totlijn,768            

kgedg:
    mov  ah,11h                 
    mov  al,30h                 
    cmp  linen,8                
    jne  kflp1                  
    mov  bl,3                   
    mov  bh,3                   
    jmp  kgrt                   

kflp1:
    cmp  linen,14               
    jne  kflp2                  
    mov  bl,2                   
    mov  bh,2                   
    jmp  kgrt                   

kflp2:
    mov  bl,6                   
    mov  bh,6                   

kgrt:
    int  10h                    
    mov  gtabel,bp              ;offset
    mov  hulp,es                ;segment

    pop  di                     
    pop  si                     
    pop  es                     
    pop  ds                     
    pop  sp                     
    pop  bp                     
    ret                         
_videom             endp 
;;</code></function>

;;<function id="rec">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _rec
;*********************************************************

_rec                proc near   
    push bx                     ;bijhouden
    mov  bh,ah                  ;op nul gesteld tussen register
    xor  ah,ah                  

recl:
    shr  bh,1                   ;schuif een bit op
    jnc  recz                   
    rcr  bh,1                   
    call _pix                   ;schrijf vorige pixels weg
    xor  ah,ah                  
    mov  ah,128                 
    inc  di                     

recz:
    or   ah,bh                  ;maak klaar
	loop recl
    call _pix                   

ft:
    mov  ah,bh                  
    pop  bx                     
    ret                         
_rec                endp 
;;</code></function>

;;<function id="lin">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _lin
;*********************************************************

_lin                proc near   
    push bx                     
    mov  bh,ah                  
    xor  ah,ah                  

linl:
    shl  bh,1                   
    jnc  linz                   
    rcl  bh,1                   
    call _pix                   
    xor  ah,ah                  
    mov  ah,1                   
    dec  di                     

linz:
    or   ah,bh                  
    loop linl                   
    call _pix                   
    mov  ah,bh                  
    pop  bx                     
    ret                         
_lin                endp 
;;</code></function>

;;<function id="ond">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _ond
;*********************************************************

_ond                proc near   
    mov  dl,bytes_lijn          
    xor  dh,dh                  

ondl:
    add  di,dx                  
    push dx                     
    call _pix                   
    pop  dx                     
    loop ondl                   
    ret                         
_ond                endp 
;;</code></function>

;;<function id="bov">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _bov
;*********************************************************

_bov                proc near   
    mov  dl,bytes_lijn          
    xor  dh,dh                  

bovl:
    sub  di,dx                  
    push dx                     
    call _pix                   
    pop  dx                     
    loop bovl                   
    ret                         
_bov                endp 
;;</code></function>

;;<function id="linon">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _linon
;*********************************************************

_linon              proc near   
    mov  dl,bytes_lijn          
	xor  dh,dh

linonl:
    shl  ah,1                   
    jnc  linonz                 
    rcl  ah,1                   
    dec  di                     

linonz:
    add  di,dx                  
    push dx                     
    call _pix                   
    pop  dx                     
    loop linonl                 
    ret                         
_linon              endp 
;;</code></function>

;;<function id="onder">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _onder
;*********************************************************

_onder              proc near   
    mov  dl,bytes_lijn          
    xor  dh,dh                  

tr1:
    add  di,dx                  
    loop tr1                    
    ret                         
_onder              endp 
;;</code></function>

;;<function id="rechts">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _rechts
;*********************************************************

_rechts             proc near   

tr33:
    shr  ah,1                   
    jnc  er1                    
    rcr  ah,1                   
    inc  di                     

er1:
    loop tr33                   
    ret                         
_rechts             endp 
;;</code></function>

;;<function id="links">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _links
;*********************************************************

_links              proc near   

tr44:
    shl  ah,1                   
    jnc  er2                    
    rcl  ah,1                   
    dec  di                     

er2:
    loop tr44                   
    ret                         
_links              endp 
;;</code></function>

;;<function id="pix">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _pix
;*********************************************************

_pix                proc near
    mov  dx,03cfh               
    mov  al,ah                  
    out  dx,al                  ;hier weggeschreven adres masker
    mov  dx,03c4h               
    mov  al,02                  
    out  dx,al                  
    inc  dl                     
    mov  al,0ffh                ;overal 1 zetten
    out  dx,al                  
    mov  al,es:[di]             ;lezen
    xor  al,al                  
    mov  es:[di],al             ;0 wegschrijven
    dec  dl                     
    mov  al,02h                 
    out  dx,al                  
    inc  dl                     
    cmp  blank,0                
    jne  zwart                  
    mov  al,bl                  
    jmp  verder                 

zwart:
    mov  al,7                   

verder:
    out  dx,al                  ;wegschrijven kleur
    dec  dx                     
    mov  al,es:[di]             
    mov  al,0ffh                
    mov  es:[di],al             
    ret                         
_pix                endp 
;;</code></function>

;;<function id="init">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _init
;*********************************************************

_init               proc near
    mov  cx,0a000h              ;&&guy
    mov  es,cx                  ;es gebruikt voor videogeheugen

    mov  al,0bh                 
    mov  dx,03c8h               
    out  dx,al                  
    inc  dl                     
    in   al,dx                  
    dec  dl                     
    mov  ax,0101h               
    out  dx,al                  
    and  ah,0feh                
    out  dx,ax                  
    mov  dx,03c2h               
    in   al,dx                  
    mov  al,08fh                
    out  70h,al                 
    in   al,61h                 
    or   al,08h                 
    out  61h,al                 
    and  al,0f7h                
    out  61h,al                 
    mov  al,0fh                 
    out  70h,al                 
    xor  al,al                  
    mov  dx,03c8h               
    out  dx,al                  
    inc  dx                     
    in   al,dx                  
    and  al,0f7h                
    out  dx,al                  
    mov  dl,0ceh                
    mov  al,08                  
    out  dx,al                  
    ret                         
_init               endp 
;;</code></function>

;;<function id="deinit">
;;<heading>
  ;;<description/>
  ;;<parameters/>
;;</heading>
;;<code>
;*********************************************************
;                _deinit
;*********************************************************

_deinit             proc near   
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
_deinit             endp 
;;</code></function>

_prog ends
end

;*****************************************************************************
;;</body>
;;</source>
;;<!-- eof -->
