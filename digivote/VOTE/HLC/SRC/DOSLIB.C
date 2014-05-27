/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="doslib.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.7 $</revision>
  <archiveTimeStamp>$Date: 2005/09/12 12:27:04 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>doslib.c</fileName>
  <creationDate>02/05/2005</creationDate>
  <rights></rights>
  <author>Jurgen Rousseau</author>
  <description>
    <fr>Fonctions DOS</fr>
    <nl>DOS functies</nl></description>
  <history></history>
  <comment>Functionality       : Ter vervanging van DOS-commando's
                                 Het doel is om geheugen te winnen
                                 en zo weinig mogelijk DOS-calls
                                 in PROGRESS te moeten uitvoeren.
  </comment>
</comments>

<header>*/

/************************************************************/
/*                                                          */
/*  INCLUDE FILES                                           */
/*                                                          */
/************************************************************/
/*<includes>*/
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <ctype.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <bios.h>

#include "hlc.h"
#include "genlib.h"
/*</includes>*/

/************************************************************/
/*                                                          */
/*      CONSTANT DEFINITIONS                                */
/*                                                          */
/************************************************************/
#define BLOCK 5120

/************************************************************/
/*                                                          */
/*  GLOBAL VARIABLES FOR THIS FILE ONLY                     */
/*                                                          */
/************************************************************/
static char   gcLibName[] = "DosLib";
unsigned char data[BLOCK];
FILE          *streamin,
         *streamout;

/*</header>
<body>*/

/*<function id="C_ATTRIB">
<heading>
<description>
  <fr>Change les attributes d' un fichier</fr>
  <nl>Wijzigt de attributen van een bestand</nl></description>
<parameters>
  <parameter>
    <datatype>int</datatype>
      <name>argc</name>
      <description>
        <fr>Nombre de paramètres</fr>
        <nl>Aantal parameters</nl></description></parameter>
  <parameter>
    <datatype>char*</datatype>
      <name>argv[1]</name>
        <description>
          <fr>le nom du fichier dont on veut changer les attributes</fr>
          <nl>de naam van het bestand waarvan men de atributen wil veranderen</nl></description></parameter>
  <parameter>
    <datatype>char*</datatype>
      <name>argv[2]</name>
        <description>
          <fr>code pour les attributes
    0  visible + writable
    1  Read-Only
    2  Hidden
    3  Read-Only + Hidden</fr>
          <nl>code voor de attributen
    0  visible + writable
    1  Read-Only
    2  Hidden
    3  Read-Only + Hidden
</nl></description></parameter>
</parameters>
</heading>
<code>*/
/************************************************************/
/*                                                          */
/*  Functionname        : c_attrib                          */
/*                                                          */
/*  Functionality       : Wijzig het bestandsattribuut      */
/*                                                          */
/*                        Parameter 1 : bestandsnaam        */
/*                        Parameter 2 : te bekomen attrib   */
/*                           - 0 : Visible + Writable       */
/*                           - 1 : Read-Only                */
/*                           - 2 : Hidden                   */
/*                           - 3 : Read-Only + Hidden       */
/*                                                          */
/************************************************************/

c_attrib ( argc, argv)
    int argc;
    char *argv[];
{
    int param;
    unsigned attrib;
    struct find_t ffblk;

    /* Init error handling */
    InitFunction( gcLibName, "c_attrib", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    param = atoi( argv[2]);
    if( !_dos_findfirst( argv[1], _A_NORMAL|_A_RDONLY|_A_HIDDEN, &ffblk)) {
   attrib = ffblk.attrib;
   if( attrib | _A_HIDDEN) {
       if( param == 2 || param == 3)
      attrib |= _A_HIDDEN;
   }
   if( attrib & _A_HIDDEN) {
       if( param == 0 || param == 1)
      attrib &= ~_A_HIDDEN;
   }

   if( attrib | _A_RDONLY) {
       if( param == 1 || param == 3)
      attrib |= _A_RDONLY;
   }
   if( attrib & _A_RDONLY) {
       if( param == 0 || param == 2)
      attrib &= ~_A_RDONLY;
   }
   giStatus = !_dos_setfileattr( argv[1], attrib);
    }
    else {
   giStatus = 0;
    }
    updSharedVar();
    return( 0);
}
//</code></function>

/************************************************************/
/*                                                          */
/*  Functionname        : c_copy                            */
/*                                                          */
/*  Functionality       : copy file argv[1]                 */
/*                               (+ argv[2])                */
/*                        to file argv[last]                */
/*                                                          */
/************************************************************/

/*<function id="setverify">
<heading>
<description>
  <fr>Activation de la vérification sur la copie</fr>
  <nl>Inschakelen van controle op het kopiëren</nl></description>
<parameters>
  <parameter>
    <datatype>int</datatype>
      <name>argc</name>
      <description>
        <fr>Nombre de paramètres</fr>
        <nl>Aantal parameters</nl></description></parameter>
  <parameter>
    <datatype>int</datatype>
      <name>On</name>
        <description>
          <fr>TRUE : vérification sur la copie,
FALSE : pas de la vérification sur le copie</fr>
          <nl>TRUE : controle op het kopiëren
FALSE : geen controle op het kopiëren</nl></description></parameter></parameters>
</heading>
<code>*/
void setverify ( int On)
{
    union REGS Reg;

    Reg.h.ah = 0x2E;
    Reg.h.al = On;
    Reg.h.dl = 0x00;
    intdos( &Reg, &Reg);
}
/*</code></function>

<function id="C_COPY">
<heading>
<description>
  <fr>Prend un ou deux copies d' un fichier d' entrée</fr>
  <nl>Neemt een of twee kopieëen van een input-bestand</nl></description>
<parameters>
  <parameter>
    <datatype>int</datatype>
      <name>argc</name>
      <description>
        <fr>Nombre de paramètres</fr>
        <nl>Aantal parameters</nl></description></parameter>
  <parameter>
    <datatype>char*</datatype>
      <name>argv[1]</name>
        <description>
          <fr>fichier d' entrée</fr>
          <nl>input-bestand</nl></description></parameter>
  <parameter>
    <datatype>char*</datatype>
      <name>argv[2]</name>
        <description>
          <fr>fichier de sortie</fr>
          <nl>output-bestand</nl></description></parameter>
  <parameter>
    <datatype>char*</datatype>
      <name>argv[3]</name>
        <description>
          <fr>fichier de sortie</fr>
          <nl>output-bestand</nl></description></parameter>
</parameters></heading>
<code>*/

c_copy ( argc, argv)
    int argc;
    char *argv[];
{
    unsigned int  numread;
    long llength, i, j;

    /* Init error handling */
    InitFunction( gcLibName, "c_copy", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    /* Last  parameter = destination */
    if( ( streamout = fopen( argv[argc - 1], "wb")) == NULL) {
   // Unable to create argv[argc - 1] !
   giStatus = -10;
   goto error;
    }

    setverify( TRUE);
    for( i = 1 ; i < ( argc - 1) ; i++) {
   if( ( streamin = fopen( argv[i], "rb")) == NULL) {
       // Unable to open argv[i] !
       giStatus = -20;
       goto error;
   }
   llength  = filelength( fileno( streamin));
   while( llength > 0) {
       if( ( numread = fread( &data[0], 1, BLOCK, streamin)) <= 0) {
      // Unable to read from argv[i] !
      giStatus = -30;
      goto error;
       }
       llength -= numread;
       if( fwrite( &data[0], 1, numread , streamout) != numread) {
      // Unable to write to argv[argc - 1] !
      giStatus = -40;
      goto error;
       }
   } /* end while */
   fclose( streamin);
    } /* end for */
    fclose( streamout);

error:
    if( streamin)  fclose( streamin);
    if( streamout) fclose( streamout);
    if( giStatus) {
   giStatus -= 1000; /* customize error code */
   RaiseError( FALSE);
    }
    else {
   updSharedVar();
    }
    setverify( FALSE);
    return( 0);
}
/*</code></function>

<function id="C_DEL">
<heading>
<description/>
<parameters/>
</heading>
<code>*/
/************************************************************/
/*                                                          */
/*  Functionname        : c_del                             */
/*                                                          */
/*  Functionality       : Verwijderen van een bestand       */
/*                                                          */
/************************************************************/

c_del ( argc, argv)
    int argc;
    char *argv[];
{
    /* Init error handling */
    InitFunction( gcLibName, "c_del", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    giStatus = !access( argv[1], 00);
    if( giStatus == 1) {
   giStatus = !remove( argv[1]);
    }
    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="C_DIR">
<heading>
<description/>
<parameters/>
</heading>
<code>*/
/************************************************************/
/*                                                          */
/*  Functionname        : c_dir                             */
/*                                                          */
/*  Functionality       : Uitlijsten van dir, bestanden, .. */
/*                                                          */
/************************************************************/

c_dir ( argc, argv)
    int argc;
    char *argv[];
{
    struct find_t fileinfo;
    int retcode;
    char dirEntry[20];

    InitFunction( gcLibName, "c_dir", "fileok", TRUE);

    if( ( streamout = fopen( argv[2], "wt")) == NULL) {
   // Could not create outputfile argv[2] !
   giStatus = -10;
   goto error;
    }

    retcode = _dos_findfirst( argv[1],
               _A_ARCH | _A_HIDDEN | _A_NORMAL | _A_RDONLY,
               &fileinfo);
    while( !retcode) {
   sprintf( dirEntry, "%s\n", fileinfo.name);
   if( fwrite( dirEntry, 1, strlen(dirEntry), streamout) != strlen(dirEntry)) {
       // Could not write to argv[2] !
       giStatus = -20;
       goto error;
   }
   retcode = _dos_findnext( &fileinfo);
    }

error:
    if( streamout) fclose( streamout);
    if( giStatus) {
   giStatus -= 2000; /* customize error code */
   RaiseError( FALSE);
    }
    else {
   updSharedVar();
    }
    return( 0);
}
/*</code></function>

<function id="c_mkdir">
<heading>
<description/>
<parameters/>
</heading>
<code>*/
/************************************************************/
/*                                                          */
/*  Functionname        : c_mkdir                           */
/*                                                          */
/*  Functionality       : Create a directory                */
/*                                                          */
/************************************************************/

c_mkdir ( argc, argv)
    int argc;
    char *argv[];
{
    int param;
    unsigned attrib;
    struct find_t ffblk;

    /* Init error handling */
    InitFunction( gcLibName, "c_mkdir", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    giStatus = mkdir( argv[1]);

    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="c_fsize">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_fsize                           */
/*                                                          */
/*  Functionality       : Determine filesize                */
/*                                                          */
/************************************************************/

c_fsize ( argc, argv)
    int argc;
    char *argv[];
{
    struct find_t fileinfo;

    InitFunction( gcLibName, "c_fsize", "filesize", TRUE);
    if( giStatus == -1) return( 0);

    if( !_dos_findfirst( argv[1],
          _A_ARCH | _A_HIDDEN | _A_NORMAL | _A_RDONLY,
          &fileinfo)) {
   giStatus = fileinfo.size;
    }
    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="c_dfree">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_dfree                           */
/*                                                          */
/*  Functionality       : Determine free diskspace of       */
/*                        selected drive                    */
/*                                                          */
/************************************************************/

c_dfree ( argc, argv)
    int argc;
    char *argv[];
{
    struct diskfree_t diskinfo;

    InitFunction( gcLibName, "c_dfree", "diskfree", TRUE);
    if( giStatus == -1) return( 0);

    _dos_getdiskfree( toupper( argv[1][0]) - 'A' + 1, &diskinfo);
    giStatus = (long) diskinfo.avail_clusters      *
          (long) diskinfo.sectors_per_cluster *
          (long) diskinfo.bytes_per_sector;

    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="C_GETENV">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_getenv                          */
/*                                                          */
/*  Functionality       : get value of an ENVIRONMENT       */
/*                        variable                          */
/*                                                          */
/************************************************************/

c_getenv ( argc, argv)
    int argc;
    char *argv[];
{
    InitFunction( gcLibName, "c_getenv", argv[2], FALSE);

    if( getenv( argv[1]) == NULL) strcpy( gcStatus, "?");
    else                          strcpy( gcStatus, getenv( argv[1]));
    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="del_label">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_label                           */
/*                                                          */
/*  Functionality       : get disklabel in var dsklbl       */
/*                        or write it from 1st parameter    */
/*                                                          */
/************************************************************/

#define DRIVE         7
#define NAME          8
char fcb[128]      = {0xff, 0, 0, 0, 0, 0, 0x8, 0,
            '?','?','?','?','?','?','?','?','?','?','?'};
char creat_fcb[44] = {0xff, 0, 0, 0, 0, 0, 0x8, 0,
            ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

void del_label ( char *Drive)
{
    char findstring[7];
    struct find_t findstr;
    union REGS regs;

    strcpy(findstring, "?:\\*.*");
    *findstring=*Drive;
    if( !_dos_findfirst( findstring, _A_VOLID, &findstr)) {
   fcb[DRIVE] = *Drive - 'A' + 1;
   regs.x.ax = 0x1300;
   regs.x.dx = ( unsigned) fcb;
   intdos( &regs, &regs);
   } // end if.

} // end del_label.
/*</code></function>

<function id="make_label">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

int make_label ( char *Drive, char *Label)
{
    union REGS regs;
    int i,length;

    creat_fcb[DRIVE] = *Drive - 'A' + 1;
    length = strlen( Label);
    for( i = 0 ; i < length ; i++)
   creat_fcb[NAME+i] = Label[i];
    for(       ; i < 11     ; i++)
   creat_fcb[NAME+i] = ' ';

    regs.x.ax = 0x1600;                    // Create the file.
    regs.x.dx =( unsigned) creat_fcb;
    intdos( &regs, &regs);
    if( !regs.h.al) {
   regs.x.ax = 0x1000;                // Close the file.
   regs.x.dx = ( unsigned) creat_fcb;
   intdos( &regs, &regs);
   return( 1);
    } // end if.
    else
   return( 3);

} // end make_label.
/*</code></function>

<function id="C_LABEL">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

c_label ( argc, argv)
    int argc;
    char *argv[];
{
    struct find_t finfo;

    InitFunction( gcLibName, "c_label", "dsklbl", FALSE);

    if( argc == 3) {
      argv[1][0] = toupper( argv[1][0]);
   del_label(  argv[1]);
   make_label( argv[1], argv[2]);
    }
    if( !_dos_findfirst( "a:\*.*", (unsigned) _A_VOLID, &finfo)) {
   memcpy( gcStatus, 0x00, 12);
   strncpy( gcStatus, finfo.name, 8);
   if( strlen( finfo.name) > 9)
       strcat( &gcStatus[8], &finfo.name[9]);
    }
    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="c_tstdsk">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_tstdsk                          */
/*                                                          */
/*  Functionality       : test current state of disk(drive) */
/*                                                          */
/************************************************************/

c_tstdsk ( argc, argv)
    int argc;
    char *argv[];
{
    struct diskinfo_t disk_info;

    InitFunction( gcLibName, "c_tstdsk", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    disk_info.drive    = toupper( argv[1][0]) - 'A';
    disk_info.head     = 0;
    disk_info.track    = 1;
    disk_info.sector   = 1;
    disk_info.nsectors = 8;

    _bios_disk( _DISK_RESET,  &disk_info);
    _bios_disk( _DISK_VERIFY, &disk_info);
    // No disk inserted
    if( _bios_disk( _DISK_VERIFY, &disk_info) == 0x8000 )   giStatus = 0L;
    else {
   // if( status == 0x200 ) // Disk not formatted
   // Diskette is write protected
   if( _bios_disk( _DISK_WRITE, &disk_info) == 0x0300) giStatus = 2L;
   else                                                giStatus = 1L;
    }
    _bios_disk( _DISK_RESET, &disk_info);
    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="c_tstfn">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_tstfn                           */
/*                                                          */
/*  Functionality       : test existance of a file          */
/*                                                          */
/************************************************************/

c_tstfn ( argc, argv)
    int argc;
    char *argv[];
{
    InitFunction( gcLibName, "c_tstfn", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    giStatus = !access( argv[1], 00);
    if( giStatus == 0) {
   if( ( streamin = fopen( argv[1], "rb")) != NULL) {
       giStatus = 1;
       fclose( streamin);
   }
    }
    updSharedVar();
}
/*</code></function>

<function id="C_EXEC">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_exec                            */
/*                                                          */
/*  Functionality       : Uitvoeren van een DOS-commando    */
/*                        (suppress DOS messages)           */
/*                                                          */
/************************************************************/

void ( _interrupt _far *oldinterr)( void);
static void _far Handler ( unsigned deverr,
           unsigned doserr,
           unsigned _far *hdr )
{
    _hardresume( _HARDERR_ABORT);
}

c_exec ( argc, argv)
    int argc;
    char *argv[];
{
    int Result;
    char StdIn[20];

    InitFunction( gcLibName, "c_exec", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    oldinterr = _dos_getvect(36);

    _harderr( Handler);

    if( getenv( "RAM") == NULL) sprintf( StdIn, "C:\\STDIN");
    else                        sprintf( StdIn, "%s:\\STDIN", getenv( "RAM"));

    /* If STDIN does not exist, take 'nul' as stdin */
    if( access( StdIn, 00))     sprintf( StdIn, "nul");

    freopen( StdIn, "rt", stdin );
    freopen( "nul", "wt", stdout);
    freopen( "nul", "wt", stderr);

    Result = spawnvp( P_WAIT, argv[1], &argv[1]);

    freopen( "con", "rt", stdin );
    freopen( "con", "wt", stdout);
    freopen( "con", "wt", stderr);

    if( strcmp( StdIn, "nul"))  remove( StdIn);

    _dos_setvect( 36, oldinterr);

    if( Result) giStatus = 1;
    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="c_touch">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

/************************************************************/
/*                                                          */
/*  Functionname        : c_touch                           */
/*                                                          */
/*  Functionality       : Forceren datum/tijd van bestanden */
/*                                                          */
/************************************************************/

c_touch ( argc, argv)
    int argc;
    char *argv[];
{
    unsigned int a_day, a_month, a_year, file_date,
                 a_hour, a_minute, a_second, file_time;
    int fHandle;

    InitFunction( gcLibName, "c_touch", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    a_day     = atoi( argv[2]);
    a_month   = atoi( argv[3]);
    a_year    = atoi( argv[4]) - 1980;
    file_date =   a_day
              + ( a_month  <<  5)
              + ( a_year   <<  9);
    a_hour    = atoi( argv[5]);
    a_minute  = atoi( argv[6]);
    a_second  = atoi( argv[7]);
    file_time =   a_second
              + ( a_minute <<  5)
              + ( a_hour   << 11);
    if( !_dos_open( argv[1], O_RDWR, &fHandle)) {
        if( !_dos_setftime( fHandle, file_date, file_time))
            giStatus = 1;
        _dos_close( fHandle);
    }
    updSharedVar();
    return( 0);
}
/*</code></function>

</body>
</source>*/
