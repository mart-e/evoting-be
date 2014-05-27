 //=============================================================================
 /*

   File        : ETCARD.CPP
   Creation    : 19/04/95

   Project     : Bull DIGIVOTE
   Author      : TMT

   Description : ETCARD.CPP header file

 */
 //=============================================================================

 //======================== I N C L U D E   F I L E S ==========================

 #include <dos.h>
 #include <conio.h>
 #include <ctype.h>
 #include <stdlib.h>
 #include <time.h>
 #include <graphics.h>
 #include <setjmp.h>
 #include <string.h>
 #include <alloc.h>

 #include "..\gen\gentype.h"
 #include "..\gen\gencard.h"
 #include "..\gen\genlang.h"
 #include "..\gen\gendata.h"
 #include "..\gen\genflop.h"
 #include "..\gen\genvideo.h"
 #include "..\gen\gencryp.h"

 #include "..\urn\urnint.h"
// #include "..\urn\urnx25.h"
 #include "..\urn\urnmes.h"
// #include "..\urn\urnmsg.h"
 #include "..\urn\urnerro.h"
 #include "..\urn\urnreco.h"

 //=================== E X T E R N A L   V A R I A B L E S =====================

 extern Select_Reader * Intern_Reader;

 extern char Floppy_MASTER[];
 extern char Floppy_BACKUP[];
 extern char *Floppy_Current;

 extern struct ElectionMain Root;
 extern struct Select_College Colleges;
 extern struct Select_Party Parties;
 extern struct Select_Candidate Candidates;
 extern struct cardinfo Card_Info_Per_Election[MAX_ELECTIONS];

 extern int CurrentTypeElector;
 extern int PositionCardTypeElector;

#if 0
 extern BYTE Result_Key  [];              // holds MAC
 extern AESKEY mKey;                // stores the MASTER  key
 extern AESKEY sKey;
 extern AESKEY mavSessionKey;
#endif

 extern char Vote_Data [];
 extern char Blanco_Votes [];

 //=================== F U N C T I O N   P R O T O T Y P E S =====================

 void Convert_Card_Buffer_To_Vote_Buffer(BYTE*,int*,BYTE*,int*);
 extern void Decrypt_Session_Key( BYTE * key);

 // ================== D E F I N E =============================================

 #define C_F10         324
 #define C_ESC         27
 #define C_DOWN        336
 #define C_UP          328
 #define C_PAGEDOWN    337
 #define C_PAGEUP      329
 #define C_N 78
 #define C_n 110
 #define C_P 80
 #define C_p 112

 #define C_BACKSPACE 8
 #define C_A 65
 #define C_Z 90
 #define C_a 97
 #define C_z 122

 struct localcardinfo
 {
    Boolean VotedFor;
    char* Election;
    char* College;
    int Party;
    int HeadVote;
    char Effectives[MAX_CANDIDATES];
    char Suppliants[MAX_CANDIDATES];
 };

 static struct localcardinfo Local_Card_Info[MAX_ELECTIONS];
 char EmptyString[2];

 /*------------------------------------------------------------------------*/

struct English_Text
 {
   int ElectInd;
   char* Election;
   char* College[3];
 };

struct English_Text EngText[] =
 {
   1, "Council of the Brussels-Capital Region"   , ""             , ""              , ""              ,
   2, "Municipal Council"                        , ""             , ""              , ""              ,
   3, "House of Representatives"                 , ""             , ""              , ""              ,
   4, "Senate"                                   , "Dutch college", "French college", ""              ,
   5, "Council of the German-speaking Community" , ""             , ""              , ""              ,
   6, "Provincial Council"                       , ""             , ""              , ""              ,
   7, "European Parliament"                      , "Dutch college", "French college", "German college",
   8, "Flemish Council"                          , ""             , ""              , ""              ,
   9, "Walloon Regional Council"                 , ""             , ""              , ""              ,
 };
int num_EngText = sizeof(EngText) / sizeof(struct English_Text);


 //============================================================================

void ScrHideCur( void )
 {
  int i;
  union REGS Reg;

  i=0;
  Reg.h.bh = (unsigned char)i;
  i=24;
  Reg.h.dh = (unsigned char)i;
  i=80;
  Reg.h.dl = (unsigned char)i;
  Reg.h.ah = 2;
  int86(0x10, &Reg, &Reg);
 }

void ScrBox( int ULC, int ULR, int LRC, int LRR, int Style)
 {
                                             // style may be from 0 to 3
   register int i; // Loop counter
   int Delta1, Delta2; // Bytes between lines

   static char ulc[] = {218,201,213,214}; // Box characters for each style
   static char urc[] = {191,187,184,183};
   static char llc[] = {192,200,212,211};
   static char lrc[] = {217,188,190,189};
   static char hl[] =  {196,205,205,196};
   static char vl[] =  {179,186,179,186};

   char far* Video; // far pointer to video memory
   unsigned char far* PtrVideoMode = (unsigned char far*) 0x00400049;

   ULR--; ULC--; LRR--; LRC--; // coordinates calculate from 0

   Delta1 = (LRC -ULC) * 2; // Bytes between 2 vertical lines
   Delta2 = 160 - Delta1; // Bytes between right vertical line and left
                          // vertical line of next row
   // Init. far pointer to video memory: upper left corner of box
   FP_SEG (Video) = (*PtrVideoMode == 7) ? 0xb000 : 0xb800;
   FP_OFF (Video) = ULR*160 + ULC*2;

   *Video = ulc[Style]; // Draw upper left corner
   Video += 2; // Skip attrib. byte
   for(i=1; i <= LRC-ULC-1; ++i) // Draw top horizontal line
   {
      *Video = hl[Style];
      Video += 2;
   }
   *Video = urc[Style]; // Draw upper right corner
   Video += Delta2;
   for(i=1; i <= LRR-ULR-1;++i) // Draw both vertical lines
   {
      *Video = vl[Style];
      Video += Delta1;
      *Video = vl[Style];
      Video += Delta2;
   }
   *Video = llc[Style]; // Draw lower left corner
   Video += 2;
   for(i=1; i <= LRC-ULC-1; ++i) // Draw Buttom horizontal line
   {
      *Video = hl[Style];
      Video += 2;
   }
   *Video = lrc[Style]; // Draw lower right corner
 }

void ScrButtomBar( int c1, int c2, int c3, int c4 )
 {
   char char2[2];
   memset(char2,'\0',sizeof(char2));

   window(1,25,80,25);
   textcolor(BLACK);
   textbackground(LIGHTGRAY);
   clrscr();
   if( c1 != 0 )
   {
      gotoxy(2,1);
      textcolor(BLACK);
      cputs("ESC");
      gotoxy(6,1);
      textcolor(DARKGRAY);
      cputs("Retrieve Card");
   }
   if( c2 != 0 )
   {
      textcolor(BLACK);
      char2[0]=0x19;
      gotoxy(30,1);
      cputs(char2);
      gotoxy(32,1);
      cputs("N");
      textcolor(DARKGRAY);
      gotoxy(33,1);
      cputs("EXT");
   }
   if( c3 != 0 )
   {
      textcolor(BLACK);
      char2[0]=0x18;
      gotoxy(45,1);
      cputs(char2);
      gotoxy(47,1);
      cputs("P");
      textcolor(DARKGRAY);
      gotoxy(48,1);
      cputs("REVIOUS");
   }
   if( c4 != 0 )
   {
      gotoxy(72,1);
      textcolor(BLACK);
      cputs("F10");
      gotoxy(76,1);
      textcolor(DARKGRAY);
      cputs("Exit");
   }
   window(1,1,80,25);
   ScrHideCur();
   textcolor(WHITE);
   textbackground(BLACK);
 }

void ScrTextCentered( char* string, int line )
 {
   gotoxy((80-strlen(string))/2,line);
   cputs(string);
 }

void ScrExit(char* ErrorString)
 {
   window(1,1,80,25);
   clrscr();
   ScrBox( 1, 1, 80, 25, 1);
   highvideo();
   ScrTextCentered(ErrorString,10);
   ScrBox(10,13,70,17,0);
   ScrTextCentered("P O W E R    D O W N",15);
   ScrHideCur();

   for(;;);                  // endless loop
 }

void ShowMinimized(int InfoCounter, int *line)
 {
  int col=2;
  char char3[3];
  char char80[80];
  int phase=0;
  int len;

  window(1,*line,80,*line+3);
 //  ScrBox(1,*line,80,*line+2, 0);

  gotoxy(76,2);
  sprintf(char3,"-%d-",InfoCounter+1);
  lowvideo();
  cputs(char3);

  while(1)
  {
    switch(phase)
    {
      case 0:
        strcpy(char80,"Election: ");
        lowvideo();
        break;
      case 1:
        strcpy(char80,Local_Card_Info[InfoCounter].Election);
        highvideo();
        if( Local_Card_Info[InfoCounter].VotedFor == FALSE)
           phase=97;
        break;
      case 2:
        if( Local_Card_Info[InfoCounter].College[0] == '\0' )
          char80[0]='\0';
        else
          strcpy(char80," - ");
        lowvideo();
        break;
      case 3:
        if( Local_Card_Info[InfoCounter].College[0] == '\0' )
          char80[0]='\0';
        else
          strcpy(char80,Local_Card_Info[InfoCounter].College);
        highvideo();
        break;
      case 98:
        strcpy(char80," - VOTING NOT ALLOWED");
        break;
      default:
        phase = 99;
    }
    if(phase == 99)
      break;
    phase++;
    len=strlen(char80);
    if(col+len > 80)
      break;
    gotoxy(col,2);
    cputs(char80);
    col+=len;
  }

  *line+=3;
  window(1,1,80,25);
  ScrHideCur();
 }

void ShowMaximized(int InfoCounter, int *line)
 {
  int col;
  char char3[3],char4[4];
  char char80[80];
  int phase=0;
  int len,i,j,k,l, printnow;
  int winline=2;

  window(1,*line,80,*line+12);

  while(1)
  {
    switch(phase)
    {
      case 0:
        col=2;
        strcpy(char80,"Election: ");
        lowvideo();
        break;
      case 1:
        strcpy(char80,Local_Card_Info[InfoCounter].Election);
        highvideo();
        if( Local_Card_Info[InfoCounter].VotedFor == FALSE)
           phase=97;
        break;
      case 2:
        if( Local_Card_Info[InfoCounter].College[0] == '\0' )
          char80[0]='\0';
        else
          strcpy(char80," - ");
        lowvideo();
        break;
      case 3:
        if( Local_Card_Info[InfoCounter].College[0] == '\0' )
          char80[0]='\0';
        else
          strcpy(char80,Local_Card_Info[InfoCounter].College);
        highvideo();
        break;
      case 4:
        col=2;
        winline++;
        strcpy(char80,"Party: ");
        lowvideo();
        break;
      case 5:
        if(Local_Card_Info[InfoCounter].Party == 0)
          strcpy(char80,"BLANCO");
        else
          sprintf(char80,"%d",Local_Card_Info[InfoCounter].Party);
        highvideo();
        break;
      case 6:
        strcpy(char80,"   Head vote: ");
        lowvideo();
        break;
      case 7:
        if(Local_Card_Info[InfoCounter].Party == 0)
          strcpy(char80,"-");
        else
        {
          if(Local_Card_Info[InfoCounter].HeadVote > 3)
            strcpy(char80,"YES");
          else
            strcpy(char80,"NO");
        }
        highvideo();
        break;
      case 98:
        strcpy(char80," - VOTING NOT ALLOWED");
        break;
      default:
        phase = 99;
    }
    if(phase == 99)
      break;
    phase++;
    len=strlen(char80);
    if(col+len > 80)
      break;
    gotoxy(col,winline);
    cputs(char80);
    col+=len;
  } /* end while(1) */

  winline++;
  if( Local_Card_Info[InfoCounter].VotedFor == TRUE)
  {
     for(k=0;k<2;k++)
     {
       l=0;
       for(i=0, j=0; i<MAX_CANDIDATES; i++)
       {
         if(i == 0)
         {
           if(k == 0)
             strcpy(char80,"Effective nbr:");
           if(k == 1)
             strcpy(char80,"Suppliant nbr:");
           lowvideo();
           gotoxy(2,winline);
           cputs(char80);
           char80[0] = '\0';
           highvideo();
           printnow = 0;
         }
         if( (k == 0 && Local_Card_Info[InfoCounter].Effectives[i] == '1') ||
             (k == 1 && Local_Card_Info[InfoCounter].Suppliants[i] == '1') )
         {
           l=1;
           sprintf(char4,"%2d ", i+1);
           strcat(char80,char4);
           j++;
           if(j>=21)
           {
             printnow = 1;
           }
         }
         if( printnow == 1 || i == MAX_CANDIDATES-1 )
         {
           gotoxy(16,winline);
           if( strlen(char80) == 0 )
             if( l == 0 )
               strcpy(char80," -");
             else
               winline--;
           cputs(char80);
           winline++;
           printnow = 0;
           char80[0] = '\0';
           j = 0;
         }
       } /* end for( i ) */
     } /* end for( k ) */
  } /* end if( VotedFor == TRUE) */

  winline--;
  ScrBox(1,*line,80,*line+winline, 1);
  gotoxy(77,1);
  lowvideo();
  sprintf(char3,"%d",InfoCounter+1);
  cputs(char3);

  *line+=winline+1;
  window(1,1,80,25);
  ScrHideCur();
 }

int key_get( void )
 {
   int c;
   if( !(c = getch() ) )
       c = getch() | 0x100;
   return( c );
 }

int UpperCase( int key )
 {
   if( key >= C_a && key <= C_z )
       key-=32;
    return(key);
 }

 /*------------------------------------------------------------------------*/

int key_edit(int row,int col,char* text)
  {
    int column;
    int c;
    int pos = 0;
    int length = 0;
    char buffer[21];

    while( text[length] )
      length++;
    if( length > 20 )
      return(2);

    strcpy(buffer,text);

    for(;;)
    {
      gotoxy(col,row);
      cputs(buffer);
      gotoxy(col+pos,row);

      if(pos >= length)
         return(0);                           // EXIT OK

      c = UpperCase( key_get() );

      if( c == C_BACKSPACE && pos > 0 )
      { pos--;
        text[pos] = '.';
        buffer[pos] = text[pos];
      }
      else
      if( (c >  96 && c < 123) ||        // a-z
          (c >  63 && c <  91) ||        // A-Z
          isdigit( c))                   // 0-9
//      if( c >= C_A && c <= C_Z)
      { if(pos <= length)
        {
            text[pos] = c;
            buffer[pos]='*';
            pos++;
        }
      }
      else
        printf( "\7");                                         // beep

    } /* end for */
  } /* end key_edit */

 /*------------------------------------------------------------------------*/

int ETInitSET( char* Value, long Pos )
 {
   FILE *fh_;
   char PosVal[2];

   int rslt;

   if( ( fh_ = fopen( "a:\\autoexec.bat", "rt")) == NULL )
      return( -1);

   if( ( rslt = fseek( fh_, Pos, SEEK_SET)) != 0 )
   {
      fclose( fh_);
      return( -2);
   }

   rslt = fread( PosVal, sizeof(char), 1, fh_);
   if( rslt != 1 )
   {
      fclose( fh_);
      return( -2);
   }
   fclose( fh_);
   PosVal[1] = '\0';

   *Value = PosVal[0];
   return( rslt);
 }

 /*------------------------------------------------------------------------*/

void ETInitEnglish( void )
 {
   ElectionList  LinkE;
   CollegeList   LinkCol;
   PartyList     LinkP;
   CandidateList LinkCan;
   VOTE_TYPE_1 * Ptr1;
   int i;

   for(LinkE = Root.FirstElection ;
         LinkE != NULL ;
         LinkE = LinkE->Next )
   {
      for(i=0; i<num_EngText; i++)
         if( EngText[i].ElectInd == LinkE->Ind )
            break;

      LinkE->Long_Name[0] = EngText[i].Election;

      for(LinkCol = LinkE->FirstCollege;
            LinkCol != NULL;
            LinkCol = LinkCol->Next)
      {
         LinkCol->Long_Name[0] =
            EngText[i].College[LinkCol->Ind];
      }
   }
}

 /*------------------------------------------------------------------------*/

void ETDiskA( void )
 {
  FILE *fh_;
   int phase = 0;
   char Label[13];

  window(1,1,80,25);

  while(1)
  {
      switch( phase)
      {
         case 0:
            // SCREEN "INSERT MASTER or BACKUP1"
            clrscr();
            ScrBox(15,10,65,12, 0);
            ScrTextCentered("Insert MASTER or BACKUP1 disk",11);
            ScrTextCentered("Press any key when done...",15);
            ScrButtomBar(0,0,0,0);
            ScrHideCur();
            phase++;

         case 1:
            // WAIT KEYBOARD
            key_get();
            phase++;

         case 2:
            // CHECK FILE EXISTANCE
            clrscr();
            ScrTextCentered("One moment please...",11);
            ScrButtomBar(0,0,0,0);
            ScrHideCur();
            if( (fh_=fopen("a:\\config.sys","rb")) == NULL) // cannot open
            {
               phase=9;
               continue;
            }
            fclose(fh_);
            phase++;

         case 3:
            // CHECK IF MASTER OR BACKUP1
            if( Floppy_Get_Label( Label) == ABORTED)
            {
               phase=9;
               continue;
            }
            if( strcmp( Label, "MASTER") == 0 )
               Floppy_Current = Floppy_MASTER;
            else if( strcmp( Label, "BACKUP1") == 0 )
               Floppy_Current = Floppy_BACKUP;
            else
            {
               phase=9;
               continue;
            }
            phase++;

         case 4:
            // CHECK IF VOTE MACHINE CLOSED
            if( (fh_=fopen("a:\\b001","rb")) == NULL) // cannot open
            {
               phase=10;
               continue;
            }
            fclose(fh_);
            return;                                     // EXIT

         case 9:
            // SCREEN "INVALID DISK"
            clrscr();
            ScrTextCentered("DISK MISSING OR INVALID",11);
            ScrButtomBar(0,0,0,0);
            ScrHideCur();
            sleep(3);
            phase=0;
            continue;

         case 10:  // SCREEN "INVALID DISK"
            clrscr();
            ScrTextCentered("VOTE MACHINE NOT CLOSED",11);
            ScrButtomBar(0,0,0,0);
            ScrHideCur();
            sleep(3);
            phase=0;
            continue;

      } /* end switch */
  } /* end while(1) */
}

 /*------------------------------------------------------------------------*/

RET_CODE ETPassWord( void )
 {

   BYTE record[(MAX_IV_SIZE+AESKEYLEN)];   // holds iv + crypted session key
   AESKEY PassWord;

   window(1,1,80,25);
   clrscr();
   ScrTextCentered("ENTER PASSWORD",11);
   ScrButtomBar(0,0,0,0);

   strcpy( PassWord.data, "................");
   key_edit(13, 32, PassWord.data);

   clrscr();
   ScrTextCentered("One moment please...",11);
   ScrButtomBar(0,0,0,0);
   ScrHideCur();

   memcpy( mKey.data, PassWord.data, AESKEYLEN);

   // we need the sKey from the floppy
   if( Read_Key_On_Floppy( KEY_SESSION_FILE, record) == ABORTED)
   {
      // the disk is not correct, so we must reset the mKey.data
      memset( mKey.data, 0, AESKEYLEN+1);
      return ABORTED;
   }
   memcpy( iv.data, record, MAX_IV_SIZE);                      //set IV
   InitDec( N_METHODS);                                        //init AES decryption engine
   memcpy( sKey.data, &record[MAX_IV_SIZE], MAX_IV_SIZE);      //set encrypted key
   Decrypt( sKey.data, (AESKEYLEN / AESBLOCKLEN));             //decrypt SESSION key

   // :TODO: 20030516 avn - remove these later
   //printf( "Sessionkey [%s]", sKey.data);getch();

   Calculate_Internal_Keys();  // we still suppose that the given Password is
                             // correct, so we already compute the
                              // Internal_Keys with it rather than first
                               // computing a partial Internal_Keys struct
   return SUCCEEDED;
 }

 /*------------------------------------------------------------------------*/

RET_CODE ETPrepare( void )
{
   int retcode;

   if( Floppy_Rename_File( "A:\B021", "A:\B021.EXE") == ABORTED)
      return ABORTED;

   retcode = system( "A:\B021.EXE > NUL");

   if( Floppy_Rename_File( "A:\B021.EXE", "A:\B021") == ABORTED)
      return ABORTED;

   switch( retcode)
   {
      case 0:
         return SUCCEEDED;
      default:
         return ABORTED;
   }
}

 /*------------------------------------------------------------------------*/

void ETCard( void )
 {
  int phase = 0;
  int key = 0;
  unsigned char Buffer[200];
  unsigned char Card_Buffer[200];
  int i,j,k,Flag;
  int ElectCounter,ElectMax,BufferPointer;
  char ButtomBuffer[80],char3[3];
  int NbrOfEffectives, NbrOfSuppliants, NbrOfElections;
  int DisplayLine;
  char char90[90];
  int VirginCard;

  ElectionList  LinkE;
  CollegeList   LinkCol;
  PartyList     LinkP;
  CandidateList LinkCan;
  int BufferCollege;
  VOTE_TYPE_1 * Ptr1;

  // INITIALIZE IMORTANT PARAMETERS
  memset(EmptyString,'\0',sizeof(EmptyString));
  Intern_Reader = Open_Command( MCT145_1A, COM2);

  while(1)
  {
    switch( phase )
    {
      case 0:
        // CLEAR SCREEN AND PUT MESSAGE 'INSERT CARD NOW'
        window(1,1,80,25);
        clrscr();
        ScrHideCur();
        ScrBox(10,10,70,14,0);
        highvideo();
        ScrTextCentered("I N S E R T    C A R D",12);

        ScrButtomBar(0,0,0,C_F10);
        phase++;

      case 1:
        // ACCEPT CARD IN INTERNAL READER, OR F10 TO EXIT
        memset(Card_Buffer,'\0',sizeof(Card_Buffer));
        memset(Buffer,'\0',sizeof(Buffer));
        if( Reader_Card_Position(Intern_Reader) == HOME )
        {
             Reader_Physical_Read_Card( Intern_Reader,Card_Buffer);
             phase++;
             continue;
        }
        for(;;)
        {
         if(Reader_Formatted_Card(Intern_Reader) == HOME)
         {
            phase++;
            break;
         }
         if( kbhit() )
         {
            key = key_get();
            if(key == C_F10)
            {
               phase = 901;
               break;
            }
         }
        }
        continue;
      case 2:
        // READ CARD
        Reader_Read_Card(Intern_Reader,Card_Buffer);
        clrscr();
        ScrButtomBar(C_ESC,0,0,C_F10);
        phase++;

      case 3:
        // CHECK CARD VALIDITY, CONVERT CARD BUFFER TO VOTE BUFFER
        ButtomBuffer[0] = '\0';
        if(strlen(Card_Buffer) <= 0)
        {
          phase = 800;
          continue;
        }
        if( Split_Card_Buffer(Card_Buffer, &Flag, Buffer) == ABORTED )
        {
          phase = 800;
          continue;
        }
        if( Flag == 0 )
        {
          strcpy(ButtomBuffer,"THIS CARD HAS NOT BEEN INSERTED IN THE VOTING MACHINE");
          VirginCard=1;
        }
        else
          VirginCard=0;
        ScrTextCentered(ButtomBuffer,23);
        phase++;

      case 4:
        // FILL LOCAL_CARD_INFO[ElectCounter]
        BufferPointer=0;
        CurrentTypeElector = Card_Buffer[PositionCardTypeElector + 8] - 0x30 + 1;
        for( ElectCounter=0,
             LinkE = Root.FirstElection ;
             LinkE != NULL ;
             LinkE = LinkE->Next,
             ElectCounter++ )
        {
          if( Skip_Election( LinkE->Ind, CurrentTypeElector ) == TRUE )
             Local_Card_Info[ElectCounter].VotedFor = FALSE;
          else
             Local_Card_Info[ElectCounter].VotedFor = TRUE;

          Local_Card_Info[ElectCounter].Election =
            LinkE->Long_Name[0];

          memset(char3,'\0',sizeof(char3));
          memcpy(char3,&Buffer[BufferPointer],1);
          BufferCollege = atoi(char3);
          memset(char3,'\0',sizeof(char3));
          memcpy(char3,&Buffer[BufferPointer+1],2);
          Local_Card_Info[ElectCounter].Party = atoi(char3);
          memset(char3,'\0',sizeof(char3));
          memcpy(char3,&Buffer[BufferPointer+3],1);
          Local_Card_Info[ElectCounter].HeadVote = atoi(char3);

          if(VirginCard == 0)
          {
            for( LinkCol = LinkE->FirstCollege;
                 LinkCol != NULL;
                 LinkCol = LinkCol->Next)
            {
              if(LinkCol->Ind == BufferCollege)
                break;
            }
            if(LinkE->NbrOfColleges == 1)
              Local_Card_Info[ElectCounter].College = EmptyString;
            else
              Local_Card_Info[ElectCounter].College =
                LinkCol->Long_Name[0];
          }
          else
            Local_Card_Info[ElectCounter].College = EmptyString;

          memset( Local_Card_Info[ElectCounter].Effectives,'0',
                  sizeof(Local_Card_Info[ElectCounter].Effectives) );
          memset( Local_Card_Info[ElectCounter].Suppliants,'0',
                  sizeof(Local_Card_Info[ElectCounter].Suppliants) );

          switch(Card_Info_Per_Election[ElectCounter].Type_Election)
          {
            case 0: //SINGLE
              if(VirginCard == 0)
              {
                memset(char3,'\0',sizeof(char3));
                memcpy(char3,&Buffer[BufferPointer+4],2);
                if( strcmp(char3,"00") != 0 )
                    Local_Card_Info[ElectCounter].Effectives[atoi(char3)-1]
                      = '1';
                memset(char3,'\0',sizeof(char3));
                memcpy(char3,&Buffer[BufferPointer+6],2);
                if( strcmp(char3,"00") != 0 )
                    Local_Card_Info[ElectCounter].Suppliants[atoi(char3)-1]
                      = '1';
              }
              BufferPointer+=8;
              break;
            case 1: //MULTIPLE
              if(VirginCard == 0)
              {
                for(LinkP = LinkCol->FirstParty ;
                    LinkP != NULL;
                    LinkP = LinkP->Next)
                {
                  if(LinkP->Ind == Local_Card_Info[ElectCounter].Party)
                    break;
                }
                LinkCan = LinkP->FirstEffective;
                for( i=0; LinkCan != NULL; ++i, LinkCan = LinkCan->Next);
                NbrOfEffectives = i;
                LinkCan = LinkP->FirstSuppliant;
                for(i=0 ; LinkCan != NULL ; ++i, LinkCan = LinkCan->Next);
                NbrOfSuppliants = i;

                Ptr1 = (VOTE_TYPE_1 *) &Buffer[BufferPointer];
                memset(char90,'0',sizeof(char90));
                for( i=0; i < ((MAX_CANDIDATES+2)/3); i++)
                {
                  if( Ptr1 -> Candidate [i].bit1)
                    char90[i*3] = '1';
                  if( Ptr1 -> Candidate [i].bit2)
                    char90[i*3+1] = '1';
                  if( Ptr1 -> Candidate [i].bit3)
                    char90[i*3+2] = '1';
                }
                memcpy(Local_Card_Info[ElectCounter].Effectives,
                       char90,
                       NbrOfEffectives);
                memcpy(Local_Card_Info[ElectCounter].Suppliants,
                       &char90[NbrOfEffectives],
                       NbrOfSuppliants);
              }
              BufferPointer+= ((MAX_CANDIDATES+2) / 3) + 4;
              break;
          } /* end switch */
        }
        NbrOfElections=ElectCounter;
        ElectCounter=0;
        phase++;

      case 5:
        // DISPLAY CARD CONTENTS
        window(1,1,80,22);
        clrscr();
        ScrHideCur();
        DisplayLine=1;
        for(i=0; i<ElectCounter; i++)
        {
          ShowMinimized(i,&DisplayLine);
        }
        ShowMaximized(ElectCounter,&DisplayLine);
        for(i=ElectCounter+1; i<NbrOfElections; i++)
        {
          ShowMinimized(i,&DisplayLine);
        }
        ScrButtomBar(C_ESC,C_DOWN,C_UP,C_F10);
        phase ++;
        phase ++;

      case 7:
        key = key_get();
        switch( key )
        {
         case C_DOWN:
         case C_N:
         case C_n:
         case C_PAGEDOWN:
           ElectCounter++;
           if( ElectCounter==NbrOfElections )
             ElectCounter=0;
           phase=5;
           break;
         case C_UP:
         case C_P:
         case C_p:
         case C_PAGEUP:
           ElectCounter--;
           if( ElectCounter<0 )
             ElectCounter=NbrOfElections-1;
           phase=5;
           break;
         case C_ESC:
            phase++;
            break;
         case C_F10:
            Reader_Eject_Card( Intern_Reader);
            phase = 900;
            break;
        }
        continue;

      case 8:
        Reader_Eject_Card( Intern_Reader);

      case 9:
        // CLEAR SCREEN AND PUT MESSAGE 'TAKE CARD'
        window(1,1,80,22);
        clrscr();
        ScrHideCur();
        highvideo();
        ScrTextCentered("T A K E    C A R D",12);
        ScrButtomBar(0,0,0,C_F10);
        phase++;

      case 10:
        for(;;)
        {
         if(Reader_Card_Position(Intern_Reader) == NO_CARD)
         {
            phase=0;
            break;
         }
         if( kbhit() )
         {
            key = key_get();
            if(key == C_F10)
            {
               phase = 900;
               break;
            }
         }
        }
        continue;

      case 800:
        strcpy(ButtomBuffer,"INVALID CARD");
        ScrTextCentered(ButtomBuffer,23);
        phase = 8;
        continue;

      case 900:
        // CLEAR SCREEN AND PUT MESSAGE 'TAKE CARD'
        window(1,1,80,25);
        clrscr();
        ScrHideCur();
        highvideo();
        ScrTextCentered("T A K E    C A R D",12);
        ScrButtomBar(0,0,0,0);
        for(;;)
        {
         if(Reader_Card_Position(Intern_Reader) == NO_CARD)
         {
            phase=901;
            break;
         }
        }
        phase++;

      case 901:
            ScrExit("END OF TEST PROGRAM");
    } /* end switch() */
  } /* end while(1) */
 }
