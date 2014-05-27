/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="test.cpp"><!-- use lower case for name -->
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.3 $</revision>
  <archiveTimeStamp>$Date: 2007/03/20 14:29:47 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <description>
    <fr>Programme pour tester les composants du système</fr>
    <nl>Programma om de systeemcomponenten te testen</nl>
  </description>
</comments>*/
//
//============================================================= BOF HEADER ===
//<header>
//
//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h> // _dos_findfirst
#include <dir.h> // _dos_findfirst

#include "..\diagnost\general.h"
#include "..\diagnost\setup.h"
//</includes>
//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//
// TRAP DOS ERRORS
//////////////////
#define IGNORE  0
#define ABORT   2
//</defines>
//
//--------------------------------------------------------------- TYPEDEFS ---
//
enum PARAM           { SYNTAX, PXL, RAM, RAMDRV, DRV, PRT, PRTMSG, MEM, DSK};
enum RET_CODE        { SUCCEEDED, ABORTED, RETRY};
enum INTEGRITY_STATE { CONSISTENT, CORRUPTED};
enum LIGHTPENSTATUS  { CONNECTED, NOT_CONNECTED, WRONG_VERSION};
enum FLOPPY_STATE    { WRITE_PROTECTED, _DUMMY_, DRIVE_NOT_READY, DATA_ERROR_CRC};
//
//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
PARAM           parameter;
INTEGRITY_STATE Flag_Integrity;
char TestFileName[] = "TEST.WRP";
int retval = 0;
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="ShowSyntax">
<heading>
  <description>
    <fr>Affichage du syntax des paramètres</fr>
    <nl>Tonen van de syntax van de parameters</nl>
  </description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void ShowSyntax ( void)
// ---------------------------------------------------------------------------
{
    cout << "\n"
     << "TEST Utility for DIGIVOTE\n"
     << "=========================\n"
     << "Syntax : TEST [-option(s)]\n"
     << "******\n"
     << "> TEST                  : Show this information\n"
     << ">      -ram             : Test RAMdrive (BATCH)\n"
     << ">      -ramdrive        : Test RAMdrive (PROGRESS)\n"
     << ">      -drv filename    : Test Drives using [filename] to check\n"
     << ">      -mem             : Test Extended Memory\n"
     << ">      -dsk drivename   : Test Write protected\n"
     << "\n";
}
//</code></function>

//===========================================================================
//===================== T R A P   F U N C T I O N S =========================
//===========================================================================

#pragma warn -par
/*<function id="handler">
<heading>
  <description><fr>Hardware error handler</fr><nl>Hardware error handler</nl></description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int handler (int errval,int ax,int bp,int si)
// ---------------------------------------------------------------------------
{
    /* if this is not a disk error then it was */
    /* another device having trouble           */
    if (ax < 0)
    {
       /* report the error */
       cout << "Device Error - Aborting ...";
       /* and return to the program directly requesting abort */
       hardretn( ABORT);
    }
    /* otherwise it was a disk error */
    hardresume( IGNORE);
    cout << "Unknown Error Handler - Aborting ...";
    return( ABORT);
}
//</code></function>

#pragma argsused
/*<function id="Reader_Error_Handler">
<heading>
  <description>
     <fr>La fonction a executer en cas d' erreur de lecteur de carte</fr>
     <nl>De functie die uitgevoerd wordt in geval van een kaartlezerfout</nl>
  </description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Reader_Error_Handler ( int errval, int ax, int bp, int si)
// ---------------------------------------------------------------------------
{
   unsigned di;
   int errorno;

   di= _DI;

   if (ax < 0)
       hardretn( ABORT);
 /* otherwise it was a disk error */
   errorno = di & 0x00FF;

   if( errorno == 0)
   {
       retval = 1;
       hardretn( ABORT);
   }

   return( ABORT);
}
//</code></function>

/*<function id="c_break">
<heading>
  <description>
    <fr>Captation du bouton 'Control break'</fr>
    <nl>Opvangen van de 'Control break'-toets</nl>
  </description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int c_break ( void)
// ---------------------------------------------------------------------------
{
   return( 1); // continue the program
}
//</code></function>

//===========================================================================
//============================ R A M D R I V E ==============================
//===========================================================================
// Return values :
// =============
// 26-3 : RAMdrive found (Z-C)
//    0 : No RAMdrive found

#pragma warn +par
/*<function id="Check_RAMdrive">
<heading>
  <description><fr>Test du ramdrive</fr><nl>Test van de ramdrive</nl></description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Check_RAMdrive ( void)
// ---------------------------------------------------------------------------
{
    int retcode=0, Drive;
    struct find_t Control_Block;
    char DiskDrive[1], *PathName;
    BOOLEAN CriticalErrors=FALSE;

    harderr( handler);
    for( Drive='Z' ; Drive >= 'C' ; Drive--)
    {
        sprintf( PathName, "%c:*.*", Drive);
    if( _dos_findfirst( PathName, _A_VOLID, &Control_Block)==0)
            if( strcmp( Control_Block.name, "MS-RAMDR.IVE")==0)
        {
                ofstream RamFile;
                DiskDrive[0]=Drive;
                DiskDrive[1]=0;
                switch( parameter)
                {
            case RAM    :
                        RamFile.open( "\\SETRAM.BAT");
                        if( !RamFile)        CriticalErrors=TRUE;
                        if( !CriticalErrors)
                            RamFile << "@Echo OFF\n"
                    << "Set RAM=" << DiskDrive;
                        break;

            case RAMDRV :
                        RamFile.open( "\\VOTE\\FILES\\RAMDRIVE.LSI");
                        if( !RamFile)        CriticalErrors=TRUE;
                        if( !CriticalErrors)
                RamFile << DiskDrive << ":\\";
                        break;
                }
                if( !CriticalErrors) retcode=(Drive-64);
        }
    }
    return( retcode);
}
//</code></function>

//===========================================================================
//============================== D R I V E S ================================
//===========================================================================
// Return values :
// =============
// 30 : PRP Iomega ZIP  C: and D: are Iomega ZIP
// 21 : HYBRIDE         C: is SyQuest and D: is Iomega ZIP
// 20 :                 Only D: is Iomega ZIP
// 12 :                 C: is Iomega ZIP and D: is SyQuest (theoretical)
// 10 : TOT Iomega ZIP  Only C: is Iomega ZIP
//  3 : PRP SyQuest     C: and D: are SyQuest
//  2 :                 Only D: is SyQuest
//  1 : TOT SyQuest     Only C: is SyQuest
//  0 : No tests have been executed (ERROR ?)

/*<function id="StringPartOf">
<heading>
  <description>
    <fr>Fonction qui teste si une chaîne de charactère fait parti d' une autre chaîne</fr>
    <nl>Functie die test of een string een onderdeel is van een andere string</nl></description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
BOOLEAN StringPartOf ( char *Source, char *Search)
// ---------------------------------------------------------------------------
{
    char *TempString;
    int i;

    for( i=0 ; i < strlen( Source) ; i++)
    {
        TempString=&Source[i];
    if( strncmp( TempString, Search, strlen(Search))==0)
            return( TRUE);
    }
    return( FALSE);
}
//</code></function>

/*<function id="Check_Drives">
<heading>
  <description><fr>Test des drives</fr><nl>Test van de drives</nl></description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Check_Drives ( int argc, char *OutputFile)
// ---------------------------------------------------------------------------
{
    int retcode=0;

    BOOLEAN CriticalErrors, DriveC, DriveD, DriveE, DriveF;
    BIOS_TEST bios;
    long t, c, h, s;
    char Drive;
    char Buffer[100], Message[100];

    if( argc==3)
    {
    DriveC=FALSE;
    DriveD=FALSE;
    DriveE=FALSE;
        DriveF=FALSE;
        //SyQuest
        bios=Bios_Test();
    switch( bios)
    {
            case AMI_1 :
            case AMI_2 :
        case AMI_3 :
                //Checking PM
        c=(GetByteFromCMOS(0x1C) << 8)+GetByteFromCMOS(0x1B);
                h=(GetByteFromCMOS(0x1E) << 8)+GetByteFromCMOS(0x1D);
        s=GetByteFromCMOS(0x23);
                t=GetByteFromCMOS(0x19);
                if( t>0 && c==841 && h==16 && s==16)
            if( retcode!=255)
                        if     ( !DriveC) { DriveC=TRUE; retcode+=1; }
                        else if( !DriveD) { DriveD=TRUE; retcode+=2; }
                        else if( !DriveE) { DriveE=TRUE; retcode+=4; }
                        else if( !DriveF) { DriveF=TRUE; retcode+=8; }
            else                             retcode=255;

                //Checking PS
                c=(GetByteFromCMOS(0x25) << 8)+GetByteFromCMOS(0x24);
                h=(GetByteFromCMOS(0x27) << 8)+GetByteFromCMOS(0x26);
        s=GetByteFromCMOS(0x2C);
        t=GetByteFromCMOS(0x1A);
        if( t>0 && c==841 && h==16 && s==16)
            if( retcode!=255)
                        if     ( !DriveC) { DriveC=TRUE; retcode+=1; }
            else if( !DriveD) { DriveD=TRUE; retcode+=2; }
                        else if( !DriveE) { DriveE=TRUE; retcode+=4; }
            else if( !DriveF) { DriveF=TRUE; retcode+=8; }
                        else                             retcode=255;
                break;

        case AWARD_W6178MS:
        case AWARD_P5SJ_B :
//        case AWARD_20d :
                //Checking PM
        c=(GetByteFromCMOS(0x27) << 8)+GetByteFromCMOS(0x26);
                h=GetByteFromCMOS(0x28);
        s=GetByteFromCMOS(0x2D);
                t=GetByteFromCMOS(0x19);
                if( t>0 && c==841 && h==16 && s==16)
            if( retcode!=255)
                        if     ( !DriveC) { DriveC=TRUE; retcode+=1; }
                        else if( !DriveD) { DriveD=TRUE; retcode+=2; }
                        else if( !DriveE) { DriveE=TRUE; retcode+=4; }
                        else if( !DriveF) { DriveF=TRUE; retcode+=8; }
            else                             retcode=255;

                //Checking PS
                c=(GetByteFromCMOS(0x1F) << 8)+GetByteFromCMOS(0x1E);
                h=GetByteFromCMOS(0x20);
        s=GetByteFromCMOS(0x25);
        t=GetByteFromCMOS(0x1A);
        if( t>0 && c==841 && h==16 && s==16)
            if( retcode!=255)
                        if     ( !DriveC) { DriveC=TRUE; retcode+=1; }
                        else if( !DriveD) { DriveD=TRUE; retcode+=2; }
            else if( !DriveE) { DriveE=TRUE; retcode+=4; }
            else if( !DriveF) { DriveF=TRUE; retcode+=8; }
            else                             retcode=255;

        //Checking SM
        c=(GetByteFromCMOS(0x72) << 8)+GetByteFromCMOS(0x71);
        h=GetByteFromCMOS(0x73);
        s=GetByteFromCMOS(0x78);
        t=GetByteFromCMOS(0x67);
        if( t>0 && c==841 && h==16 && s==16)
            if( retcode!=255)
            if     ( !DriveC) { DriveC=TRUE; retcode+=1; }
            else if( !DriveD) { DriveD=TRUE; retcode+=2; }
            else if( !DriveE) { DriveE=TRUE; retcode+=4; }
            else if( !DriveF) { DriveF=TRUE; retcode+=8; }
            else                             retcode=255;

        //Checking SS
        c=(GetByteFromCMOS(0x69) << 8)+GetByteFromCMOS(0x68);
        h=GetByteFromCMOS(0x6A);
        s=GetByteFromCMOS(0x6F);
        t=GetByteFromCMOS(0x70);
        if( t>0 && c==841 && h==16 && s==16)
            if( retcode!=255)
            if     ( !DriveC) { DriveC=TRUE; retcode+=1; }
            else if( !DriveD) { DriveD=TRUE; retcode+=2; }
            else if( !DriveE) { DriveE=TRUE; retcode+=4; }
            else if( !DriveF) { DriveF=TRUE; retcode+=8; }
            else                             retcode=255;
        break;
    }

    //Iomega ZIP
    CriticalErrors=FALSE;
    ifstream DrvFile( OutputFile);
    if( !DrvFile)
        CriticalErrors=TRUE;
    while(  DrvFile       &&
           !DrvFile.eof() &&
           !CriticalErrors  )
    {
        DrvFile.getline( Buffer, sizeof( Buffer));
        if( !DrvFile && !DrvFile.eof())
        CriticalErrors=TRUE;
        else
        for( Drive=0 ; Drive<=1 ; Drive++)
        {
            sprintf( Message, "SCSI ID %d, LUN 0 - IOMEGA  ZIP 100", Drive);
            if( StringPartOf( Buffer, Message))
            if( retcode!=255)
                if     ( !DriveC) { DriveC=TRUE; retcode+=10; }
                else if( !DriveD) { DriveD=TRUE; retcode+=20; }
                else if( !DriveE) { DriveE=TRUE; retcode+=40; }
                else if( !DriveF) { DriveF=TRUE; retcode+=80; }
                else                             retcode=255;
        }
    }
    if( CriticalErrors)
        retcode=0;
    }
    return( retcode);
}
//</code></function>

#pragma warm +par
/*<function id="Check_WriteProtected">
<heading>
  <description>
    <fr>Teste si la disquette est protégée en écriture</fr>
    <nl>Test of de diskette beschermd is tegen schrijven</nl></description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Check_WriteProtected ( char *Drive )
// ---------------------------------------------------------------------------
{
    FILE *fp;
    char FileName[12];

    retval = 0;

    harderr( Reader_Error_Handler);

    sprintf( FileName, "%s:\\%s", Drive, TestFileName);
    fp = fopen( FileName , "w" );
    fclose( fp);
    return( retval );
}
//</code></function>

//===========================================================================
//============================== M E M O R Y ================================
//===========================================================================
// Return values :
// =============
// ?? : Size of Extended Memory in MB

/*<function id="Check_ExtMemory">
<heading>
  <description>
    <fr>Retourne la taille de la mémoire étendue en MB</fr>
    <nl>Keert de geheugencapaciteit van het extended memory in MB terug</nl></description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Check_ExtMemory ()
{
    unsigned long ExtMem=GetExtMemorySize();
    return( ExtMem/1024);
}
//</code></function>

/*<function id="Test" type="main">
<heading>
  <description><fr>Programme principal</fr><nl>Hoofdprogramma</nl></description>
  <parameters></parameters>
</heading>
<code>*/
//===========================================================================
//================================ M A I N ==================================
//===========================================================================

// ---------------------------------------------------------------------------
int main (int argc, char *argv[])
// ---------------------------------------------------------------------------
{
    int retcode=0;

    switch( argc)
    {
    case 1  :
        parameter=SYNTAX;
        break;

    case 2  :
        if( strcmp( argv[1], "-ram" ) == 0 )
        parameter =             RAM;

        else if( strcmp( argv[1], "-ramdrive" ) == 0 )
        parameter =             RAMDRV;

        else if( strcmp( argv[1], "-mem" ) == 0 )
        parameter =             MEM;

        else
        parameter =             SYNTAX;

        break;

    case 3  :
        if( strcmp( argv[1], "-drv" ) == 0 )
        parameter =             DRV;
        else if( strcmp( argv[1], "-dsk" ) == 0 )
        parameter =             DSK;

        else
        parameter=SYNTAX;
        break;

    default :
        parameter=SYNTAX;
        break;
    }

    switch( parameter)
    {

    case RAM    :
    case RAMDRV :
        retcode=Check_RAMdrive();
        break;

    case DRV    :
        retcode=Check_Drives( argc, argv[2] );
        break;

    case MEM    :
        retcode=Check_ExtMemory();
        break;

    case DSK    :
        retcode = Check_WriteProtected( argv[2] );
        break;

    case SYNTAX :
    default     :
        ShowSyntax();
        break;
    }
    return( retcode);
}
//</code></function>
//=============================================================== EOF BODY ===
/*</body>
</source>
<!-- eof -->*/
