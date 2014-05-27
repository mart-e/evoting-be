#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>

union REGS rin,rout;

int main(void)
{
 char media;
 unsigned int i, med2;

 for(i = 65535, med2 = 10;(i == 65535) && med2 > 0;med2--) /* 65535 = erreur */
  {
     rin.h.ah = 0x36;
     rin.h.dl = med2;
     int86(0x21,&rin,&rout);
     i = rout.x.ax;
  }

  /* positionnement sur la derniere unit‚ logique */
  rin.h.ah = 0x0E;
  rin.h.dl = med2;
  int86(0x21,&rin,&rout);

  return(0);
}
