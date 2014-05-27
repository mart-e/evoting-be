/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="diskima.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.2 $</revision>
  <archiveTimeStamp>$Date: 2005/09/12 12:27:04 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>diskima.c</fileName>
  <creationDate/>
  <rights></rights>
  <author>JRU</author>
  <description/>
  <history/>
  <comment/>
</comments>
<header>*/

/*<includes>*/
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hlc.h"
#include "genlib.h"
/*</includes>*/

#define SECTOR_SIZE 512

static char gcLibName[]  = "DiskIma";

typedef union {
    unsigned int a_int;
    char         a_char[2];
} fDateTime;
fDateTime fileDate, fileTime;
unsigned int a_day, a_month, a_year, a_hour, a_minute, a_second;

typedef union {
  char SN[4];
  struct {
      union {
          struct {
              unsigned char Minutes;
              unsigned char Hours;
          } Hours_Minutes;
          int Year;
      } part2;
      union {
          struct {
              unsigned char Day_Hundreds;
              unsigned char Month_Sec;
          } Date_Time;
          int Build_DiskID;
      } part1;
  } calc;
} fVolID;
fVolID fileVolID;

union {
    char rawSector[SECTOR_SIZE];

    struct {
        char BS_jmpBoot[3];
        char BS_OEMName[8];
        char BPB_BytsPerSec[2];
        char BPB_SecPerClus[1];
        char BPB_RsvdSecCnt[2];
        char BPB_NumFATs[1];
        char BPB_RootEntCnt[2];
        char BPB_TotSec16[2];
        char BPB_Media[1];
        char BPB_FATSz16[2];
        char BPB_SecPerTrck[2];
        char BPB_NumHeads[2];
        char BPB_HiddSec[4];
        char BPB_TotSec32[4];
        char BS_DrvNum[1];
        char BS_Reserved1[1];
        char BS_BootSig[1];
        char BS_VolID[4];
        char BS_VolLab[11];
        char BS_FilSysType[8];
        char dummy[450];
    } Sector0;

    struct {
        char DIR_Name[11];
        char DIR_Attr[1];
        char DIR_NTRes[1];
        char DIR_CrtTimeTenth[1];
        char DIR_CrtTime[2];
        char DIR_CrtDate[2];
        char DIR_LstAccDate[2];
        char DIR_FstCluHI[2];
        char DIR_WrTime[2];
        char DIR_WrDate[2];
        char DIR_FstClusLO[2];
        char DIR_FileSize[4];
    } FAT_Entry[15];
} Sector;

FILE *source, *destination;

/*</header>
<body>*/

/*<function id="conv1">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
unsigned int conv1 ( char *cValue)
{
   union {
        unsigned char a_int;
        char          a_char[1];
    } conv;

    memcpy( conv.a_char, cValue, sizeof( conv.a_char));
    return( ( unsigned int) conv.a_int);
}
/*</code></function>

<function id="conv2">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
unsigned int conv2 ( char *cValue)
{
    union {
        unsigned int  a_int;
        char          a_char[2];
   } conv;

    memcpy( conv.a_char, cValue, sizeof( conv.a_char));
    return( conv.a_int);
}
/*</code></function>

<function id="conv4">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
unsigned long conv4 ( char *cValue)
{
   union {
        unsigned long a_int;
        char          a_char[4];
    } conv;

    memcpy( conv.a_char, cValue, sizeof( conv.a_char));
    return( conv.a_int);
}
/*</code></function>

<function id="mk_image">
<heading>
  <description><en>Reproduce a customized Disk Image File.</en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
int
mk_image ( argc, argv)
     int argc;
     char *argv[];
{
    char new_disk_label[15];
    unsigned int FirstRootDirSecNum, currSector, done, i;

    /* Init error handling */
    InitFunction( gcLibName, "mk_image", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    // Open Source Image File for reading ...
    if( ( source = fopen( argv[1], "rb")) == NULL) {
        sprintf( gcMessage, "Unable to open '%s'", argv[1]);
        giStatus = -10;
        goto error;
    }
    // Create Destination Image File for writing ...
    if( ( destination = fopen( argv[2], "wb")) == NULL) {
        sprintf( gcMessage, "Unable to create '%s'", argv[2]);
        giStatus = -20;
        goto error;
    }
    // Read Sector 0 - The Boot Sector -
    if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
        sprintf( gcMessage, "Unable to read 'Sector 0' from '%s'", argv[1]);
        giStatus = -30;
        goto error;
    }

    // Check SECTOR_SIZE ...
    if( conv2( Sector.Sector0.BPB_BytsPerSec) != SECTOR_SIZE) {
        sprintf( gcMessage, "SECTOR_SIZE %d <> %d", conv2( Sector.Sector0.BPB_BytsPerSec),
                                                    SECTOR_SIZE);
        giStatus = -40;
        goto error;
    }

    // Replace Volume Label ... ( Sector 0 )
    memset( new_disk_label, ' ', sizeof( new_disk_label));
    memcpy( new_disk_label, argv[3], strlen( argv[3]));
    memcpy( Sector.Sector0.BS_VolLab, new_disk_label,
            sizeof( Sector.Sector0.BS_VolLab));
    // Force Volume Serial Number ...
    fileVolID.calc.part1.Date_Time.Month_Sec    = atoi( argv[8]);
    fileVolID.calc.part1.Date_Time.Day_Hundreds = atoi( argv[7]);
    fileVolID.calc.part1.Build_DiskID          += ( ( atoi( argv[6]) * 10000)
                                                  + atoi( argv[10]));
    fileVolID.calc.part2.Hours_Minutes.Hours    = atoi( argv[4]);
    fileVolID.calc.part2.Hours_Minutes.Minutes  = atoi( argv[5]);
    fileVolID.calc.part2.Year                  += atoi( argv[9]);

    memcpy( Sector.Sector0.BS_VolID, fileVolID.SN,
            sizeof( Sector.Sector0.BS_VolID));

    FirstRootDirSecNum =    conv2( Sector.Sector0.BPB_RsvdSecCnt)
                       + (  conv1( Sector.Sector0.BPB_NumFATs)
                          * conv2( Sector.Sector0.BPB_FATSz16));

    // Write the new Sector 0 to the destination file ...
    if( fwrite( Sector.rawSector, SECTOR_SIZE, 1, destination) != 1) {
        giStatus = -50;
        goto error;
    }

    for( currSector = 1 ; currSector < FirstRootDirSecNum ; currSector++) {
        if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
            giStatus = -60;
            goto error;
        }
        if( fwrite( Sector.rawSector, SECTOR_SIZE, 1, destination) != 1) {
            giStatus = -70;
            goto error;
        }
    }
    done = 0;
    while( !done) {
        if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
            giStatus = -80;
            goto error;
        }

        currSector += 1;
        for( i = 0 ; i < 16 ; i++) {
            if( strlen( Sector.FAT_Entry[i].DIR_Name) == 0) {
                done = 1;
                break;
            }
            // Replace Volume Label ... ( FAT_Entry )
            if( Sector.FAT_Entry[i].DIR_Attr[0] & 0x08) {
                memcpy( Sector.FAT_Entry[i].DIR_Name, new_disk_label,
                        sizeof( Sector.FAT_Entry[i].DIR_Name));
            }
            // Force Date & Time of Non System files to version info
            if( memcmp( Sector.FAT_Entry[i].DIR_Name, "IO      SYS",
                        sizeof( Sector.FAT_Entry[i].DIR_Name)) &&
                memcmp( Sector.FAT_Entry[i].DIR_Name, "MSDOS   SYS",
                        sizeof( Sector.FAT_Entry[i].DIR_Name))   ) {
                a_day    = atoi( argv[7]);
                a_month  = atoi( argv[8]);
                a_year   = atoi( argv[9]) - 1980;
                fileDate.a_int =   a_day
                               + ( a_month  <<  5)
                               + ( a_year   <<  9);
                memcpy( Sector.FAT_Entry[i].DIR_WrDate, fileDate.a_char,
                        sizeof( Sector.FAT_Entry[i].DIR_WrDate));

                a_hour   = atoi( argv[4]);
                a_minute = atoi( argv[5]);
                a_second = atoi( argv[6]);
                fileTime.a_int =   a_second
                               + ( a_minute <<  5)
                               + ( a_hour   << 11);
                memcpy( Sector.FAT_Entry[i].DIR_WrTime, fileTime.a_char,
                        sizeof( Sector.FAT_Entry[i].DIR_WrTime));

                if(     Sector.FAT_Entry[i].DIR_Attr[0] & 0x20)
                    Sector.FAT_Entry[i].DIR_Attr[0] ^= 0x20;
            }
            // Set PXLBIOS?.DTA files Read-Only & Hidden
            if( !memcmp( Sector.FAT_Entry[i].DIR_Name,     "PXLBIOS", 7) &&
                !memcmp( Sector.FAT_Entry[i].DIR_Name + 8, "DTA",     3)   ) {
                if(     !( Sector.FAT_Entry[i].DIR_Attr[0] & 0x01))
                    Sector.FAT_Entry[i].DIR_Attr[0] |= 0x01;
                if(     !( Sector.FAT_Entry[i].DIR_Attr[0] & 0x02))
                    Sector.FAT_Entry[i].DIR_Attr[0] |= 0x02;
            }
        }
        if( fwrite( Sector.rawSector, SECTOR_SIZE, 1, destination) != 1) {
            giStatus = -90;
            goto error;
        }
    }
    // Copy rest of Source Image File to Destination Image File ...
    while( !feof( source)) {
        if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1)
            if( feof( source))
                break;
            else {
                giStatus = -100;
                goto error;
            }
        if( fwrite( Sector.rawSector, SECTOR_SIZE, 1, destination) != 1) {
            giStatus = -110;
            goto error;
        }
    }

error:
    // Close open files ...
    if( destination) fclose( destination);
    if( source)      fclose( source);

    if( giStatus) {
        giStatus -= 1000; /* customize error code */
        RaiseError( FALSE);
    }
    else {
        updSharedVar();
    }

    return( 0);
}
/*</code></function>

<function id="get_from_image">
<heading>
  <description><en>Extract a file from a Disk Image File.</en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
get_from_image ( argc, argv)
     int argc;
     char *argv[];
{
    unsigned long selFileSize;
             long remainingBytes; // Don't use 'unsigned' for remainingBytes !!
    unsigned int  selFileSector,
                  FirstRootDirSecNum,
                  RootDirSectors,
                  FirstDataSector,
                  ReservedSectorCount,
                  currSector,
                  done,
                  i;

    /* Init error handling */
    InitFunction( gcLibName, "get_from_image", "fileok", TRUE);
    if( giStatus == -1) return( 0);

    // Open Source Image File for reading ...
    if( ( source = fopen( argv[1], "rb")) == NULL) {
        giStatus = -10;
        goto error;
    }
    // Create Destination File for writing ...
    if( ( destination = fopen( argv[4], "wb")) == NULL) {
        giStatus = -20;
        goto error;
    }
    // Read Sector 0 - The Boot Sector -
    if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
        giStatus = -30;
        goto error;
    }
    // Check SECTOR_SIZE ...
    if( conv2( Sector.Sector0.BPB_BytsPerSec) != SECTOR_SIZE) {
        giStatus = -40;
        goto error;
    }

    FirstRootDirSecNum  =    conv2( Sector.Sector0.BPB_RsvdSecCnt)
                        + (  conv1( Sector.Sector0.BPB_NumFATs)
                           * conv2( Sector.Sector0.BPB_FATSz16));

    RootDirSectors      = (  ( conv2( Sector.Sector0.BPB_RootEntCnt) * 32)
                           + ( conv2( Sector.Sector0.BPB_BytsPerSec) - 1) )
                        / conv2( Sector.Sector0.BPB_BytsPerSec);

    FirstDataSector     = conv2( Sector.Sector0.BPB_RsvdSecCnt)
                        + (  conv1( Sector.Sector0.BPB_NumFATs)
                           * conv2( Sector.Sector0.BPB_FATSz16))
                        + RootDirSectors;

    ReservedSectorCount = conv2( Sector.Sector0.BPB_RsvdSecCnt);

    for( currSector = 1 ; currSector < FirstRootDirSecNum ; currSector++)
        if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
            giStatus = -50;
            goto error;
        }
    done          = 0;
    selFileSize   = 0;
    selFileSector = 0;
    while( !done) {
        if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
            giStatus = -60;
            goto error;
        }

        currSector += 1;
        for( i = 0 ; i < 16 ; i++) {
            if( strlen( Sector.FAT_Entry[i].DIR_Name) == 0) {
                done = 1;
                break;
            }
            if( !strncmp( Sector.FAT_Entry[i].DIR_Name,
                          argv[2], strlen( argv[2]))    &&
                !strncmp( Sector.FAT_Entry[i].DIR_Name + 8,
                          argv[3], strlen( argv[3]))    ) {
                selFileSize   = conv4( Sector.FAT_Entry[i].DIR_FileSize);
                selFileSector = conv2( Sector.FAT_Entry[i].DIR_FstClusLO);
                done = 1;
                break;
            }
        }
    }
    // Check if requested file is in Disk Image File ...
    if( selFileSize == 0 || selFileSector == 0) {
        giStatus = -40;
        goto error;
    }

    // Advance to the requested file ...
    for( ; currSector < selFileSector + FirstDataSector - 1 - ReservedSectorCount ;
           currSector++)
        if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
            giStatus = -80;
            goto error;
        }

    // Write the requested file ...
    for( remainingBytes = selFileSize ; remainingBytes > 0 ; currSector++) {
        if( fread( Sector.rawSector, SECTOR_SIZE, 1, source) != 1) {
            giStatus = -90;
            goto error;
        }
        if( remainingBytes > SECTOR_SIZE) {
            if( fwrite( Sector.rawSector, SECTOR_SIZE, 1, destination) != 1) {
                giStatus = -100;
                goto error;
            }
        } else {
            if( fwrite( Sector.rawSector, ( size_t) remainingBytes, 1, destination) != 1) {
                giStatus = -110;
                goto error;
            }
        }
        remainingBytes -= SECTOR_SIZE;
    }

error:
    // Close open files ...
    if( destination) fclose( destination);
    if( source)      fclose( source);

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

</body>
</source>
<!-- eof -->*/
