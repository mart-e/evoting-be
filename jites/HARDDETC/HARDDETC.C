#include <bios.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#define ERRORLEVEL_MAV_1     1
#define ERRORLEVEL_MAV_2     2
#define ERRORLEVEL_URNE_1    3
#define ERRORLEVEL_URNE_2    4

#define JITES_MAV_ADD_VER1  0xC0000087
#define JITES_MAV_TXT_VER1  "Voting Terminal"
#define JITES_MAV_ADD_VER2  0xC00000DF
#define JITES_MAV_TXT_VER2  "Voting Terminal"
#define JITES_URNE_ADD_VER1  0xFE9D0022
#define JITES_URNE_TXT_VER1  "Version 1.0.1"
#define JITES_URNE_ADD_VER2  0xC7FC0019
#define JITES_URNE_TXT_VER2  "Version 2.0.0"


char *ReadMemory(char* pzString, unsigned long ulAddress, unsigned int uiSize)
{
 unsigned char ucIndex;
 unsigned char far *pointy;

 /*Read direcly in the memory*/
 pointy = (unsigned char far*)ulAddress;

 for(ucIndex = 0;ucIndex < uiSize;ucIndex++)
     pzString[ucIndex] = *pointy++;

 /* insert null*/
 pzString[ucIndex] = 0x00;

 return pzString;
}


/**************************** CORPS DU PROGRAMME ***************************/
int main(void)
{
  char szTerminal[16];

  if(strcmp((char*)ReadMemory((char*)szTerminal , JITES_MAV_ADD_VER1, 15), JITES_MAV_TXT_VER1) == 0)
  {
    return ERRORLEVEL_MAV_1;
  }
  else if(strcmp(ReadMemory((char*)szTerminal , JITES_MAV_ADD_VER2, 15), JITES_MAV_TXT_VER2) == 0)
  {
    return ERRORLEVEL_MAV_2;
  }
  else if(strcmp(ReadMemory((char*)szTerminal , JITES_URNE_ADD_VER1, 13), JITES_URNE_TXT_VER1) == 0)
  {
    return ERRORLEVEL_URNE_1;
  }
  else if(strcmp(ReadMemory((char*)szTerminal , JITES_URNE_ADD_VER2, 13), JITES_URNE_TXT_VER2) == 0)
  {
    return ERRORLEVEL_URNE_2;
  }

  return -1;
}
