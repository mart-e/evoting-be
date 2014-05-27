; $Log: ABSDISK.ASM,v $
; Revision 1.1  2005/02/07 11:32:19  avanderhoudelingen
; Initial version.
;
;  
;     Rev 9.7   Jun 26 2000 15:57:04   JRU
;   
;  
;     Rev 9.6   May 30 2000 15:47:28   mth
;   
;  
;     Rev 9.5   Apr 28 2000 15:26:08   mth
;   
;  
;     Rev 9.4   Mar 15 2000 11:12:02   mth
;   
;  
;     Rev 9.3   Jan 27 2000 16:40:02   jru
;   
;  
;     Rev 9.2   Oct 20 1999 11:16:38   jru
;   
;  
;     Rev 9.1   Apr 07 1999 15:36:34   jru
;   
;  
;     Rev 1.3   Feb 18 1999 16:51:38   jru
;   
;  
;     Rev 1.2   Dec 22 1998 16:06:00   jru
;  Synchronize with my sources/files (JRU)
;  
;     Rev 1.1   Dec 22 1998 13:20:02   tde
;  - Adaptation of DIAGFR, DIAGNL and DIAGEN
;  for the production tests
;  - Modification for DIGIVOTE II
;  
;     Rev 1.0   Nov 06 1998 12:39:34   ebg
;  Checked in from initial workfile by PVCS Version Manager Project Assistant.
; 
;    Rev 52.0   08 Jul 1994 16:16:32   superuser
; Version of European elections
; 
;    Rev 1.0   07 Jul 1994 16:22:38   superuser
; Version of European elections
;/*H*/
;//=============================================================================
;/*
;
;   File        : ABSDISK.ASM
;   Creation    : 01/04/93
;
;   Project     : Bull DIGIVOTE
;   Author      : Willy Born, B.S.S.
;
;   Description : Absolute disk access functions
;
;*/
;//=============================================================================
;/*h*/
;/*R*/
;//=============================================================================
;/*
;
;   Revision History
;   ################
;
;   01/06/93: revision history starting date
;
;*/
;//=============================================================================
;/*r*/
;/*#*/

.MODEL large,C

PUBLIC OwnAbsRead
PUBLIC OwnAbsWrite

DeviceHeaderLayout STRUC
  Next       DD 0FFFFh
  DevAttr    DW 0
  Strategy   DW 0
  Interrupt  DW 0
  DeviceName DB 'Unknown '
DeviceHeaderLayout ENDS

AbsDiskData segment word public 'DATA'
  DeviceHeader  DeviceHeaderLayout {}
  bp_value      DW                 ?
AbsDiskData ENDS

;//============================================================================

absread MACRO
  mov  al,BYTE PTR Drive
  mov  cx,NrSectors
  mov  dx,StartSector
  lds  bx,Buffer
  clc
  int  25h
  pop  ds
ENDM

;//============================================================================

abswrite MACRO
  mov  al,BYTE PTR Drive
  mov  cx,NrSectors
  mov  dx,StartSector
  lds  bx,Buffer
  clc
  int  26h
  pop  ds
ENDM

AbsDisk SEGMENT byte public 'CODE'
    ASSUME cs:AbsDisk

;//============================================================================

OwnAbsRead PROC
    ARG Drive:WORD, NrSectors:WORD, StartSector:WORD, Buffer:FAR PTR

absreadRetry:
    pushf
    lea  bx,absreadErrorReturned
    push cs
    push bx

    push es
    push ds
    push bp
    push di
    push si
    push dx
    push cx
    push bx
    push ax
    absread
    jc   absreadErrorPreparation
    pop  ax
    pop  bx
    pop  cx
    pop  dx
    pop  si
    pop  di
    pop  bp
    pop  ds
    pop  es
    add  sp,6
    xor  ax,ax
    ret

absreadErrorPreparation:
    ;
    ; Preparation for INT24
    ;;;;;;;;;;;;;;;;;;;;;;;
    mov  di,ax
    mov  al,BYTE PTR Drive
    mov  ah,00011110b
    mov  bp,SEG DeviceHeader
    mov  si,OFFSET DeviceHeader
    int  24h
    pop  bx
    pop  bx
    pop  cx
    pop  dx
    pop  si
    pop  di
    pop  bp
    pop  ds
    pop  es
    add  sp,6

absreadErrorReturned:
    cmp  al,1                  ;Retry
    je   absreadRetry
    cmp  al,2                  ;Abort
    je   absreadAborted
    mov  ax,1                  ;Cancel or Ignore pressed
    ret

absreadAborted:
    mov  ax,4CFFh
    int  21h

OwnAbsRead ENDP

;//============================================================================

OwnAbsWrite PROC
    ARG Drive:WORD, NrSectors:WORD, StartSector:WORD, Buffer:FAR PTR

abswriteRetry:
    pushf
    lea  bx,abswriteErrorReturned
    push cs
    push bx

    push es
    push ds
    push bp
    push di
    push si
    push dx
    push cx
    push bx
    push ax
    abswrite
    jc   abswriteErrorPreparation
    pop  bx
    pop  bx
    pop  cx
    pop  dx
    pop  si
    pop  di
    pop  bp
    pop  ds
    pop  es
    add  sp,6
    xor  ax,ax
    ret

abswriteErrorPreparation:
    ;
    ; Preparation for INT24
    ;;;;;;;;;;;;;;;;;;;;;;;
    mov  di,ax
    mov  al,BYTE PTR Drive
    mov  ah,00011111b
    mov  bp,SEG DeviceHeader
    mov  si,OFFSET DeviceHeader
    int  24h
    pop  bx
    pop  bx
    pop  cx
    pop  dx
    pop  si
    pop  di
    pop  bp
    pop  ds
    pop  es
    add  sp,6

abswriteErrorReturned:
    cmp  al,1                  ;Retry
    je   abswriteRetry
    cmp  al,2                  ;Abort
    je   abswriteAborted
    mov  ax,1                  ;Cancel or Ignored pressed
    ret

abswriteAborted:
    mov  ax,4CFFh
    int  21h

OwnAbsWrite ENDP

AbsDisk ENDS
END

;//=== ABSDISK.ASM ============================================================
