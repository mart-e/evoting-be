/* START CONFIDENTIAL */  
/*----------------------------------------------------------------------*/
#ifndef uchar
#define uchar   unsigned char
#endif

#ifndef uint
#define uint    unsigned int
#endif

typedef struct
  {
    uchar far *next_write; /* ptr to the next element to write       */
    uchar far *next_read;  /* ptr to the next element to read        */
    uchar far *start;      /* ptr to the begin of the queue          */
    uchar far *end;        /* ptr to the last element of the queue   */
    uint element_count;     /* amount of elements in the queue        */
    uint max_elements;      /* max elements possible in the queue     */
  } UQMC;

int  uqmc_init( uint count, uchar _far *ptr, UQMC *uqmc);
int  uqmc_read( UQMC *uqmc);
int  uqmc_read_ahead( UQMC *uqmc);
int  uqmc_write( uchar c, UQMC *uqmc);
uint uqmc_check_used( UQMC *uqmc);
uint uqmc_check_free( UQMC *uqmc);
void uqmc_flush( UQMC *uqmc);
void uqmc_exit( UQMC *uqmc);
/* END CONFIDENTIAL */  
