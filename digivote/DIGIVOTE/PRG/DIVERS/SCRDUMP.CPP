#include <dos.h>
#include <dir.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <fstream.h>

#include "..\Diagnost\General.h"
#include "..\Diagnost\DosError.h"

char dump_file[] = "?:\\SCRDUMP.NUL";
char raw_buff[200], txt_buff[100];

//#pragma warn +par
char Get_RAMdrive( void)
{
	char Drive;
	struct find_t Control_Block;
	char PathName[6];
	BOOLEAN Found = FALSE;

	Found = FALSE;
	for( Drive='Z' ; Drive>='C' ; Drive--)
	{
		sprintf( PathName, "%c:*.*", Drive);
		if( _dos_findfirst( PathName, _A_VOLID, &Control_Block)==0)
			if( strcmp( Control_Block.name, "MS-RAMDR.IVE")==0)
			{
				Found = TRUE;
				break;
			}
	}
	if( !Found)
	   Drive = 'A' + getdisk();
	#ifndef _LIGHTPEN_
	DosErrorNr = 0;
	#endif // _LIGHTPEN_
	return( Drive);
}

void main(void)
{
   int i, j, k;

   remove( &dump_file[3]);
   dump_file[0] = Get_RAMdrive();
   remove( dump_file);
   ofstream ScrDump( dump_file);
   for( i = 1 ; i <= 25 ; i++)
   {
	  gettext(1, i, 80, i, raw_buff);
	  for( j = 0, k = 0 ; j < strlen( raw_buff) ; j++)
		 if( ( j % 2) == 0)
			txt_buff[k++] = raw_buff[j];
	  for( j = strlen( txt_buff) - 1 ; j > 0 ; j--)
		 if( txt_buff[j] == ' ')
		 {
			txt_buff[j]     = '\n';
			txt_buff[j + 1] = '\0';
		 }
		 else
			j = 0;
	  ScrDump << txt_buff;
   }
   ScrDump.close();
}
