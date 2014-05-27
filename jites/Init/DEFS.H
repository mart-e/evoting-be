#define  true  1
#define  false 0
typedef int boolean;

/*******************************  EPROM *************************************/

#define _EPROM_TAILLE  127

#define _LCD_REG_BASE      (0x378)                  
#define _LCD_REG_DONNEE    (_LCD_REG_BASE + 0x0000) /* Adresse du port */
                                                    /* de donnee       */
#define _LCD_REG_CONTROLE  (_LCD_REG_BASE + 0x0002) /* Adresse du port */
                                                    /* de controle     */
#define _LCD_TEMPS_ATTENTE (0x30)








