//=======================================================================

#ifndef GENERAL_H
#define GENERAL_H

#define program_version "6.3"

enum SYSTEM
{
#ifndef RTC_LOAD
   MAV,        //  0
   URN,        //  1
   TOT,        //  2
   PRP,        //  3
   COMPONENTS, //  4
   NAME,       //  5
   CONFIG,     //  6
   INFO,       //  7
#endif // RTC_LOAD
   DATEINFO,   //  8 -> 0 for RTC-LOAD
   TIMEINFO,   //  9 -> 1 for RTC-LOAD
   INFO_WINCOR // 10
};
typedef enum { UNKNOWN_BIOS,
			   UNSUPPORTED_BIOS,
			   AMI_1, AMI_2, AMI_3,     // DV I
			   BAD_AMI,                 // DV I (not specific digivote)
			   AMI_REPLACEMENT,         // DV I replacement !
			   AWARD_P5SJ_B,            // DV II
			   AWARD_W6178MS,           // DV III
			   AWARD_UPGRADE,           // DV II/III without Bull/E-vote
			   FS_SCENIC_D1561,         // Fujitsu-Siemens SCENIC P300 (D1561)
			   FS_SCENICO_D1711         // Fujitsu-Siemens SCENICO P (D1711)
			 } BIOS_TEST;

typedef enum { UNKNOWN_VERSION,
			   CORRECT_VERSION,
			   AMI_V693_Baby_AT,        // DV I replacement !
			   AWARD_v451PG_Ver_20c,    // DV II  - AWARD v4.51PG - Ver 2.0c
			   AWARD_v451PG_Ver_20d,    // DV II  - AWARD v4.51PG - Ver 2.0d
			   AWARD_v600PG_V20,        // DV III - AWARD v6.00PG - V2.0
			   SIEMENS_CONSTRUCTOR,     // FUJITSU-SIEMENS
			   SIEMENS_MODEL,           // SCENIC P300
			   SIEMENS_BIOS_TYPE_D1561, // Phoenix Technologies Ltd.
			   SIEMENS_BIOS_REV_D1561,  // 4.06  Rev. 1.06.1561
			   SIEMENS_BIOS_TYPE_D1711, // American Megatrends Inc.
			   SIEMENS_BIOS_REV_D1711   // 08.00.09 Rev. 1.06.1711
			 } BIOS_VERSION;

typedef enum { NOT_WIZARD,
			   WIZARD,
			   WIZARDS,
			   WIZZARD
			 } IS_WIZARD;

typedef int BOOLEAN;

#ifndef _APPLICATION_MAV
#ifndef _APPLICATION_URN

#define FALSE 0
#define TRUE  1

#define NOKEY				   -1
#define ENTER				 0x0D
#define ESCAPE			     0x1B
#define TAB				     0x09
#define SPACE				 0x20
#define BACKSPACE            0x08
#define SPECIAL_CHAR         0x00
#define SPECIAL_DELETE       0x53 // Follows always SPECIAL_CHAR
#define SPECIAL_CURSOR_LEFT  0x4B // Follows always SPECIAL_CHAR
#define SPECIAL_CURSOR_RIGHT 0x4D // Follows always SPECIAL_CHAR
#define SPECIAL_CURSOR_UP    0x48 // Follows always SPECIAL_CHAR
#define SPECIAL_CURSOR_DOWN  0x50 // Follows always SPECIAL_CHAR

#endif
#endif

#endif
