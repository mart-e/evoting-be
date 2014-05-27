void main(void)
{
asm {
  mov  ax,0xFFFF;
  push ax;
  xor  ax,ax;
  push ax;
  retf;
  };
}
