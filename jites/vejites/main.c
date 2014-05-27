/* vejites compare les fichiers 'a:\pct?.dsk' et 'c:\jites\pct?.dsk'
    si pas de fichier 'pct?.dsk' dans 'a:' ou dans 'c:\jites\'
    ou si les contenus de ces fichiers sont différents
    -> renomme le répertoire 'c:\jites\' en 'c:\jites.x\'  (x == 1, 2, 3... ) */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>

#define FILE_NO_DISK_SPACE "c:\jites.spa"

typedef unsigned char BYTE;

/*******************/
int space(void)
/*******************/
{
	struct dfree free;
	long avail, min_space = 5000000;
	int drive;

	drive = 2/*getdisk()*/;
	getdfree(drive+1, &free);
	if(free.df_sclus == 0xFFFF)
	{
		printf("Error in getdfree() call\n");
		/*exit(1);*/
		return(-1);
	}

	avail =  (long) free.df_avail * (long) free.df_bsec * (long) free.df_sclus;
	/*printf("avail: %ld (>5000000)\n", avail);
	printf("df_avail: %ld\n", (long) free.df_avail);
	printf("df_bsec : %ld\n", (long) free.df_bsec);
	printf("df_sclus: %ld\n", (long) free.df_sclus);*/


	if(avail < min_space)
		return 0;

	return 1;
}
/*******************/
void renjites(void)
/*******************/
{
	char buff[16];
	FILE *fu;
	int i;

	/* Assez d'espace disque ? */
	if(!space())
	{
		/*printf("Pas assez d'espace sur le disque pour sauvegarder");*/
		fu = fopen(FILE_NO_DISK_SPACE, "w");
		if(!fu)
			printf("Can't create the file: '%s'", FILE_NO_DISK_SPACE);
		else
			fclose(fu);
			return;
	}

	for(i=1; i<100; ++i) /* jites.100 (...)*/
	{
		sprintf(buff, "c:\\jites.%d", i);
		if(rename("c:\\jites", buff) == -1) /* Ne peut renommer */
		{
			switch(errno)
			{
				case EACCES: /* Fichier cible existe déjà */
					break;
				case ENOENT: /* Fichier non trouvé */
					return;
				case ENOTSAM: /* Unité de disque différente */
					break;
			}
		}
	}
}

/************************************************/
#define RETRY  1
#pragma argsused
int handler(int errval, int ax, int bp, int si)
/************************************************/
{
	return RETRY;
}
/********************************/
void main(void)
/********************************/
{
	int handle1, handle2, res_renjites;
	BYTE *buff1, *buff2;
	unsigned fl;

	harderr(handler);

	remove(FILE_NO_DISK_SPACE);

	/* a:\\pct?.dsk*/
	handle1 = open("a:\\pct1.dsk", O_RDONLY);
	if(handle1 == -1)
	{
		handle1 = open("a:\\pct2.dsk", O_RDONLY);
		if(handle1 == -1)
		{
			handle1 = open("a:\\pct3.dsk", O_RDONLY);
			if(handle1 == -1)
			{
				/* Si on ne trouve pas de fichier a:\\pct?.dsk on renomme c:\\jites */
				renjites();
				return;
			}
		}
	}

	/* c:\\jites\\pct?.dsk */
	handle2 = open("c:\\jites\\pct1.dsk", O_RDONLY);
	if(handle2 == -1)
	{
		handle2 = open("c:\\jites\\pct2.dsk", O_RDONLY);
		if(handle2 == -1)
		{
			handle2 = open("c:\\jites\\pct3.dsk", O_RDONLY);
			if(handle2 == -1)
			{
				/* Si on ne trouve pas de fichier c:\\jites\\pct?.dsk on renomme c:\\jites */
				close(handle1);
				renjites();
				return;
			}
		}
	}

	/* Comparaison des fichiers pct?.dsk */
	fl = filelength(handle1);

	/* Taille identique ? */
	if(fl != filelength(handle2))
	{
		close(handle1);
		close(handle2);
		renjites();
		return;
	}

	/* a:\\pct?.dsk*/
	buff1 = malloc(fl);
	read(handle1, buff1, fl);
	buff1[fl-1] = 0x00;

	/* c:\\jites\\pct?.dsk */
	buff2 = malloc(fl);
	read(handle2, buff2, fl);
	buff2[fl-1] = 0x00;

	/* 1) pct?.dsk identiques ? */
	if(strcmp(buff1, buff2)) /* différents -> Renommer */
	{
		free(buff1);
		free(buff2);
		close(handle1);
		close(handle2);
		renjites();
		return;
	}

	/* 2) RW ? */
	if(access("a:\\comm.pc", 0) == 0)
	{
		if(access("c:\\jites\\comm.pc", 0) != 0)
		{
			/* comm.pc présent sur disquette et non sur e:\jites -> Renommer */
			free(buff1);
			free(buff2);
			close(handle1);
			close(handle2);
			renjites();
			return;
		}
	}
	else
	{
		if(access("a:\\prov.pc", 0) == 0)
		{
			if(access("c:\\jites\\prov.pc", 0) != 0)
			{
				/* prov.pc présent sur disquette et non sur e:\jites -> Renommer */
				free(buff1);
				free(buff2);
				close(handle1);
				close(handle2);
				renjites();
				return;
			}
		}
		else /* Pas de comm.pc ni de prov.pc sur la disquette -> LV -> "on démarre" */
		{
			;
		}
	}

	/* 3) LV -> Renommer  */

	free(buff1);
	free(buff2);

	close(handle1);
	close(handle2);
}
