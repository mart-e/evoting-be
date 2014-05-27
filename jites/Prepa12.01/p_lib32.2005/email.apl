.head 0 +  Application Description: EMAIL.APL

Interface to send Emails (with parameters) directly or with the user interface (the email-client will come up).
or send it in the background with the function EmailSendBackground().

Programming: J�rg Ellinghaus
Email: ELLI@ONLINE-CLUB.DE

This software is for demonstration purposes only.
It may be distributed freely as long as it is unchanged.
The author guarantees nothing and is not responsible in any way.
.head 1 -  Outline Version - 4.0.31
.head 1 +  Design-time Settings
.data VIEWINFO
0000: 6F00000001000000 FFFF01000D004347 5458566965775374 6174650400800000
0020: 0000000000C40000 002C000000020000 0003000000FFFFFF FFFFFFFFFFFCFFFF
0040: FFE9FFFFFFFFFFFF FF000000007C0200 004D010000010000 0000000000010000
0060: 000F4170706C6963 6174696F6E497465 6D02000000075769 6E646F777308646C
0080: 67454D61696C
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
.head 2 -  File Include: cgmail.apl
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
.head 2 +  Formats
.head 3 -  Number: 0'%'
.head 3 -  Number: #0
.head 3 -  Number: ###000
.head 3 -  Number: ###000;'($'###000')'
.head 3 -  Date/Time: hh:mm:ss AMPM
.head 3 -  Date/Time: M/d/yy
.head 3 -  Date/Time: MM-dd-yy
.head 3 -  Date/Time: dd-MMM-yyyy
.head 3 -  Date/Time: MMM d, yyyy
.head 3 -  Date/Time: MMM d, yyyy hh:mm AMPM
.head 3 -  Date/Time: MMMM d, yyyy hh:mm AMPM
.head 2 +  External Functions
.head 2 +  Constants
.data CCDATA
0000: 3000000000000000 0000000000000000 00000000
.enddata
.data CCSIZE
0000: 1400
.enddata
.head 3 -  System
.head 3 +  User
.head 3 -  Enumerations
.head 2 -  Resources
.head 2 -  Variables
.head 2 +  Internal Functions
.head 3 +  Function: EmailSendBackground
.head 4 -  Description:
.head 4 +  Returns
.head 5 -  Boolean:
.head 4 +  Parameters
.head 5 -  String: p_sEMail_To
.head 5 -  String: p_sEMail_CC
.head 5 -  String: p_sEMail_BC
.head 5 -  String: p_sEMail_Subject
.head 5 -  Long String: p_lsEMail_Text
.head 5 -  Long String: p_lsEMail_Files
.head 5 -  Boolean: p_bDirect
.head 4 -  Static Variables
.head 4 +  Local variables
.head 5 -  FunctionalVar: EMail
.head 6 -  Class: cQOEmail
.head 4 +  Actions
.head 5 -  Call SalWaitCursor( TRUE )
.head 5 -  !
.head 5 +  If Not EMail.EmailConnect('','','', EMAIL_SYS_MSMAIL)
.head 6 -  Call SalWaitCursor( 0 )
.head 6 -  Call SalMessageBox( 'The system could not connect to the Email-Client!', 'EMail', MB_IconExclamation )
.head 6 -  Return FALSE
.head 5 -  !
.head 5 -  Call EMail.EmailSetDebugMode( 0 )
.head 5 -  !
.head 5 +  If p_bDirect
.head 6 -  Call EMail.EmailSendMessage( 0, p_sEMail_Subject, p_lsEMail_Text, p_sEMail_To, p_sEMail_CC, p_sEMail_BC, p_lsEMail_Files, 0, 0 ) 
.head 5 +  Else
.head 6 -  Call EMail.EmailSendMessage( 1, p_sEMail_Subject, p_lsEMail_Text, p_sEMail_To, p_sEMail_CC, p_sEMail_BC, p_lsEMail_Files, 0, 0 ) 
.head 5 -  !
.head 5 -  Call EMail.EmailDisconnect()
.head 5 -  !
.head 5 -  Call SalWaitCursor( 0 )
.head 5 -  !
.head 5 -  Return TRUE
.head 2 -  Named Menus
.head 2 +  Class Definitions
.data RESOURCE 0 0 1 3387185589
0000: F300000089000000 0000000000000000 0200000100FFFF01 00160000436C6173
0020: 73566172004F7574 6C696E6552006567 496E666F81003C00 00086300514F456D
0040: 61696CC080000000 0600000087190000 00F2010000FFDF19 AC00190001FD00FF
0060: 0732000000196B00 0100FFFFC14B0000 0019DA000100FF7F 64B00000001900F6
0080: 0100FF1F7D000000 0004000000010200 FC00003F8001F800 000007
.enddata
.head 2 +  Default Classes
.head 3 -  MDI Window: cBaseMDI
.head 3 -  Form Window:
.head 3 -  Dialog Box:
.head 3 -  Table Window:
.head 3 -  Quest Window:
.head 3 -  Data Field: cGenericDataField
.head 3 -  Spin Field:
.head 3 -  Multiline Field: cGenericMultiline
.head 3 -  Pushbutton: cGenericPushbutton
.head 3 -  Radio Button:
.head 3 -  Option Button:
.head 3 -  ActiveX:
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
.head 2 +  Application Actions
.head 3 +  On SAM_AppStartup
.head 4 -  Call SalModalDialog( dlgEMail, hWndNULL, '', '', '', '', '', '' )
.head 4 -  !
.head 4 -  Call EmailSendBackground( 'elli@online-club.de', '', '', 'EMAIL.APL Test', 'This is the function EmailSendBackground(). Thank you.', '', TRUE )
.head 1 +  Dialog Box: dlgEMail
.head 2 -  Class:
.head 2 -  Property Template:
.head 2 -  Class DLL Name:
.head 2 -  Title: EMail
.head 2 -  Accessories Enabled? Yes
.head 2 -  Visible? Yes
.head 2 -  Display Settings
.head 3 -  Display Style? Default
.head 3 -  Visible at Design time? Yes
.head 3 -  Type of Dialog: Modal
.head 3 -  Allow Dock to Parent? No
.head 3 -  Docking Orientation: All
.head 3 -  Window Location and Size
.head 4 -  Left: 0.2"
.head 4 -  Top: 0.167"
.head 4 -  Width:  9.967"
.head 4 -  Width Editable? Yes
.head 4 -  Height: 5.845"
.head 4 -  Height Editable? Yes
.head 3 -  Absolute Screen Location? Yes
.head 3 -  Font Name: Default
.head 3 -  Font Size: Default
.head 3 -  Font Enhancement: Default
.head 3 -  Text Color: Default
.head 3 -  Background Color: Default
.head 2 -  Description:
.head 2 +  Tool Bar
.head 3 -  Display Settings
.head 4 -  Display Style? Default
.head 4 -  Location? Top
.head 4 -  Visible? No
.head 4 -  Size: Default
.head 4 -  Size Editable? Yes
.head 4 -  Docking Toolbar? No
.head 4 -  Toolbar Docking Orientation: Top | Bottom
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 3 -  Contents
.head 2 +  Contents
.head 3 -  Background Text: To:
.head 4 -  Resource Id: 9685
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.183"
.head 5 -  Top: 0.179"
.head 5 -  Width:  0.5"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.167"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Justify: Left
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 3 +  Pushbutton: pbAn
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Title:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.883"
.head 5 -  Top: 0.155"
.head 5 -  Width:  0.4"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.25"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Keyboard Accelerator: (none)
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Picture File Name:
.head 4 -  Picture Transparent Color: None
.head 4 -  Image Style: Single
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 4 -  Button Appearance: Standard
.head 4 +  Message Actions
.head 5 +  On SAM_Click
.head 6 -  Call Adresse( 1 )
.head 3 +  Data Field: dfs_To
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Data
.head 5 -  Maximum Data Length: 2000
.head 5 -  Data Type: Long String
.head 5 -  Editable? Yes
.head 4 -  Display Settings
.head 5 -  Window Location and Size
.head 6 -  Left: 1.283"
.head 6 -  Top: 0.155"
.head 6 -  Width:  8.4"
.head 6 -  Width Editable? Yes
.head 6 -  Height: 0.25"
.head 6 -  Height Editable? Yes
.head 5 -  Visible? Yes
.head 5 -  Border? Yes
.head 5 -  Justify: Left
.head 5 -  Format: Unformatted
.head 5 -  Country: Default
.head 5 -  Font Name: Default
.head 5 -  Font Size: Default
.head 5 -  Font Enhancement: Default
.head 5 -  Text Color: Default
.head 5 -  Background Color: Default
.head 5 -  Input Mask: Unformatted
.head 4 -  Message Actions
.head 3 -  Background Text: CC:
.head 4 -  Resource Id: 9686
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.183"
.head 5 -  Top: 0.429"
.head 5 -  Width:  0.5"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.167"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Justify: Left
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 3 +  Pushbutton: pbCC
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Title:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.883"
.head 5 -  Top: 0.405"
.head 5 -  Width:  0.4"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.25"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Keyboard Accelerator: (none)
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Picture File Name:
.head 4 -  Picture Transparent Color: None
.head 4 -  Image Style: Single
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 4 -  Button Appearance: Standard
.head 4 +  Message Actions
.head 5 +  On SAM_Click
.head 6 -  Call Adresse( 2 )
.head 3 +  Data Field: dfs_CC
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Data
.head 5 -  Maximum Data Length: 2000
.head 5 -  Data Type: Long String
.head 5 -  Editable? Yes
.head 4 -  Display Settings
.head 5 -  Window Location and Size
.head 6 -  Left: 1.283"
.head 6 -  Top: 0.405"
.head 6 -  Width:  8.4"
.head 6 -  Width Editable? Yes
.head 6 -  Height: 0.25"
.head 6 -  Height Editable? Yes
.head 5 -  Visible? Yes
.head 5 -  Border? Yes
.head 5 -  Justify: Left
.head 5 -  Format: Unformatted
.head 5 -  Country: Default
.head 5 -  Font Name: Default
.head 5 -  Font Size: Default
.head 5 -  Font Enhancement: Default
.head 5 -  Text Color: Default
.head 5 -  Background Color: Default
.head 5 -  Input Mask: Unformatted
.head 4 -  Message Actions
.head 3 -  Background Text: BC:
.head 4 -  Resource Id: 28319
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.183"
.head 5 -  Top: 0.679"
.head 5 -  Width:  0.5"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.167"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Justify: Left
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 3 +  Pushbutton: pbBC
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Title:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.883"
.head 5 -  Top: 0.655"
.head 5 -  Width:  0.4"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.25"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Keyboard Accelerator: (none)
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Picture File Name:
.head 4 -  Picture Transparent Color: None
.head 4 -  Image Style: Single
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 4 -  Button Appearance: Standard
.head 4 +  Message Actions
.head 5 +  On SAM_Click
.head 6 -  Call Adresse( 3 )
.head 3 +  Data Field: dfs_BC
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Data
.head 5 -  Maximum Data Length: 2000
.head 5 -  Data Type: Long String
.head 5 -  Editable? Yes
.head 4 -  Display Settings
.head 5 -  Window Location and Size
.head 6 -  Left: 1.283"
.head 6 -  Top: 0.655"
.head 6 -  Width:  8.4"
.head 6 -  Width Editable? Yes
.head 6 -  Height: 0.25"
.head 6 -  Height Editable? Yes
.head 5 -  Visible? Yes
.head 5 -  Border? Yes
.head 5 -  Justify: Left
.head 5 -  Format: Unformatted
.head 5 -  Country: Default
.head 5 -  Font Name: Default
.head 5 -  Font Size: Default
.head 5 -  Font Enhancement: Default
.head 5 -  Text Color: Default
.head 5 -  Background Color: Default
.head 5 -  Input Mask: Unformatted
.head 4 -  Message Actions
.head 3 -  Background Text: Subject:
.head 4 -  Resource Id: 9687
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.183"
.head 5 -  Top: 0.929"
.head 5 -  Width:  0.9"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.167"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Justify: Left
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 3 +  Data Field: dfs_Subject
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Data
.head 5 -  Maximum Data Length: 250
.head 5 -  Data Type: String
.head 5 -  Editable? Yes
.head 4 -  Display Settings
.head 5 -  Window Location and Size
.head 6 -  Left: 1.283"
.head 6 -  Top: 0.905"
.head 6 -  Width:  8.4"
.head 6 -  Width Editable? Yes
.head 6 -  Height: 0.25"
.head 6 -  Height Editable? Yes
.head 5 -  Visible? Yes
.head 5 -  Border? Yes
.head 5 -  Justify: Left
.head 5 -  Format: Unformatted
.head 5 -  Country: Default
.head 5 -  Font Name: Default
.head 5 -  Font Size: Default
.head 5 -  Font Enhancement: Default
.head 5 -  Text Color: Default
.head 5 -  Background Color: Default
.head 5 -  Input Mask: Unformatted
.head 4 -  Message Actions
.head 3 -  Background Text: Text:
.head 4 -  Resource Id: 9688
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.183"
.head 5 -  Top: 1.429"
.head 5 -  Width:  0.9"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.167"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Justify: Left
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 3 +  Multiline Field: ml_Text
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Data
.head 5 -  Maximum Data Length: Default
.head 5 -  String Type: String
.head 5 -  Editable? Yes
.head 4 -  Display Settings
.head 5 -  Border? Yes
.head 5 -  Word Wrap? No
.head 5 -  Vertical Scroll? Yes
.head 5 -  Window Location and Size
.head 6 -  Left: 1.283"
.head 6 -  Top: 1.405"
.head 6 -  Width:  8.4"
.head 6 -  Width Editable? Yes
.head 6 -  Height: 3.0"
.head 6 -  Height Editable? Yes
.head 5 -  Visible? Yes
.head 5 -  Font Name: Default
.head 5 -  Font Size: Default
.head 5 -  Font Enhancement: Default
.head 5 -  Text Color: Default
.head 5 -  Background Color: Default
.head 4 -  Message Actions
.head 3 -  Background Text: Files:
.head 4 -  Resource Id: 9689
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.183"
.head 5 -  Top: 4.679"
.head 5 -  Width:  0.7"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.167"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Justify: Left
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 3 +  Pushbutton: pbDatei
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Title:
.head 4 -  Window Location and Size
.head 5 -  Left: 0.883"
.head 5 -  Top: 4.655"
.head 5 -  Width:  0.4"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.25"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Keyboard Accelerator: (none)
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Picture File Name:
.head 4 -  Picture Transparent Color: None
.head 4 -  Image Style: Single
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 4 -  Button Appearance: Standard
.head 4 +  Message Actions
.head 5 +  On SAM_Click
.head 6 -  Call Attachment()
.head 3 +  Data Field: dfs_Files
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Data
.head 5 -  Maximum Data Length: 2000
.head 5 -  Data Type: Long String
.head 5 -  Editable? Yes
.head 4 -  Display Settings
.head 5 -  Window Location and Size
.head 6 -  Left: 1.283"
.head 6 -  Top: 4.655"
.head 6 -  Width:  8.4"
.head 6 -  Width Editable? Yes
.head 6 -  Height: 0.25"
.head 6 -  Height Editable? Yes
.head 5 -  Visible? Yes
.head 5 -  Border? Yes
.head 5 -  Justify: Left
.head 5 -  Format: Unformatted
.head 5 -  Country: Default
.head 5 -  Font Name: Default
.head 5 -  Font Size: Default
.head 5 -  Font Enhancement: Default
.head 5 -  Text Color: Default
.head 5 -  Background Color: Default
.head 5 -  Input Mask: Unformatted
.head 4 -  Message Actions
.head 3 +  Pushbutton: pbSendUI
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Title: &Send UI
.head 4 -  Window Location and Size
.head 5 -  Left: 1.283"
.head 5 -  Top: 5.155"
.head 5 -  Width:  1.2"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.298"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Keyboard Accelerator: (none)
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Picture File Name:
.head 4 -  Picture Transparent Color: None
.head 4 -  Image Style: Single
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 4 -  Button Appearance: Standard
.head 4 +  Message Actions
.head 5 +  On SAM_Click
.head 6 -  Call SendMail( FALSE )
.head 3 +  Pushbutton: pbSendDirect
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Title: Send &direct
.head 4 -  Window Location and Size
.head 5 -  Left: 2.483"
.head 5 -  Top: 5.155"
.head 5 -  Width:  1.2"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.298"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Keyboard Accelerator: (none)
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Picture File Name:
.head 4 -  Picture Transparent Color: None
.head 4 -  Image Style: Single
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 4 -  Button Appearance: Standard
.head 4 +  Message Actions
.head 5 +  On SAM_Click
.head 6 -  Call SendMail( TRUE )
.head 3 -  Line
.head 4 -  Resource Id: 9692
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Coordinates
.head 5 -  Begin X:  0.183"
.head 5 -  Begin Y:  1.238"
.head 5 -  End X:  9.683"
.head 5 -  End Y:  1.238"
.head 4 -  Visible? Yes
.head 4 -  Line Style: Etched
.head 4 -  Line Thickness: 1
.head 4 -  Line Color: Default
.head 3 -  Line
.head 4 -  Resource Id: 9691
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Coordinates
.head 5 -  Begin X:  0.183"
.head 5 -  Begin Y:  4.488"
.head 5 -  End X:  9.683"
.head 5 -  End Y:  4.488"
.head 4 -  Visible? Yes
.head 4 -  Line Style: Etched
.head 4 -  Line Thickness: 1
.head 4 -  Line Color: Default
.head 3 -  Line
.head 4 -  Resource Id: 9690
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Coordinates
.head 5 -  Begin X:  0.183"
.head 5 -  Begin Y:  4.988"
.head 5 -  End X:  9.683"
.head 5 -  End Y:  4.988"
.head 4 -  Visible? Yes
.head 4 -  Line Style: Etched
.head 4 -  Line Thickness: 1
.head 4 -  Line Color: Default
.head 3 +  Pushbutton: pbAbbruch
.head 4 -  Class Child Ref Key: 0
.head 4 -  Class ChildKey: 0
.head 4 -  Class:
.head 4 -  Property Template:
.head 4 -  Class DLL Name:
.head 4 -  Title: &Close
.head 4 -  Window Location and Size
.head 5 -  Left: 8.483"
.head 5 -  Top: 5.155"
.head 5 -  Width:  1.2"
.head 5 -  Width Editable? Yes
.head 5 -  Height: 0.298"
.head 5 -  Height Editable? Yes
.head 4 -  Visible? Yes
.head 4 -  Keyboard Accelerator: (none)
.head 4 -  Font Name: Default
.head 4 -  Font Size: Default
.head 4 -  Font Enhancement: Default
.head 4 -  Picture File Name:
.head 4 -  Picture Transparent Color: None
.head 4 -  Image Style: Single
.head 4 -  Text Color: Default
.head 4 -  Background Color: Default
.head 4 -  Button Appearance: Standard
.head 4 +  Message Actions
.head 5 +  On SAM_Click
.head 6 -  Call SalEndDialog( hWndForm, 1 )
.head 2 +  Functions
.head 3 +  Function: Init
.head 4 -  Description:
.head 4 -  Returns
.head 4 -  Parameters
.head 4 -  Static Variables
.head 4 -  Local variables
.head 4 +  Actions
.head 5 +  If p_sEMail_To != ''
.head 6 -  Set dfs_To = p_sEMail_To
.head 5 -  !
.head 5 +  If p_sEMail_CC != ''
.head 6 -  Set dfs_CC = p_sEMail_CC
.head 5 -  !
.head 5 +  If p_sEMail_BC != ''
.head 6 -  Set dfs_BC = p_sEMail_BC
.head 5 -  !
.head 5 +  If p_sEMail_Subject != ''
.head 6 -  Set dfs_Subject = p_sEMail_Subject
.head 5 -  !
.head 5 +  If p_lsEMail_Text != ''
.head 6 -  Set ml_Text = p_lsEMail_Text
.head 5 -  !
.head 5 +  If p_lsEMail_Files != ''
.head 6 -  Set dfs_Files = p_lsEMail_Files
.head 3 +  Function: SendMail
.head 4 -  Description:
.head 4 +  Returns
.head 5 -  Boolean:
.head 4 +  Parameters
.head 5 -  Boolean: p_bDirect
.head 4 -  Static Variables
.head 4 -  Local variables
.head 4 +  Actions
.head 5 -  !
.head 5 +  If SalIsNull( dfs_To ) or SalIsNull( dfs_Subject ) 
.head 6 -  Return FALSE
.head 5 -  !
.head 5 +  If ConnectEmailClient()
.head 6 +  If p_bDirect
.head 7 -  Call EMail.EmailSendMessage( 0, dfs_Subject, ml_Text, dfs_To, dfs_CC, dfs_BC, dfs_Files, 0, 0 ) 
.head 6 +  Else
.head 7 -  Call EMail.EmailSendMessage( 1, dfs_Subject, ml_Text, dfs_To, dfs_CC, dfs_BC, dfs_Files, 0, 0 ) 
.head 6 -  Return TRUE
.head 3 +  Function: Adresse
.head 4 -  Description:
.head 4 -  Returns
.head 4 +  Parameters
.head 5 -  Number: p_nArg
.head 4 -  Static Variables
.head 4 +  Local variables
.head 5 -  String: sTemp1
.head 5 -  String: sTemp2
.head 5 -  String: sTemp3
.head 4 +  Actions
.head 5 +  If ConnectEmailClient()
.head 6 +  If p_nArg = 1
.head 7 -  Call EMail.EmailAddress( 'Choose To', 1, sTemp1, dfs_To, sTemp2, sTemp3, 0 )
.head 6 +  Else If p_nArg = 2
.head 7 -  Call EMail.EmailAddress( 'Choose CC', 1, sTemp1, dfs_CC, sTemp2, sTemp3, 0 )
.head 6 +  Else
.head 7 -  Call EMail.EmailAddress( 'Choose BC', 1, sTemp1, dfs_BC, sTemp2, sTemp3, 0 )
.head 3 +  Function: Attachment
.head 4 -  Description:
.head 4 -  Returns
.head 4 -  Parameters
.head 4 -  Static Variables
.head 4 +  Local variables
.head 5 -  String: saFilters[*]
.head 5 -  Number: nIndexFilters
.head 5 -  String: sCurrentPath
.head 5 -  String: sFile
.head 5 -  String: sPath
.head 5 -  Number: nCounter
.head 4 +  Actions
.head 5 -  Set saFilters[ 0 ] = 'All Files'
.head 5 -  Set saFilters[ 1 ] = '*.*'
.head 5 -  Call SalDlgOpenFile( hWndForm, 'EMail - Datei hinzuf�gen', saFilters, 2, nIndexFilters, sFile, sPath )
.head 5 +  If sFile != ''
.head 6 -  Set dfs_Files = dfs_Files || sPath || ';'
.head 3 +  Function: ConnectEmailClient
.head 4 -  Description:
.head 4 +  Returns
.head 5 -  Boolean:
.head 4 -  Parameters
.head 4 -  Static Variables
.head 4 -  Local variables
.head 4 +  Actions
.head 5 +  If Not EMail.EmailIsConnected()
.head 6 +  If Not EMail.EmailConnect('','','', EMAIL_SYS_MSMAIL)
.head 7 -  Call SalWaitCursor( 0 )
.head 7 -  Call SalMessageBox( 'The system could not connect to the Email-Client!', 'EMail', MB_IconExclamation )
.head 7 -  Return FALSE
.head 6 -  Call EMail.EmailSetDebugMode( 0 )
.head 5 -  Return TRUE
.head 2 +  Window Parameters
.head 3 -  String: p_sEMail_To
.head 3 -  String: p_sEMail_CC
.head 3 -  String: p_sEMail_BC
.head 3 -  String: p_sEMail_Subject
.head 3 -  Long String: p_lsEMail_Text
.head 3 -  Long String: p_lsEMail_Files
.head 2 +  Window Variables
.head 3 -  FunctionalVar: EMail
.head 4 -  Class: cQOEmail
.head 2 +  Message Actions
.head 3 +  On SAM_Create
.head 4 -  Call SalCenterWindow( hWndForm )
.head 4 -  Call Init()
.head 3 +  On SAM_Destroy
.head 4 +  If EMail.EmailIsConnected()
.head 5 -  Call EMail.EmailDisconnect()