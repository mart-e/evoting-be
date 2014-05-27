.head 0 +  Application Description: CTD2000 - Mailslots Library

Copyright (C) 2000 Ice Tea Group, LLC.
Author: Joe Meyer
.head 1 -  Outline Version - 4.0.31
.head 1 +  Design-time Settings
.data VIEWINFO
0000: 6F00000001000000 FFFF01000D004347 5458566965775374 6174650400010000
0020: 0000000000A70000 002C000000020000 0003000000FFFFFF FFFFFFFFFFFCFFFF
0040: FFE9FFFFFFFFFFFF FF000000007C0200 004D010000010000 0001000000010000
0060: 000F4170706C6963 6174696F6E497465 6D00000000
.enddata
.head 2 -  Outline Window State: Normal
.head 2 +  Outline Window Location and Size
.data VIEWINFO
0000: 6600040003002D00 0000000000000000 0000B71E5D0E0500 1D00FFFF4D61696E
0020: 0000000000000000 0000000000000000 0000003B00010000 00000000000000E9
0040: 1E800A00008600FF FF496E7465726E61 6C2046756E637469 6F6E730000000000
0060: 0000000000000000 0000000000003200 0100000000000000 0000E91E800A0000
0080: DF00FFFF56617269 61626C6573000000 0000000000000000 0000000000000000
00A0: 3000010000000000 00000000F51E100D 0000F400FFFF436C 6173736573000000
00C0: 0000000000000000 0000000000000000
.enddata
.data VIEWSIZE
0000: D000
.enddata
.head 3 -  Left: -0.013"
.head 3 -  Top: 0.0"
.head 3 -  Width:  8.013"
.head 3 -  Height: 4.969"
.head 2 +  Options Box Location
.data VIEWINFO
0000: D4180909B80B1A00
.enddata
.data VIEWSIZE
0000: 0800
.enddata
.head 3 -  Visible? Yes
.head 3 -  Left: 4.15"
.head 3 -  Top: 1.885"
.head 3 -  Width:  3.8"
.head 3 -  Height: 2.073"
.head 2 +  Class Editor Location
.head 3 -  Visible? No
.head 3 -  Left: 0.575"
.head 3 -  Top: 0.094"
.head 3 -  Width:  5.063"
.head 3 -  Height: 2.719"
.head 2 +  Tool Palette Location
.head 3 -  Visible? No
.head 3 -  Left: 6.388"
.head 3 -  Top: 0.729"
.head 2 -  Fully Qualified External References? Yes
.head 2 -  Reject Multiple Window Instances? No
.head 2 -  Enable Runtime Checks Of External References? Yes
.head 2 -  Use Release 4.0 Scope Rules? No
.head 1 +  Libraries
.head 2 +  File Include: Win32API.apl
.head 3 -  ! The Windows API functions are located in a separate file.
This way it's easier for you to include the mail slots functions
into your libraries. If you already have some of the Win32API.apl
functions declared in your project, simply copy the missing
functions into your own Windows API library and include yours
instead of ours.
.head 1 +  Global Declarations
.head 2 +  Window Defaults
.head 3 +  Tool Bar
.head 4 -  Display Style? Etched
.head 4 -  Font Name: MS Sans Serif
.head 4 -  Font Size: 8
.head 4 -  Font Enhancement: System Default
.head 4 -  Text Color: System Default
.head 4 -  Background Color: System Default
.head 3 +  Form Window
.head 4 -  Display Style? Etched
.head 4 -  Font Name: MS Sans Serif
.head 4 -  Font Size: 8
.head 4 -  Font Enhancement: System Default
.head 4 -  Text Color: System Default
.head 4 -  Background Color: System Default
.head 3 +  Dialog Box
.head 4 -  Display Style? Etched
.head 4 -  Font Name: MS Sans Serif
.head 4 -  Font Size: 8
.head 4 -  Font Enhancement: System Default
.head 4 -  Text Color: System Default
.head 4 -  Background Color: System Default
.head 3 +  Top Level Table Window
.head 4 -  Font Name: MS Sans Serif
.head 4 -  Font Size: 8
.head 4 -  Font Enhancement: System Default
.head 4 -  Text Color: System Default
.head 4 -  Background Color: System Default
.head 3 +  Data Field
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Multiline Field
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Spin Field
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Background Text
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Pushbutton
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 3 +  Radio Button
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Check Box
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Option Button
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 3 +  Group Box
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Child Table Window
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  List Box
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Combo Box
.head 4 -  Font Name: Use Parent
.head 4 -  Font Size: Use Parent
.head 4 -  Font Enhancement: Use Parent
.head 4 -  Text Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 3 +  Line
.head 4 -  Line Color: Use Parent
.head 3 +  Frame
.head 4 -  Border Color: Use Parent
.head 4 -  Background Color: 3D Face Color
.head 3 +  Picture
.head 4 -  Border Color: Use Parent
.head 4 -  Background Color: Use Parent
.head 2 -  Formats
.head 2 +  External Functions
.head 2 +  Constants
.data CCDATA
0000: 3000000000000000 0000000000000000 00000000
.enddata
.data CCSIZE
0000: 1400
.enddata
.head 3 +  System
.head 3 +  User
.head 3 -  Enumerations
.head 2 -  Resources
.head 2 +  Variables
.head 3 -  Number: gnMailSlotTimer
.head 3 -  Number: gnMailSlotDelayTimer
.head 3 -  Window Handle: gwhMailSlot
.head 3 -  Number: ghMailSlotHandle
.head 3 -  Long String: gsaMsg[*]
.head 3 -  Number: nMailSlotMin
.head 3 -  Number: nMailSlotMax
.head 2 +  Internal Functions
.head 3 -  ! Mail slot functions:
.head 3 +  Function: MailslotSend
.head 4 -  Description:
.head 4 +  Returns
.head 5 +  Boolean:
.head 6 -  ! returns FALSE on error
.head 4 +  Parameters
.head 5 +  String: p_sMachineName
.head 6 -  ! The name of the machine where the message is to be sent to.
Use "." for the local machine
.head 5 +  String: p_sMailslotName
.head 6 -  ! The name of the mailslot to send to
.head 5 +  Long String: p_sMessage
.head 6 -  ! The text to be sent
.head 4 -  Static Variables
.head 4 +  Local variables
.head 5 -  String: sMachineName
.head 5 -  String: sSlotname
.head 5 -  Number: nLen
.head 5 -  Number: hSlotHandle
.head 5 -  Number: nBytesWritten
.head 5 -  Boolean: bOk
.head 4 +  Actions
.head 5 -  Set bOk = TRUE
.head 5 -  ! check parameters
.head 5 +  If NOT p_sMailslotName OR NOT p_sMessage
.head 6 -  Return FALSE
.head 5 +  If NOT p_sMachineName
.head 6 -  Set p_sMachineName = "."
.head 5 -  ! check the machine name
.head 5 -  Set nLen = 32
.head 5 -  Call SalStrSetBufferLength( sMachineName, nLen )
.head 5 +  If not GetComputerNameA( sMachineName, nLen )
.head 6 -  Return FALSE
.head 5 +  If SalStrUpperX( sMachineName ) = SalStrUpperX( p_sMachineName )
.head 6 -  Set p_sMachineName = "."
.head 5 -  ! build the complete mail slot name
.head 5 -  Set sSlotname = "\\\\" || p_sMachineName || "\\mailslot\\" || p_sMailslotName
.head 5 -  ! open the mail slot
.head 5 -  Set hSlotHandle = CreateFileA( sSlotname, 0x40000000, 1, 0, 3, 0x00000080, 0 )
.head 5 +  If hSlotHandle = -1
.head 6 -  Return FALSE
.head 5 -  ! write to the mail slot
.head 5 -  Set bOk = bOk AND WriteFile( hSlotHandle, p_sMessage, SalStrLength( p_sMessage ) + 1, nBytesWritten, 0 )
.head 5 -  ! close mail slot
.head 5 -  Set bOk = bOk AND CloseHandle( hSlotHandle )
.head 5 -  ! Done.
.head 5 -  Return bOk
.head 3 +  Function: MailslotCreate
.head 4 -  Description: Creates a mail slot for receiving messages
.head 4 +  Returns
.head 5 +  Boolean:
.head 6 -  ! Returns TRUE on success
.head 4 +  Parameters
.head 5 +  Receive Number: r_hSlotHandle
.head 6 -  ! On success the handle to the mail slot is returned.
On error this value is -1
.head 5 +  String: p_sMailslotName
.head 6 -  ! The name of the mailslot to listen on
.head 4 -  Static Variables
.head 4 +  Local variables
.head 5 -  String: sSlotname
.head 4 +  Actions
.head 5 -  ! check parameters
.head 5 +  If NOT p_sMailslotName
.head 6 -  Return FALSE
.head 5 -  ! build the complete mail slot name
.head 5 -  Set sSlotname = "\\\\.\\mailslot\\" || p_sMailslotName
.head 5 -  ! create the mail slot
.head 5 -  Set r_hSlotHandle = CreateMailslotA( sSlotname, 0, 0, 0 )
.head 5 -  Return r_hSlotHandle != -1
.head 3 +  Function: MailslotDestroy
.head 4 -  Description: Destroys a mail slot that was opened using MailslotCreate()
.head 4 -  Returns
.head 4 +  Parameters
.head 5 +  Receive Number: r_nSlotHandle
.head 6 -  ! Handle of the mail slot to be closed.
The value is set to -1 after completion
.head 4 -  Static Variables
.head 4 -  Local variables
.head 4 +  Actions
.head 5 +  If r_nSlotHandle != -1
.head 6 +  If CloseHandle( r_nSlotHandle )
.head 7 -  Set r_nSlotHandle = -1
.head 3 +  Function: MailslotRead
.head 4 -  Description:
.head 4 +  Returns
.head 5 +  Boolean:
.head 6 -  ! Returns TRUE on success
.head 4 +  Parameters
.head 5 +  Number: p_nSlotHandle
.head 6 -  ! Handle of the mail slot to read
.head 5 +  Receive Long String: r_saMsg [*]
.head 6 -  ! the messages that were read.
If there were no messages, the array is empty
.head 4 -  Static Variables
.head 4 +  Local variables
.head 5 -  Number: nNextSize
.head 5 -  Number: nNumMsg
.head 5 -  Number: nBytesRead
.head 5 -  Number: nCnt
.head 4 +  Actions
.head 5 -  ! clear the return array
.head 5 -  Call SalArraySetUpperBound( r_saMsg, 1, -1 )
.head 5 -  ! check if handle is valid
.head 5 +  If p_nSlotHandle = -1
.head 6 -  Return FALSE
.head 5 -  ! loop until all messages are retrieved
.head 5 +  While TRUE
.head 6 -  ! get information about next message
.head 6 +  If not GetMailslotInfo( p_nSlotHandle, 0, nNextSize, nNumMsg, 0 )
.head 7 -  Return FALSE
.head 6 -  ! if there's no message then let's get out of here
.head 6 +  If (nNextSize < 0) OR (nNumMsg = 0)
.head 7 -  Break
.head 6 -  ! there was a message, so receive it
.head 6 -  Call SalStrSetBufferLength( r_saMsg [nCnt], nNextSize )
.head 6 +  If not ReadFile( p_nSlotHandle, r_saMsg [nCnt], nNextSize, nBytesRead, 0 )
.head 7 -  Return FALSE
.head 6 -  ! next message goes into next array element
.head 6 -  Set nCnt = nCnt + 1
.head 5 -  ! done
.head 5 -  Return TRUE
.head 2 -  Named Menus
.head 2 -  Class Definitions
.head 2 +  Default Classes
.head 3 -  MDI Window: cBaseMDI
.head 3 -  Form Window:
.head 3 -  Dialog Box:
.head 3 -  Table Window:
.head 3 -  Quest Window:
.head 3 -  Data Field:
.head 3 -  Spin Field:
.head 3 -  Multiline Field:
.head 3 -  Pushbutton:
.head 3 -  Radio Button:
.head 3 -  Option Button:
.head 3 -  Check Box:
.head 3 -  Child Table:
.head 3 -  Quest Child Window: cQuickDatabase
.head 3 -  List Box:
.head 3 -  Combo Box:
.head 3 -  Picture:
.head 3 -  Vertical Scroll Bar:
.head 3 -  Horizontal Scroll Bar:
.head 3 -  Column:
.head 3 -  Background Text:
.head 3 -  Group Box:
.head 3 -  Line:
.head 3 -  Frame:
.head 3 -  Custom Control:
.head 3 -  ActiveX:
.head 2 -  Application Actions
