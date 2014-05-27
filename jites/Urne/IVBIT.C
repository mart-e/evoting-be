#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <io.h>
#include <conio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
/*
#define TRACE_IVBIT 1
#define TRACE_BE3 1
*/
unsigned char ivbit[129];
#ifdef TRACE_IVBIT
FILE *fu;
#endif /*TRACE_IVBIT*/
#ifdef TRACE_BE3
FILE *fu_be3;
#endif /*TRACE_BE3*/
static unsigned long far last_timer = 0;

const char Tab_Ivbit[] = {"0123456789ABCDEF"};

extern unsigned long far *utimer;
extern int Cmpt_Gen;
extern unsigned char iv[100];
extern unsigned char ivtmp[100];

/********************/
void Read_Ivbit(void)
/********************/
{
	int i, j, hex;
	unsigned char szTemp[5];
	unsigned char szIvbit[33];

	/* _Ivbit -> HEXA */
	szIvbit[0] = 0x00;
	for(i=0; i<32;++i)
	{
		strncpy(szTemp, ivbit+(i*4), 4);
		szTemp[4] = 0x00;
		hex = 0;
		if(szTemp[3] == '1') hex += 1;
		if(szTemp[2] == '1') hex += 2;
		if(szTemp[1] == '1') hex += 4;
		if(szTemp[0] == '1') hex += 8;
		szIvbit[i] = Tab_Ivbit[hex];
	}
	szIvbit[32] = 0x00;
#ifdef TRACE_BE3
	fu_be3 = fopen("be3.txt", "a");
	fputs(ivbit, fu_be3);
	fputs("\n", fu_be3);
	fputs(szIvbit, fu_be3);
	fputs("\n", fu_be3);
	fclose(fu_be3);
	system("copy c:\\be3.txt a:");
#endif
	/*strcpy(_Ivbit, szIvbit);*/

	/* _Ivbit -> bits */
	for(i = 0 ; i<32; i++)
	{
		if(szIvbit[i] < 58)
			szIvbit[i] -= 48;
		else
			szIvbit[i] -= 55;
	}

	for(i = 0, j = 0; j<16; j++)
	{
		ivtmp[j] = szIvbit[i++] << 4;
		ivtmp[j] = szIvbit[i++] | ivtmp[j];
		iv[j] = ivtmp[j];
	}
	ivtmp[j] = 0x00;
	iv[j] = 0x00;

#ifdef TRACE_IVBIT
	fclose(fu);
	system("copy c:\\trace.txt a:");
#endif /*TRACE_IVBIT*/
}

/**************************/
void Maj_IvBit(char _cType)
/**************************/
{
	static int iPos = 0;
	char szTemp[16];
	char *pzTemp = szTemp;
	/*unsigned char h, m, s;*/

	if(_cType == 1) /* Cmpt_Gen */
	{
		sprintf(ivbit, "%s%d", ivbit+1, Cmpt_Gen%2);
#ifdef TRACE_IVBIT
		fputs(ivbit, fu);
		fputs("\n", fu);
#endif /*TRACE_IVBIT*/
	}
	else /* Time */
	{
		if((*utimer) != last_timer)
		{
			last_timer = (*utimer);
			/* hours = ( *time / 65520 ) ;
			   min = ( *time - hours * 65520 ) / 1092 ;
			   sec = ( *time - hours * 65520 - min * 1092 ) * 10 / 182 ; */
			/*h = ( last_timer / 65520 ) ;
			m = ( last_timer - h * 65520 ) / 1092 ;
			s = ( last_timer - h * 65520 - m * 1092 ) * 10 / 182;
			iPos = s;*/
			sprintf(pzTemp, "%ld", last_timer);
			pzTemp += strlen(pzTemp) - 2;
			iPos += atoi(pzTemp);
			if(iPos > 127) iPos -= 127;
			/* Si décalage
				sprintf(ivbit, "%s%d", ivbit+1, last_timer%2);*/
			sprintf(pzTemp, "%d", last_timer%2);
			ivbit[iPos] = pzTemp[0];
#ifdef TRACE_IVBIT
			fputs(ivbit, fu);
			fprintf(fu, " %ld -> %d: %s", last_timer, iPos, pzTemp);
			fputs("\n", fu);
#endif /*TRACE_IVBIT*/
		}

	}
}

/***************************************/
void Build_Ivbit(unsigned char *_Bffer)
/***************************************/
{
	int i, j;

	for(i=j=0; i<32;++i)
	{
		switch(_Bffer[i])
			{
			case '1' :
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				break;
			case '2' :
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				break;
			case '3' :
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				break;
			case '4' :
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				break;
			case '5' :
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				break;
			case '6' :
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				break;
			case '7' :
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				break;
			case '8' :
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				break;
			case '9' :
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				break;
			case 'A' :
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				break;
			case 'B' :
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				break;
			case 'C' :
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				break;
			case 'D' :
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				ivbit[j++] = '1';
				break;
			case 'E' :
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '0';
				break;
			case 'F' :
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				ivbit[j++] = '1';
				break;
			default  :
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
				ivbit[j++] = '0';
		}
	}
	ivbit[j] = 0x00;

#ifdef TRACE_IVBIT
	fputs(ivbit, fu);
	fputs("\n", fu);
#endif /*TRACE_IVBIT*/
#ifdef TRACE_BE3
	fu_be3 = fopen("be3.txt", "a");
	fputs(_Bffer, fu_be3);
	fputs("\n", fu_be3);
	fputs(ivbit, fu_be3);
	fputs("\n", fu_be3);
	fclose(fu_be3);
#endif
}

/*********************/
void Read_Be2(void)
/*********************/
{
	unsigned char Bffer[100];
	int fd, handel_Read;

	/* Fichier TRace */
#ifdef TRACE_IVBIT
	fu = fopen("c:\\trace.txt", "w");
#endif /*TRACE_IVBIT*/
	/* Lecture be.2 */
	fd = open("a:\\be.2", O_RDWR | O_BINARY);
	if(fd == -1)
		return;

	handel_Read = read(fd,Bffer,64);
	if(handel_Read == -1)
		return;
	close(fd);

	Bffer[handel_Read] = 0x00;
/* << 10.03b */
	Build_Ivbit(Bffer);
/* 10.03b >> */
}
