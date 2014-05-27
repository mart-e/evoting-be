/* START CONFIDENTIAL */
/***********************************************/
#define  NMX_SCRUTIN_T         7

#define  IS_NONE               0
#define  IS_EUROPE             1
#define  IS_CHAMBRE            2
#define  IS_SENAT              3
#define  IS_REGION             4
#define  IS_PROVINCE           5
#define  IS_COMMUNE            6
#define  IS_CONSEIL_FLAMANT    7


#define  VM_EUROPE             1
#define  VM_CHAMBRE            1
#define  VM_SENAT              1       /* votes multiples */
#define  VM_REGION             1
#define  VM_PROVINCE           1
#define  VM_COMMUNE            1
#define  VM_CONSEIL_FLAMANT    1

#define  NMXV_EUROPE          (1+21)
#define  NMXV_CHAMBRE         (1+34)
#define  NMXV_SENAT           (1+39)     /* nb max candidat */
#define  NMXV_REGION          (1+88)
#define  NMXV_PROVINCE        (1+87)
#define  NMXV_COMMUNE         (1+87)
#define  NMXV_CONSEIL_FLAMANT (1+12)

#define  NMXB_EUROPE          ( 6)
#define  NMXB_CHAMBRE         ( 9)
#define  NMXB_SENAT           (10)     /* nb max bytes */
#define  NMXB_REGION          (23)
#define  NMXB_PROVINCE        (22)
#define  NMXB_COMMUNE         (22)
#define  NMXB_CONSEIL_FLAMANT ( 4)

typedef struct {
  int              swVM;
  int              nmxV;
  int              nmxB;
} CFGSCRUT;
/***********************************************/
#define  AGO_SWDEBUG 0
/***********************************************/
/* END CONFIDENTIAL */
