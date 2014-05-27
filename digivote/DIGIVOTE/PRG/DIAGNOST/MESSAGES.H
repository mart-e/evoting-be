//=============================================================================
/*

    File       : C:\DIGIVOTE\PRG\DIAGNOST\MESSAGES.H

    Project    : Steria Benelux sa/nv - DIGIVOTE
    Author     : (Generated from MsAccess97)

    Description: Mnemonics and definitions for the messages.

    DO NOT MODIFY THIS FILE MANUALLY, USE THE APPROPRIATE TOOL TO GENERATE !
*/
//=============================================================================


#ifndef MESSAGES_H
    #define MESSAGES_H

#ifdef ONLY_IN_MSG
char *LanguageString[] = {
	{ "Nederlands" },
	{ "fran‡ais" },
	{ "Deutsch" },
	{ "English" }
};
char *LanguageCode[] = {
	{ "NL" },
	{ "FR" },
	{ "DU" },
	{ "UK" }
};
char *LanguageChoice = {
	"TAAL / LANGUE / SPRACHE / LANGUAGE"
};
char *SelectLanguage = {
	"Select your language.\nSPACEBAR, TAB or CURSOR LEFT/RIGHT to select,\nENTER to activate"
};
#endif // ONLY_IN_MSG

#ifndef RTC_LOAD
 
//=============================================================================
// (Not categorized)
//=============================================================================
 
	//=========================================================================
	// (Not categorized) => (Not categorized)
		const int End = 0 ;
		const int GatheringNICInformation = 1 ;
		const int GiveTypeOfThisSystem1 = 2 ;
		const int GiveTypeOfThisSystem2 = 3 ;
		const int Information = 4 ;
		const int InformationAccept = 5 ;
		const int NotForThisSystem = 6 ;
		const int PleaseWait = 7 ;
		const int ResultHeader = 8 ;
	//=========================================================================
 
	//=========================================================================
	// Buttons => (Not categorized)
		const int AcceptConfiguration = 9 ;
		const int AcceptPossibilities = 12 ;
		const int Continue = 15 ;
		const int EnterToContinue = 17 ;
		const int ProblemSolved = 19 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => (Not categorized)
		const int RepeatTest = 21 ;
		const int SelectComponent = 22 ;
		const int UsageFormat = 23 ;
	//=========================================================================
 
	//=========================================================================
	// Results => (Not categorized)
		const int Cancelled = 24 ;
		const int Connected = 25 ;
		const int InternalError = 26 ;
		const int NotConnected = 27 ;
		const int NotPresent = 28 ;
		const int OK = 29 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => (Not categorized)
		const int ComponentSelection = 30 ;
		const int ConfigurationTest = 31 ;
		const int DiagTitle = 32 ;
		const int HardwareTests = 33 ;
	//=========================================================================
 
//=============================================================================
// Alarm
//=============================================================================
 
	//=========================================================================
	// Headings => Alarm
		const int Alarm = 34 ;
		const int Buzzer = 35 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Alarm
		const int ConnectAlarmBox = 36 ;
		const int PushOnAlarmResetButton = 37 ;
		const int PushOnAlarmResetButtonBUZZER = 38 ;
		const int PushOnAlarmResetButtonGREENled = 39 ;
		const int PushOnAlarmResetButtonREDled = 40 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Alarm
		const int AlarmBUZZER = 41 ;
		const int AlarmLED = 42 ;
		const int AlarmNotConnected = 43 ;
		const int AlarmResetButton = 44 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Alarm
		const int AlarmTest = 45 ;
		const int BuzzerTest = 46 ;
	//=========================================================================
 
//=============================================================================
// Battery
//=============================================================================
 
	//=========================================================================
	// Instructions => Battery
		const int BatteryContinue = 47 ;
		const int BatteryWarning = 48 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Battery
		const int BatteryLog = 49 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Battery
		const int BatteryAlert = 50 ;
	//=========================================================================
 
//=============================================================================
// Bios
//=============================================================================
 
	//=========================================================================
	// (Not categorized) => Bios
		const int BadRelease = 51 ;
		const int BiosUpdate = 52 ;
		const int BIOSversion = 53 ;
		const int DetectedBIOS = 54 ;
		const int NotAmi = 55 ;
		const int RestoreFailed = 56 ;
		const int UnsupportedBios = 57 ;
		const int UpgradeBios = 58 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Bios
		const int BiosBadRelease = 59 ;
		const int BiosNoUpdate = 60 ;
		const int RestoreSucceeded = 61 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Bios
		const int SaveBiosConfig = 62 ;
	//=========================================================================
 
//=============================================================================
// Bureautica
//=============================================================================
 
	//=========================================================================
	// Instructions => Bureautica
		const int AskForBackup = 63 ;
		const int BackupInfo = 64 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Bureautica
		const int VerifySysInfo = 74 ;
	//=========================================================================
 
//=============================================================================
// CardReader
//=============================================================================
 
	//=========================================================================
	// Buttons => CardReader
		const int CardReaderType = 75 ;
	//=========================================================================
 
	//=========================================================================
	// Headings => CardReader
		const int COM1CardReader = 78 ;
		const int COM2CardReader = 79 ;
		const int ContinuousCardReader = 80 ;
		const int ExternalCardReader = 81 ;
		const int InternalCardReader = 82 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => CardReader
		const int InsertCardCOM1 = 83 ;
		const int InsertCardCOM2 = 84 ;
		const int InsertCardInExternalCardReader = 85 ;
		const int InsertCardInInternalCardReader = 86 ;
		const int PassesProcessedFormat = 87 ;
		const int SelectTypeCardReader = 88 ;
		const int TestEndsWhenAllCardsAreRemoved = 89 ;
		const int TestEndsWhenCardIsRemoved = 90 ;
	//=========================================================================
 
	//=========================================================================
	// Results => CardReader
		const int CardReaderCommands = 91 ;
		const int CardReaderErrorMsg = 114 ;
		const int CardReaderInvalid = 141 ;
		const int CardReaderName = 142 ;
		const int CardReaderNoCardEntered = 147 ;
		const int CardReaderNotConnected = 148 ;
		const int SearchingForCardReader = 149 ;
		const int TimesPerformedCOM1 = 150 ;
		const int TimesPerformedCOM2 = 151 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => CardReader
		const int COM1CardReaderTest = 152 ;
		const int COM2CardReaderTest = 153 ;
		const int ContCardReaderCOM1Test = 154 ;
		const int ContCardReaderCOM2Test = 155 ;
		const int ContinuousCardReaderTest = 156 ;
		const int ExternalCardReaderTest = 157 ;
		const int InternalCardReaderTest = 158 ;
	//=========================================================================
 
//=============================================================================
// Date/Time
//=============================================================================
 
	//=========================================================================
	// Headings => Date/Time
		const int Date = 159 ;
		const int Day = 160 ;
		const int Hour = 161 ;
		const int Minutes = 162 ;
		const int Month = 163 ;
		const int Time = 164 ;
		const int Year = 165 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Date/Time
		const int BadDate = 166 ;
		const int BadTime = 167 ;
		const int DateAccept = 168 ;
		const int DateTimeAccept = 169 ;
		const int GiveDay = 170 ;
		const int GiveHour = 171 ;
		const int GiveMinutes = 172 ;
		const int GiveMonth = 173 ;
		const int GiveYear = 174 ;
		const int TimeAccept = 175 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Date/Time
		const int DateAndTime = 176 ;
	//=========================================================================
 
//=============================================================================
// Disk
//=============================================================================
 
	//=========================================================================
	// Buttons => Disk
		const int DiskType = 177 ;
		const int FloppyType = 180 ;
		const int RemovableDiskType = 183 ;
	//=========================================================================
 
	//=========================================================================
	// Headings => Disk
		const int Disk = 186 ;
		const int Floppy = 187 ;
		const int FloppyA = 188 ;
		const int FloppyB = 189 ;
		const int RamDriveCheck = 190 ;
		const int RemovableDisk = 191 ;
		const int RemovableDisk1 = 192 ;
		const int RemovableDisk2 = 193 ;
		const int TypeDiskPM = 194 ;
		const int TypeDiskPS = 195 ;
		const int TypeDiskSM = 196 ;
		const int TypeDiskSS = 197 ;
		const int TypeFloppyA = 198 ;
		const int TypeFloppyB = 199 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Disk
		const int AskDiskInDrive = 200 ;
		const int AskForDiagnosticDisk = 201 ;
		const int AskForOtherDisk = 202 ;
		const int DiskInformation = 203 ;
		const int InsertDiagnostic = 209 ;
		const int InsertNewFloppy = 210 ;
		const int NoDiskPresent = 211 ;
		const int NotDiagnosticsDisk = 212 ;
		const int NotOtherDisk = 213 ;
		const int RemoveDiskette = 214 ;
		const int SelectTypeDisk = 215 ;
		const int WriteProtectedDisk = 216 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Disk
		const int DiskCanNotOpenFile = 217 ;
		const int DiskVerifyError = 218 ;
		const int DosErrorMsg = 219 ;
		const int DriveTypes = 232 ;
		const int FloppyRemovalError = 236 ;
		const int SyQuestNotPresentAtBootTime = 237 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Disk
		const int DiskTest = 238 ;
		const int FloppyTest = 239 ;
		const int GatheringDiskInformation = 240 ;
		const int RamDriveTest = 241 ;
	//=========================================================================
 
//=============================================================================
// Keyboard
//=============================================================================
 
	//=========================================================================
	// Headings => Keyboard
		const int Keyboard = 242 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Keyboard
		const int KeyboardNotConnected = 243 ;
	//=========================================================================
 
//=============================================================================
// Lightpen
//=============================================================================
 
	//=========================================================================
	// Headings => Lightpen
		const int Lightpen = 244 ;
		const int LightpenDriver = 245 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Lightpen
		const int PushWithLightpenOnAllButtons = 246 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Lightpen
		const int LightpenDriverCorrectLoaded = 247 ;
		const int LightpenStatus = 248 ;
		const int LoadingLightPenDriver = 264 ;
		const int NotApplicable = 265 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Lightpen
		const int LightpenTest = 266 ;
	//=========================================================================
 
//=============================================================================
// Machine ID
//=============================================================================
 
	//=========================================================================
	// Headings => Machine ID
		const int CurrentMachineID = 267 ;
		const int MachineID = 268 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Machine ID
		const int BAD_ID = 269 ;
		const int GiveYourMachineID = 270 ;
		const int MachineNameReset = 271 ;
		const int ThereCanBeOnlyOne = 272 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Machine ID
		const int Limitation = 273 ;
		const int MachineName = 274 ;
	//=========================================================================
 
//=============================================================================
// Memory
//=============================================================================
 
	//=========================================================================
	// Headings => Memory
		const int Memory = 275 ;
		const int MemoryCheck = 276 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Memory
		const int MemoryUnreliable = 277 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Memory
		const int NotEnoughMemory = 278 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Memory
		const int MemoryTest = 279 ;
	//=========================================================================
 
//=============================================================================
// Printer
//=============================================================================
 
	//=========================================================================
	// Buttons => Printer
		const int PrinterType = 280 ;
	//=========================================================================
 
	//=========================================================================
	// Headings => Printer
		const int Printer = 283 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Printer
		const int PrinterTestDATA = 284 ;
		const int SelectTypePrinter = 285 ;
		const int VerifyPrinterTest = 286 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Printer
		const int PrinterNotConnected = 287 ;
		const int PrinterProblem = 288 ;
		const int PrinterStatus = 289 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Printer
		const int PrinterTest = 296 ;
	//=========================================================================
 
//=============================================================================
// Responsable Info
//=============================================================================
 
	//=========================================================================
	// Headings => Responsable Info
		const int OfficeNumber = 297 ;
		const int PersonalID = 298 ;
		const int RepairAdres = 299 ;
		const int RepairAdresCommunity = 300 ;
		const int RepairAdresHouseNr = 301 ;
		const int RepairAdresStreet = 302 ;
		const int Responsable = 303 ;
		const int ResponsableExtension = 304 ;
		const int ResponsableName = 305 ;
		const int ResponsableTelephoneNr = 306 ;
		const int ResponsaInfo = 307 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Responsable Info
		const int AskResponsableExtension = 308 ;
		const int GiveHouseNr = 309 ;
		const int GiveOfficeNumber = 310 ;
		const int GivePID = 311 ;
		const int GiveRepairAdresCommunity = 312 ;
		const int GiveRepairAdresStreet = 313 ;
		const int GiveRepairAdresZIPCode = 314 ;
		const int GiveResponsableExtension = 315 ;
		const int GiveResponsableName = 316 ;
		const int GiveResponsableTelephoneNr = 317 ;
		const int GiveResponsableZoneNr = 318 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Responsable Info
		const int WinCor = 319 ;
	//=========================================================================
 
//=============================================================================
// System
//=============================================================================
 
	//=========================================================================
	// Buttons => System
		const int SystemString = 320 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => System
		const int CorrectSystem = 326 ;
		const int DetectedSystem = 327 ;
		const int SystemWrongConfigured = 328 ;
	//=========================================================================
 
	//=========================================================================
	// Results => System
		const int MachineNotOK = 329 ;
		const int MachineOK = 333 ;
		const int TypeTest = 337 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => System
		const int SystemSelection = 343 ;
		const int SystemSetup = 344 ;
		const int ThatsAllFolks = 345 ;
	//=========================================================================
 
//=============================================================================
// TicketPrinter
//=============================================================================
 
	//=========================================================================
	// Headings => TicketPrinter
		const int TPrinter = 346 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => TicketPrinter
		const int ConnectTPrinter = 347 ;
		const int PrintedTicketOk = 348 ;
		const int TPrinterOk = 349 ;
	//=========================================================================
 
	//=========================================================================
	// Results => TicketPrinter
		const int TicketProblem = 350 ;
		const int TPrinterProblem = 351 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => TicketPrinter
		const int TPrinterTest = 352 ;
	//=========================================================================
 
//=============================================================================
// WinCor
//=============================================================================
 
	//=========================================================================
	// Instructions => WinCor
		const int BattMoBoIncomplete = 353 ;
		const int LoopContinue = 354 ;
		const int LoopEnd = 355 ;
		const int ShutDown = 356 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => WinCor
		const int BatteryMoBoTest = 357 ;
		const int BurnInTitle = 358 ;
	//=========================================================================

	const int MAX_MSG = 360 ;

#else
 
//=============================================================================
// (Not categorized)
//=============================================================================
 
	//=========================================================================
	// (Not categorized) => (Not categorized)
		const int NoConfigFile = 0 ;
	//=========================================================================
 
	//=========================================================================
	// Buttons => (Not categorized)
		const int AcceptPossibilities = 1 ;
		const int ProblemSolved = 4 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => (Not categorized)
		const int Attention = 6 ;
		const int ProgramNotAllowed = 7 ;
	//=========================================================================
 
//=============================================================================
// Bios
//=============================================================================
 
	//=========================================================================
	// (Not categorized) => Bios
		const int PersonalRTCNotFound = 8 ;
		const int RestoreBiosConfig = 9 ;
		const int RestoreCancelled = 10 ;
		const int RestoreFailed = 11 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Bios
		const int RestoreInfo = 12 ;
		const int RestoreNotAllowed = 21 ;
		const int WarningDefaultRTC1 = 24 ;
		const int WarningDefaultRTC2 = 28 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Bios
		const int BiosNotConform = 32 ;
		const int RestoreSucceeded = 33 ;
	//=========================================================================
 
//=============================================================================
// Bureautica
//=============================================================================
 
	//=========================================================================
	// (Not categorized) => Bureautica
		const int AlreadyEnabled = 34 ;
		const int ConfigFileCorrupt = 37 ;
		const int DefaultRTCNotFound = 38 ;
	//=========================================================================
 
	//=========================================================================
	// Buttons => Bureautica
		const int AcceptRestore = 39 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Bureautica
		const int AskForReload = 43 ;
		const int AskForRestore = 44 ;
		const int AskForRestoreDefault = 45 ;
	//=========================================================================
 
//=============================================================================
// Date/Time
//=============================================================================
 
	//=========================================================================
	// Headings => Date/Time
		const int Date = 46 ;
		const int Day = 47 ;
		const int Hour = 48 ;
		const int Minutes = 49 ;
		const int Month = 50 ;
		const int Time = 51 ;
		const int Year = 52 ;
	//=========================================================================
 
	//=========================================================================
	// Instructions => Date/Time
		const int BadDate = 53 ;
		const int BadTime = 54 ;
		const int DateAccept = 55 ;
		const int GiveDay = 56 ;
		const int GiveHour = 57 ;
		const int GiveMinutes = 58 ;
		const int GiveMonth = 59 ;
		const int GiveYear = 60 ;
		const int TimeAccept = 61 ;
	//=========================================================================
 
	//=========================================================================
	// Titles => Date/Time
		const int DateAndTime = 62 ;
	//=========================================================================
 
//=============================================================================
// Disk
//=============================================================================
 
	//=========================================================================
	// Instructions => Disk
		const int WriteProtectedDisk = 63 ;
	//=========================================================================
 
	//=========================================================================
	// Results => Disk
		const int DiskError = 64 ;
	//=========================================================================
 
//=============================================================================
// Machine ID
//=============================================================================
 
	//=========================================================================
	// Headings => Machine ID
		const int CurrentMachineID = 65 ;
	//=========================================================================
 
//=============================================================================
// Responsable Info
//=============================================================================
 
	//=========================================================================
	// Instructions => Responsable Info
		const int GiveOfficeNumber = 66 ;
	//=========================================================================
 
//=============================================================================
// System
//=============================================================================
 
	//=========================================================================
	// Instructions => System
		const int DetectedSystem = 67 ;
		const int SystemLocked = 68 ;
	//=========================================================================

	const int MAX_MSG = 73 ;

#endif // RTC_LOAD

#endif // MESSAGES_H

//=== C:\DIGIVOTE\PRG\DIAGNOST\MESSAGES.H ============================================
