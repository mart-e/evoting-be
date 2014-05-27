Application Description: Centura SQLWindows/32 Standard Application Template
	Outline Version - 4.0.27
	Design-time Settings
.data VIEWINFO
0000: 6F00000001000000 FFFF01000D004347 5458566965775374 6174650400010000
0020: 0000000000A50000 002C000000020000 0003000000FFFFFF FFFFFFFFFFFCFFFF
0040: FFE9FFFFFFFFFFFF FF000000007C0200 004D010000010000 0000000000010000
0060: 000F4170706C6963 6174696F6E497465 6D0200000012496E 7465726E616C2046
0080: 756E6374696F6E73 0F53616C4E756D62 65724D6F64756C6F
.enddata
		Outline Window State: Normal
		Outline Window Location and Size
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
			Left: -0.013"
			Top:    0.0"
			Width:  8.013"
			Height: 4.969"
		Options Box Location
.data VIEWINFO
0000: D4180909B80B1A00
.enddata
.data VIEWSIZE
0000: 0800
.enddata
			Visible? No
			Left: 4.15"
			Top:    1.885"
			Width:  3.8"
			Height: 2.073"
		Class Editor Location
			Visible? No
			Left: 0.575"
			Top:    0.094"
			Width:  5.063"
			Height: 2.719"
		Tool Palette Location
			Visible? No
			Left: 6.388"
			Top:    0.729"
		Fully Qualified External References? Yes
		Reject Multiple Window Instances? No
		Enable Runtime Checks Of External References? Yes
		Use Release 4.0 Scope Rules? No
	Libraries
	Global Declarations
		Window Defaults
			Tool Bar
				Display Style? Etched
				Font Name: MS Sans Serif
				Font Size: 8
				Font Enhancement: System Default
				Text Color: System Default
				Background Color: System Default
			Form Window
				Display Style? Etched
				Font Name: MS Sans Serif
				Font Size: 8
				Font Enhancement: System Default
				Text Color: System Default
				Background Color: System Default
			Dialog Box
				Display Style? Etched
				Font Name: MS Sans Serif
				Font Size: 8
				Font Enhancement: System Default
				Text Color: System Default
				Background Color: System Default
			Top Level Table Window
				Font Name: MS Sans Serif
				Font Size: 8
				Font Enhancement: System Default
				Text Color: System Default
				Background Color: System Default
			Data Field
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Multiline Field
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Spin Field
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Background Text
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Pushbutton
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
			Radio Button
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Check Box
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Option Button
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
			Group Box
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Child Table Window
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			List Box
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Combo Box
				Font Name: Use Parent
				Font Size: Use Parent
				Font Enhancement: Use Parent
				Text Color: Use Parent
				Background Color: Use Parent
			Line
				Line Color: Use Parent
			Frame
				Border Color: Use Parent
				Background Color: 3D Face Color
			Picture
				Border Color: Use Parent
				Background Color: Use Parent
		Formats
			Number: 0'%'
			Number: #0
			Number: ###000
			Number: ###000;'($'###000')'
			Date/Time: hh:mm:ss AMPM
			Date/Time: M/d/yy
			Date/Time: MM-dd-yy
			Date/Time: dd-MMM-yyyy
			Date/Time: MMM d, yyyy
			Date/Time: MMM d, yyyy hh:mm AMPM
			Date/Time: MMMM d, yyyy hh:mm AMPM
		External Functions
		Constants
.data CCDATA
0000: 3000000000000000 0000000000000000 00000000
.enddata
.data CCSIZE
0000: 1400
.enddata
			System
			User
		Resources
		Variables
		Internal Functions
			Function: SalNumberToHex
				Description: 
						author: 	???
						date:	2000
						version:	1.00
						
						desc:
						Converts a number to a hex value
						
						example:
				Returns
					String:
				Parameters
					Number: p_nValue
.data INHERITPROPS
0000: 0100
.enddata
				Static Variables
				Local variables
					Number: nRemainder
.data INHERITPROPS
0000: 0100
.enddata
					String: sHex
.data INHERITPROPS
0000: 0100
.enddata
				Actions
					Set p_nValue=SalNumberAbs( p_nValue )
					If p_nValue > 15
						Set sHex=SalNumberToHex( SalNumberTruncate( p_nValue/16, 18, 0 ))
					Set nRemainder=SalNumberMod( p_nValue, 16 )
					If nRemainder > 9
						Set nRemainder=nRemainder + 7
					Set sHex=sHex || SalNumberToChar( nRemainder + 48 )
					Return sHex
			Function: SalNumberCeil
				Description: 
						author: 	Philippe Gibone
						date:  	10/07/2002
						version 	1.0
						
						desc: 	Calculates the lowest int above or equal n
						 
						example:
							SalNumberCeil (3.00001) = 4
							SalNumberCeil (3.99999) = 4
							SalNumberCeil (-3.00001) = -3
							SalNumberCeil (-3.99999) = -3
						
				Returns
					Number:
				Parameters
					Number: nNumber
				Static Variables
				Local variables
				Actions
					If SalNumberModulo(nNumber, 1) = 0
						Return nNumber
					Set nNumber = nNumber + (1 - SalNumberModulo( nNumber, 1 ))
					Return nNumber
			Function: SalNumberFloor
				Description: 
						author: 	Philippe Gibone
						date:  	10/07/2002
						version 	1.0
						
						desc:	Calculates the biggest int below or equal n
						
						example:	SalNumberFloor (3.00001) = 3
							SalNumberFloor (3.99999) = 3
							SalNumberFloor (-3.00001) = -4
							SalNumberFloor (-3.99999) = -4
						
				Returns
					Number:
				Parameters
					Number: nNumber
				Static Variables
				Local variables
				Actions
					If SalNumberModulo(nNumber, 1) = 0
						Return nNumber
					Set nNumber = nNumber  - SalNumberModulo( nNumber, 1 )
					Return nNumber
			Function: SalNumberModulo
				Description: author: 	Philippe Gibone
						date:  	10/07/2002
						version 	1.0
						
						desc:	Calculates the mathematical modulo function ie SalNumberModulo(-5, 8) = 3, not -5
						
						example:
						
				Returns
					Number:
				Parameters
					Number: nNumber
					Number: nNumberMod
				Static Variables
				Local variables
				Actions
					Return SalNumberMod(SalNumberMod(nNumber, nNumberMod) + nNumberMod, nNumberMod)
			Function: SalNumberToInt
				Description: 
						author: 	gb
						date:	2000
						version:	1.00
						
						
						description:
						Rounds a float to a integer (with the individual option to round up or down)
						
						parameter:
						1. Number:	Dezimal value
						2. Boolean:	True = round up; False = round down
						Return:
						1. Number:	The rounded value
						
						example:
						Set lnText = SalNumberToInt( 37.14, TRUE )
						lnTest -> 38
						
						Set lnText = SalNumberToInt( 37.14, FALSE )
						lnTest -> 37
						
				Returns
					Number:
				Parameters
					Number: pnNumber
					Boolean: pbRound
				Static Variables
				Local variables
					Number: lnNumber
				Actions
					Set lnNumber = SalNumberTruncate( pnNumber, 10, 0 )
					If SalNumberMod( pnNumber, 1 ) And pbRound
						Set lnNumber = lnNumber + 1
					Return lnNumber
			Function: SalHexToNumber
				Description: 
						author: 	???
						date:	2000
						version:	1.00
						
						desc:
						Converts a hexvalue in a number
						
						example:
						Set lnNr = SalHexToNumber('0x1a')
						-> lnNr = 26
				Returns
					Number:
				Parameters
					String: p_sHex
.data INHERITPROPS
0000: 0100
.enddata
				Static Variables
				Local variables
					Number: nDec
.data INHERITPROPS
0000: 0100
.enddata
					Number: nLen
.data INHERITPROPS
0000: 0100
.enddata
					Number: nLower
.data INHERITPROPS
0000: 0100
.enddata
				Actions
					Set p_sHex=SalStrUpperX( SalStrTrimX( p_sHex ) )
					If SalStrLeftX( p_sHex, 2 )='0X'
						Set p_sHex=SalStrRightX( p_sHex, SalStrLength( p_sHex ) - 2 )
					Set nDec=SalStrLop( p_sHex )
					Set nLen=SalStrLength( p_sHex )
					If (nDec > 47) and (nDec < 58)
						Set nDec=nDec - 48
					Else If (nDec > 64) and (nDec < 71)
						Set nDec=nDec - 55
					Else
						Set nDec=NUMBER_Null
					If (nLen > 0) and (nDec !=NUMBER_Null)
						Set nLower=SalHexToNumber( p_sHex )
						If nLower !=NUMBER_Null
							Set nDec=nDec * SalNumberPower( 16, nLen ) + nLower
						Else
							Set nDec=NUMBER_Null
					Return nDec
		Named Menus
		Class Definitions
		Default Classes
			MDI Window: cBaseMDI
			Form Window:
			Dialog Box:
			Table Window:
			Quest Window:
			Data Field:
			Spin Field:
			Multiline Field:
			Pushbutton:
			Radio Button:
			Option Button:
			Check Box:
			Child Table:
			Quest Child Window: cQuickDatabase
			List Box:
			Combo Box:
			Picture:
			Vertical Scroll Bar:
			Horizontal Scroll Bar:
			Column:
			Background Text:
			Group Box:
			Line:
			Frame:
			Custom Control:
			ActiveX:
		Application Actions
