/* keyboard */

#include <windows.h>
#include <tchar.h>
#include "SYSDEPNS.h"
#include "UI/WIN32/OSGLUWIN.h"

#if ItnlKyBdFix
LOCALPROC VkSwapZY(void)
{
	VkMapA['Z'] = 'Y';
	VkMapA['Y'] = 'Z';
}
#endif

#if ItnlKyBdFix
LOCALPROC VkSwapGraveQuote(void)
{
	VkMapA[myVK_Grave] = myVK_SingleQuote;
	VkMapA[myVK_SingleQuote] = myVK_Grave;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkSwapSlashSubtract(void)
{
	VkMapA[myVK_Slash] = myVK_Subtract;
	VkMapA[myVK_Subtract] = myVK_Slash;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkSwapAQZWGraveQuote(void)
{
	VkSwapGraveQuote();
	VkMapA['A'] = 'Q';
	VkMapA['Q'] = 'A';
	VkMapA['Z'] = 'W';
	VkMapA['W'] = 'Z';
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapBelgian(void)
{
	VkSwapAQZWGraveQuote();
	VkMapA['M'] = myVK_SemiColon;
	VkMapA[myVK_SemiColon] = myVK_RightBracket;
	VkMapA[myVK_RightBracket] = myVK_LeftBracket;
	VkMapA[myVK_LeftBracket] = myVK_Subtract;
	VkMapA[myVK_Subtract] = myVK_Equal;
	VkMapA[myVK_Equal] = myVK_Slash;
	VkMapA[myVK_Slash] = myVK_Period;
	VkMapA[myVK_Period] = myVK_Comma;
	VkMapA[myVK_Comma] = 'M';
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapSwiss(void)
{
	VkSwapZY();
	VkMapA[myVK_OEM_8] = myVK_BackSlash;
	VkMapA[myVK_BackSlash] = myVK_SingleQuote;
	VkMapA[myVK_SingleQuote] = myVK_SemiColon;
	VkMapA[myVK_SemiColon] = myVK_LeftBracket;
	VkMapA[myVK_LeftBracket] = myVK_Subtract;
	VkMapA[myVK_Subtract] = myVK_Slash;
	VkMapA[myVK_Slash] = myVK_Grave;
	VkMapA[myVK_Grave] = myVK_RightBracket;
	VkMapA[myVK_RightBracket] = myVK_Equal;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapDanish(void)
{
	VkMapA[myVK_Equal] = myVK_Subtract;
	VkMapA[myVK_Subtract] = myVK_Slash;
	VkMapA[myVK_Slash] = myVK_BackSlash;
	VkMapA[myVK_BackSlash] = myVK_Grave;
	VkMapA[myVK_Grave] = myVK_SemiColon;
	VkMapA[myVK_SemiColon] = myVK_RightBracket;
	VkMapA[myVK_RightBracket] = myVK_LeftBracket;
	VkMapA[myVK_LeftBracket] = myVK_Equal;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapBritish(void)
{
	VkMapA[myVK_OEM_8] = myVK_Grave;
	VkMapA[myVK_Grave] = myVK_SingleQuote;
	VkMapA[myVK_SingleQuote] = myVK_BackSlash;
	VkMapA[myVK_BackSlash] = myVK_OEM_102;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapSpanish(void)
{
	VkMapA[myVK_SemiColon] = myVK_LeftBracket;
	VkMapA[myVK_LeftBracket] = myVK_Subtract;
	VkMapA[myVK_Subtract] = myVK_Slash;
	VkMapA[myVK_Slash] = myVK_BackSlash;
	VkMapA[myVK_BackSlash] = myVK_Grave;
	VkMapA[myVK_Grave] = myVK_SemiColon;

	VkMapA[myVK_RightBracket] = myVK_Equal;
	VkMapA[myVK_Equal] = myVK_RightBracket;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapDutch(void)
{
	VkSwapGraveQuote();
	VkMapA[myVK_SemiColon] = myVK_RightBracket;
	VkMapA[myVK_RightBracket] = myVK_LeftBracket;
	VkMapA[myVK_LeftBracket] = myVK_Subtract;
	VkMapA[myVK_Subtract] = myVK_Slash;
	VkMapA[myVK_Slash] = myVK_Equal;
	VkMapA[myVK_Equal] = myVK_SemiColon;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapGreekIBM(void)
{
	VkSwapSlashSubtract();
	VkMapA[myVK_LeftBracket] = myVK_Equal;
	VkMapA[myVK_Equal] = myVK_LeftBracket;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapFrench(void)
{
	VkSwapAQZWGraveQuote();
	VkMapA['M'] = myVK_SemiColon;
	VkMapA[myVK_SemiColon] = myVK_RightBracket;
	VkMapA[myVK_RightBracket] = myVK_LeftBracket;
	VkMapA[myVK_LeftBracket] = myVK_Subtract;
	VkMapA[myVK_Comma] = 'M';
	VkMapA[myVK_Period] = myVK_Comma;
	VkMapA[myVK_Slash] = myVK_Period;
	VkMapA[myVK_OEM_8] = myVK_Slash;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapGerman(void)
{
	VkSwapZY();
	VkMapSpanish();
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapBosnian(void)
{
	VkSwapZY();
	/* not in Windows 95 */
	VkSwapSlashSubtract();
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapBulgarian(void)
{
	VkMapA[myVK_OEM_8] = myVK_Comma;
	VkMapA[myVK_Comma] = 'Q';
	VkMapA['Q'] = myVK_Period;
	VkMapA[myVK_Period] = myVK_Equal;
}
#endif

#if ItnlKyBdFix
LOCALPROC VkMapFromLayout(uimr sv)
{
	int i;

	for (i = 0; i < 256; ++i) {
		VkMapA[i] = i;
	}

	switch (sv) {
		case 0x00000409:
			/* United States 101 */
			break;
		case 0x0000041c:
			/* Albanian; */
			VkSwapZY();
			break;
		case 0x0000042B:
			/* Armenian Eastern; */
			VkMapDutch();
			break;
		case 0x0001042B:
			/* Armenian Western; */
			VkMapDutch();
			break;
		case 0x0000042C:
			/* not in Windows 95 */
			/* Azeri Latin */
			VkMapBritish();
			break;
		case 0x0001080C:
			/* Belgian (comma) */
			VkMapBelgian();
			break;
		case 0x0000080c:
			/* Belgian French */
			VkMapBelgian();
			break;
		case 0x00000813:
			/* not in Windows 95 */
			/* Belgian (period); */
			VkMapBelgian();
			break;
		case 0x0000141A:
			/* not in Windows 95 */
			/* Bosnian */
			VkMapBosnian();
			break;
		case 0x00000809:
			/* British / United Kingdom */
			VkMapBritish();
			break;
		case 0x00000452:
			/* not in Windows 95 */
			/* United Kingdom Extended */
			VkMapBritish();
			break;
		case 0x00000402:
			/* Bulgarian */
			/* not same in Windows 95 */
			VkMapBulgarian();
			break;
		case 0x00030402:
			/* Bulgarian */
			VkMapBulgarian();
			break;
		case 0x00020402:
			/* Bulgarian (Phonetic) */
			VkMapBosnian();
			break;
		case 0x00001009:
			/* Canadian Multilingual */
			/* not in Windows 95 */
			VkSwapGraveQuote();
			break;
		case 0x00011009:
			/* Canadian Standard */
			VkSwapGraveQuote();
			break;
		case 0x0000041a:
			/* Croatian */
			VkMapBosnian();
			break;
		case 0x00000405:
			/* Czech */
			VkMapBosnian();
#if 0
			/* but Windows 7 gives */
			VkSwapZY();
			VkMapA[myVK_Equal] = myVK_Subtract;
			VkMapA[myVK_Subtract] = myVK_Slash;
			VkMapA[myVK_Slash] = myVK_Equal;
#endif
			break;
		case 0x00020405:
			/* Czech (Programmers) */
			/* only in Windows 95 */
			/* VkSwapZY(); */
			break;
		case 0x00010405:
			/* Czech (Qwerty) */
			/* only in Windows 95 */
			/* VkSwapZY(); */
			break;
		case 0x00000406:
			/* Danish */
			VkMapDanish();
			break;
		case 0x00000413:
			/* Dutch */
			VkMapDutch();
			break;
		case 0x00000425:
			/* Estonian */
			VkMapA[myVK_Grave] = myVK_LeftBracket;
			VkMapA[myVK_LeftBracket] = myVK_RightBracket;
			VkMapA[myVK_RightBracket] = myVK_Slash;
			VkMapA[myVK_Slash] = myVK_SingleQuote;
			VkMapA[myVK_SingleQuote] = myVK_Grave;
			/* only in Windows 95 ? */
			/* VkMapA[VK_DECIMAL] = VK_DELETE; */
			break;
		case 0x00000438:
			/* Faeroe Islands */
			VkMapDanish();
			break;
		case 0x0000040b:
			/* Finnish */
			VkMapDanish();
			break;
		case 0x0001083B:
			/* not in Windows 95 */
			/* Finnish with Sami */
			VkMapDanish();
			break;
		case 0x0000040c:
			/* v = kKbdFrench; */
			/* French */
			VkMapFrench();
			break;
		case 0x00000c0c:
			/* French Canadian */
			VkSwapGraveQuote();
			break;
		case 0x00011809:
			/* not in Windows 95 */
			/* Gaelic */
			VkMapBritish();
			break;
		case 0x00010407:
			/* German (IBM) */
			VkMapGerman();
			break;
		case 0x00000407:
			/* German (Standard) */
			VkMapGerman();
			break;
		case 0x00010408:
			/* Greek IBM 220 */
			/* not in Windows 95 */
			VkMapGreekIBM();
			break;
		case 0x00030408:
			/* Greek IBM 319 */
			/* not in Windows 95 */
			VkMapGreekIBM();
			break;
		case 0x00020408:
			/* Greek Latin IBM 220 */
			/* not in Windows 95 */
			VkSwapSlashSubtract();
			break;
		case 0x00040408:
			/* Greek Latin IBM 319 */
			/* not in Windows 95 */
			VkSwapSlashSubtract();
			break;
		case 0x0000040e:
			/* Hungarian */
			VkMapBosnian();
			VkMapA[myVK_Grave] = '0';
			VkMapA['0'] = myVK_Grave;
			break;
		case 0x0001040E:
			/* Hungarian (101 Keys) */
			VkMapA[myVK_Grave] = '0';
			VkMapA['0'] = myVK_Grave;
			break;
		case 0x0000040f:
			/* Icelandic */
			VkMapDanish();
			break;
		case 0x00001809:
			/* Irish */
			VkMapBritish();
			break;
		case 0x00000410:
			/* Italian */
			VkMapSpanish();
			break;
		case 0x00010410:
			/* Italian 142 */
			VkMapSpanish();
			break;
		case 0x0000080a:
			/* Latin American */
			VkMapSpanish();
			break;
		case 0x0000046E:
			/* Luxembourgish */
			VkMapSwiss();
			break;
		case 0x00000414:
			/* Norwegian */
			VkMapDanish();
			break;
		case 0x0000043B:
			/* Norwegian with Sami */
			VkMapDanish();
			break;
		case 0x00010415:
			/* Polish (214) */
			VkSwapZY();
			/* not in windows 95 */
			VkMapA[myVK_Equal] = myVK_Subtract;
			VkMapA[myVK_Subtract] = myVK_Slash;
			VkMapA[myVK_Slash] = myVK_Equal;
			break;
		case 0x00010416:
			/* Porguguese (Brazilian ABNT2) */
			/* VkMapA[myVK_OEM_8] = ??; */
			/* VkMapA[VK_SEPARATOR] = ??; */
			break;
		case 0x00000816:
			/* Porguguese (Standard) */
			VkMapA[myVK_SemiColon] = myVK_RightBracket;
			VkMapA[myVK_RightBracket] = myVK_Equal;
			VkMapA[myVK_Equal] = myVK_LeftBracket;
			VkMapA[myVK_LeftBracket] = myVK_Subtract;
			VkMapA[myVK_Subtract] = myVK_Slash;
			VkMapA[myVK_Slash] = myVK_BackSlash;
			VkMapA[myVK_BackSlash] = myVK_Grave;
			VkMapA[myVK_Grave] = myVK_SemiColon;
			break;
		case 0x00000418:
			/* Romanian (Legacy) */
			VkSwapZY();
			/* only in Windows 95 */
			/* VkSwapSlashSubtract(); */
			break;
		case 0x0002083B:
			/* Sami Extended Finland-Sweden */
			VkMapDanish();
			break;
		case 0x0001043B:
			/* Sami Extended Norway */
			VkMapDanish();
			break;
		case 0x00010C1A:
			/* in Windows 95 */
			/* Serbian (Latin) */
			VkSwapZY();
			break;
		case 0x0000081A:
			/* not in Windows 95 */
			/* Serbian (Latin) */
			VkMapBosnian();
			break;
		case 0x0000041b:
			/* Slovak */
			VkMapBosnian();
			/* not in Windows 95 */
			VkMapA[myVK_OEM_8] = myVK_Equal;
			break;
		case 0x00000424:
			/* Slovenian */
			VkMapBosnian();
			break;
		case 0x0000040A:
			/* Spanish, not windows 95 */
			VkMapSpanish();
			break;
		case 0x0001040A:
			/* Spanish Variation, not windows 95 */
			VkMapA[myVK_OEM_8] = myVK_Slash;
			VkMapA[myVK_Slash] = myVK_BackSlash;
			VkMapA[myVK_BackSlash] = myVK_Grave;
			VkMapA[myVK_Grave] = myVK_SemiColon;
			VkMapA[myVK_SemiColon] = myVK_RightBracket;
			VkMapA[myVK_RightBracket] = myVK_LeftBracket;
			VkMapA[myVK_LeftBracket] = myVK_Equal;
			break;
		case 0x00000c0a:
			/* kKbdSpanish; */
			/* Spanish Modern, windows 95 */
			VkMapSpanish();
			break;
		case 0x00000403:
			/* Spanish Traditional */
			VkMapSpanish();
			break;
		case 0x0000041d:
			/* Swedish */
			VkMapDanish();
			break;
		case 0x0000083B:
			/* not in windows 95 */
			/* Swedish with Sami */
			VkMapDanish();
			break;
		case 0x0000100c:
			/* Swiss French */
			VkMapSwiss();
			break;
		case 0x00000807:
			/* Swiss German */
			VkMapSwiss();
			break;
		case 0x0000085D:
			/* Inuktitut Latin */
			/* in windows 7, not XP */
			VkMapBritish();
			break;
		case 0x0001045D:
			/* Inuktitut - Naqittaut */
			VkMapBritish();
			break;
		case 0x0000046F:
			/* Greenlandic */
			VkMapDanish();
			break;
		case 0x00020427:
			/* Lithuanian Standard */
			VkMapDanish();
			break;
		case 0x0000042f:
			/* Macedonian (FYROM) - Standard */
			VkMapBosnian();
			break;
		case 0x0000042E:
			/* Sorbian Standard (Legacy) */
			VkMapGerman();
			break;
		case 0x0001042E:
			/* Sorbian Extended */
			VkMapGerman();
			break;
		case 0x0002042E:
			/* Sorbian Standard */
			VkMapGerman();
			break;
		case 0x00000488:
			/* Wolof */
			VkMapFrench();
			break;
		case 0x0000041f:
			/* Turkish (Q type) */
			/* windows 95 */
			/* VkMapA[myVK_Equal] = myVK_Subtract; */
			/* VkMapA[myVK_Subtract] = myVK_Equal; */
			/* not windows 95 */
			VkMapA[myVK_OEM_8] = myVK_Subtract;
			VkMapA[myVK_Subtract] = myVK_Equal;

			VkMapA[myVK_Comma] = myVK_BackSlash;
			VkMapA[myVK_BackSlash] = myVK_Period;
			VkMapA[myVK_Period] = myVK_Slash;
			VkMapA[myVK_Slash] = myVK_Comma;
			break;
		case 0x00010409:
			/* United States Dvorak */
			VkMapA[myVK_LeftBracket] = myVK_Subtract;
			VkMapA[myVK_RightBracket] = myVK_Equal;
			VkMapA[myVK_SingleQuote] = 'Q';
			VkMapA[myVK_Comma] = 'W';
			VkMapA[myVK_Period] = 'E';
			VkMapA['P'] = 'R';
			VkMapA['Y'] = 'T';
			VkMapA['F'] = 'Y';
			VkMapA['G'] = 'U';
			VkMapA['C'] = 'I';
			VkMapA['R'] = 'O';
			VkMapA['L'] = 'P';
			VkMapA[myVK_Slash] = myVK_LeftBracket;
			VkMapA[myVK_Equal] = myVK_RightBracket;
			VkMapA['O'] = 'S';
			VkMapA['E'] = 'D';
			VkMapA['U'] = 'F';
			VkMapA['I'] = 'G';
			VkMapA['D'] = 'H';
			VkMapA['H'] = 'J';
			VkMapA['T'] = 'K';
			VkMapA['N'] = 'L';
			VkMapA['S'] = myVK_SemiColon;
			VkMapA[myVK_Subtract] = myVK_SingleQuote;
			VkMapA[myVK_SemiColon] = 'Z';
			VkMapA['Q'] = 'X';
			VkMapA['J'] = 'C';
			VkMapA['K'] = 'V';
			VkMapA['X'] = 'B';
			VkMapA['B'] = 'N';
			VkMapA['W'] = myVK_Comma;
			VkMapA['V'] = myVK_Period;
			VkMapA['Z'] = myVK_Slash;
			break;
#if 0
		/* too complicated, don't bother with */
		case 0x0x00000426:
			/* Latvian */
			VkMapA['F'] = myVK_Equal;
			VkMapA['G'] = 'W';
			VkMapA['J'] = 'E';
			VkMapA['M'] = 'T';
			VkMapA['V'] = 'Y';
			VkMapA['N'] = 'U';
			VkMapA['Z'] = 'I';
			VkMapA['W'] = 'O';
			VkMapA['X'] = 'P';
			VkMapA['Y'] = myVK_LeftBracket;
			VkMapA['H'] = myVK_RightBracket;
			VkMapA[myVK_SemiColon] = 'A';
			VkMapA['U'] = 'S';
			VkMapA['S'] = 'D';
			VkMapA['I'] = 'F';
			VkMapA['L'] = 'G';
			VkMapA['D'] = 'H';
			VkMapA['A'] = 'J';
			VkMapA['T'] = 'K';
			VkMapA['E'] = 'L';
			VkMapA['C'] = myVK_SemiColon;
			VkMapA[myVK_LeftBracket] = 'Z';
			VkMapA['B'] = 'X';
			VkMapA[myVK_RightBracket] = 'C';
			VkMapA['K'] = 'V';
			VkMapA['P'] = 'B';
			VkMapA['O'] = 'N';
			VkMapA[myVK_OEM_8] = 'M';
			break;
		case 0x0001041F:
			/* Turkish (F type) */

			VkMapA[myVK_Equal] = myVK_Subtract;
			VkMapA[myVK_Subtract] = myVK_Equal;
			VkMapA['F'] = 'Q';
			VkMapA['G'] = 'W';
			VkMapA[myVK_SemiColon] = 'E';
			VkMapA['I'] = 'R';
			VkMapA['O'] = 'T';
			VkMapA['D'] = 'Y';
			VkMapA['R'] = 'U';
			VkMapA['N'] = 'I';
			VkMapA['H'] = 'O';
			VkMapA['Q'] = myVK_LeftBracket;
			VkMapA['W'] = myVK_RightBracket;
			VkMapA['U'] = 'A';
			VkMapA[myVK_LeftBracket] = 'S';
			VkMapA['E'] = 'D';
			VkMapA['A'] = 'F';
			VkMapA[myVK_RightBracket] = 'G';
			VkMapA['T'] = 'H';
			VkMapA['K'] = 'J';
			VkMapA['M'] = 'K';
			VkMapA['Y'] = myVK_SemiColon;
			VkMapA['X'] = myVK_BackSlash;
			VkMapA['J'] = 'Z';
			VkMapA[myVK_BackSlash] = 'X';
			VkMapA['V'] = 'C';
			VkMapA['C'] = 'V';
			VkMapA[myVK_Slash] = 'B';
			VkMapA['Z'] = 'N';
			VkMapA['S'] = 'M';
			VkMapA['B'] = myVK_Comma;
			VkMapA[myVK_Comma] = myVK_Slash;
			break;
		case 0x00030409:
			/* United States LH Dvorak */
			VkMapA[myVK_LeftBracket] = '1';
			VkMapA[myVK_RightBracket] = '2';
			VkMapA[myVK_Slash] = '3';
			VkMapA['P'] = '4';
			VkMapA['F'] = '5';
			VkMapA['M'] = '6';
			VkMapA['L'] = '7';
			VkMapA['J'] = '8';
			VkMapA['4'] = '9';
			VkMapA['3'] = '0';
			VkMapA['2'] = myVK_Subtract;
			VkMapA['1'] = myVK_Equal;
			VkMapA[myVK_SemiColon] = 'Q';
			VkMapA['Q'] = 'W';
			VkMapA['B'] = 'E';
			VkMapA['Y'] = 'R';
			VkMapA['U'] = 'T';
			VkMapA['R'] = 'Y';
			VkMapA['S'] = 'U';
			VkMapA['O'] = 'I';
			VkMapA[myVK_Period] = 'O';
			VkMapA['6'] = 'P';
			VkMapA['5'] = myVK_LeftBracket;
			VkMapA[myVK_Equal] = myVK_RightBracket;
			VkMapA[myVK_Subtract] = 'A';
			VkMapA['K'] = 'S';
			VkMapA['C'] = 'D';
			VkMapA['D'] = 'F';
			VkMapA['T'] = 'G';
			VkMapA['E'] = 'J';
			VkMapA['A'] = 'K';
			VkMapA['Z'] = 'L';
			VkMapA['8'] = myVK_SemiColon;
			VkMapA['7'] = myVK_SingleQuote;
			VkMapA[myVK_SingleQuote] = 'Z';
			VkMapA['G'] = 'C';
			VkMapA['W'] = 'B';
			VkMapA['I'] = 'M';
			VkMapA['0'] = myVK_Period;
			VkMapA['9'] = myVK_Slash;
			break;
		case 0x00040409:
			/* United States RH Dvorak */
			VkMapA['J'] = '5';
			VkMapA['L'] = '6';
			VkMapA['M'] = '7';
			VkMapA['F'] = '8';
			VkMapA['P'] = '9';
			VkMapA[myVK_Slash] = '0';
			VkMapA[myVK_LeftBracket] = myVK_Subtract;
			VkMapA[myVK_RightBracket] = myVK_Equal;
			VkMapA['5'] = 'Q';
			VkMapA['6'] = 'W';
			VkMapA['Q'] = 'E';
			VkMapA[myVK_Period] = 'R';
			VkMapA['O'] = 'T';
			VkMapA['R'] = 'Y';
			VkMapA['S'] = 'U';
			VkMapA['U'] = 'I';
			VkMapA['Y'] = 'O';
			VkMapA['B'] = 'P';
			VkMapA[myVK_SemiColon] = myVK_LeftBracket;
			VkMapA[myVK_Equal] = myVK_RightBracket;
			VkMapA['7'] = 'A';
			VkMapA['8'] = 'S';
			VkMapA['Z'] = 'D';
			VkMapA['A'] = 'F';
			VkMapA['E'] = 'G';
			VkMapA['T'] = 'J';
			VkMapA['D'] = 'K';
			VkMapA['C'] = 'L';
			VkMapA['K'] = myVK_SemiColon;
			VkMapA[myVK_Subtract] = myVK_SingleQuote;
			VkMapA['9'] = 'Z';
			VkMapA['0'] = 'X';
			VkMapA['X'] = 'C';
			VkMapA[myVK_Comma] = 'V';
			VkMapA['I'] = 'B';
			VkMapA['W'] = 'M';
			VkMapA['V'] = myVK_Comma;
			VkMapA['G'] = myVK_Period;
			VkMapA[myVK_SingleQuote] = myVK_Slash;
			break;
#endif
#if 0
		case 0x0000082C:
			/* not in Windows 95 */
			/* Azeri Cyrillic */
			break;
		case 0x00000423:
			/* Belarusian */
			break;
		case 0x00000445:
			/* not in Windows 95 */
			/* Bengali */
			break;
		case 0x00010445:
			/* not in Windows 95 */
			/* Bengali (Inscript) */
			break;
		case 0x0000201A:
			/* not in Windows 95 */
			/* Bosnian Cyrillic*/
			break;
		case 0x00010402:
			/* Bulgarian Latin */
#if 0 /* Only in Windows 95 */
			VkMapA['J'] = 'Q';
			VkMapA['C'] = 'W';
			VkMapA['U'] = 'E';
			VkMapA['K'] = 'R';
			VkMapA['E'] = 'T';
			VkMapA['N'] = 'Y';
			VkMapA['G'] = 'U';
			VkMapA[myVK_SemiColon] = 'I';
			VkMapA[myVK_OEM_102] = 'O';
			VkMapA['Z'] = 'P';
			VkMapA['H'] = myVK_LeftBracket;
			VkMapA['F'] = 'A';
			VkMapA['Y'] = 'S';
			VkMapA['W'] = 'D';
			VkMapA['A'] = 'F';
			VkMapA['P'] = 'G';
			VkMapA['R'] = 'H';
			VkMapA['O'] = 'J';
			VkMapA['L'] = 'K';
			VkMapA['D'] = 'L';
			VkMapA['V'] = myVK_SemiColon;
			VkMapA[myVK_LeftBracket] = 'Z';
			VkMapA['S'] = 'X';
			VkMapA['M'] = 'C';
			VkMapA['I'] = 'V';
			VkMapA['T'] = 'B';
			VkMapA['X'] = 'N';
			VkMapA['B'] = 'M';
			VkMapA['Q'] = myVK_OEM_102;
#endif
			break;
		case 0x00000408:
			/* Greek */
			break;
		case 0x00050408:
			/* Greek Latin */
			break;
		case 0x00060408:
			/* Greek Polytonic */
			break;
		case 0x0000043F:
			/* Kazakh */
			break;
		case 0x00000440:
			/* Kyrgyz Cyrillic */
			break;
		case 0x00010426:
			/* Latvian Latin */
			break;
		case 0x00010427:
			/* Lithuanian */
			break;
		case 0x00000427:
			/* Lithuanian (IBM) */
			break;
		case 0x0000044C:
			/* Malayalam */
			break;
		case 0x0000042f:
			/* Macedonian (FYROM) */
			break;
		case 0x0000043A:
			/* Maltese 47-key */
			break;
		case 0x0001043A:
			/* Maltese 48-key */
			break;
		case 0x00000481:
			/* Maori */
			break;
		case 0x00000450:
			/* Mongolian Cyrillic */
			break;
		case 0x00000461:
			/* Nepali */
			break;
		case 0x00000463:
			/* Pashto */
			break;
		case 0x00000415:
			/* Polish (Programmers) */
			break;
		case 0x00000416:
			/* Porguguese (Brazilian standard) */
			break;
		case 0x00000419:
			/* Russian */
			break;
		case 0x00010419:
			/* Russian (Typewriter) */
			break;
		case 0x00000c1a:
			/* Serbian */
			break;
		case 0x0001041B:
			/* Slovak (Qwerty) */
			break;
		case 0x00000444:
			/* Tatar */
			break;
		case 0x00000422:
			/* Ukrainian */
			break;
		case 0x00020409:
			/* United States International */
			break;
		case 0x00000843:
			/* Uzbek Cyrillic */
			break;
		case 0x00010418:
			/* Romanian (Standard) */
			break;
		case 0x00020418:
			/* Romanian (Programmers) */
			break;
		case 0x00000401:
			/* Arabic (101) */
			break;
		case 0x00010401:
			/* Arabic (102) */
			break;
		case 0x0000044D:
			/* Assamese - INSCRIPT */
			break;
		case 0x0000046D:
			/* Bashkir */
			break;
		case 0x00040402:
			/* Bulgarian (Phonetic Traditional) */
			break;
		case 0x00000404:
			/* Chinese (Traditional) */
			break;
		case 0x00000804:
			/* Chinese (Simplified) */
			break;
		case 0x00000C04:
			/* Chinese (Traditional, Hong Kong S.A.R.) */
			break;
		case 0x00001004:
			/* Chinese (Simplified, Singapore) */
			break;
		case 0x00001404:
			/* Chinese (Traditional, Macao S.A.R.) */
			break;
		case 0x0000040D:
			/* Hebrew */
			break;
		case 0x00000447:
			/* Gujarati */
			break;
		case 0x00000468:
			/* Hausa */
			break;
		case 0x00010439:
			/* Hindi Traditional */
			break;
		case 0x00000439:
			/* Devanagari - INSCRIPT */
			break;
		case 0x00000465:
			/* Divehi Phonetic */
			break;
		case 0x00010465:
			/* Divehi Typewriter */
			break;
		case 0x00000437:
			/* Georgian */
			break;
		case 0x00010437:
			/* Georgian (QWERTY) */
			break;
		case 0x00020437:
			/* Georgian (Ergonomic) */
			break;
		case 0x00000470:
			/* Igbo */
			break;
		case 0x00000411:
			/* Japanese */
			/* VkMapA[??] = ??; */
			break;
		case 0x00000412:
			/* Korean */
			/* VkMapA[VK_ZOOM] = ??; */
			/* VkMapA[VK_HELP] = VK_ZOOM; */
			/* VkMapA[??] = VK_HELP; */
			/* VkMapA[??] = ??; */
			break;
		case 0x0000044B:
			/* Kannada */
			break;
		case 0x00000453:
			/* Khmer */
			break;
		case 0x00000454:
			/* Lao */
			break;
		case 0x00000448:
			/* Oriya */
			break;
		case 0x0000044E:
			/* Marathi */
			break;
		case 0x00000850:
			/* Mongolian (Mongolian Script) */
			break;
		case 0x00000429:
			/* Persion */
			break;
		case 0x00000446:
			/* Punjabi */
			break;
		case 0x0000046C:
			/* Sesotho sa Leboa */
			break;
		case 0x00000432:
			/* Setswana */
			break;
		case 0x0000045B:
			/* Sinhala */
			break;
		case 0x0001045B:
			/* Sinhala - Wij 9 */
			break;
		case 0x0000045A:
			/* Syriac */
			break;
		case 0x0001045A:
			/* Syriac Phonetic */
			break;
		case 0x00000428:
			/* Tajik */
			break;
		case 0x00000449:
			/* Tamil */
			break;
		case 0x0000044A:
			/* Telugu */
			break;
		case 0x0000041E:
			/* Thai Kedmanee */
			break;
		case 0x0001041E:
			/* Thai Pattachote */
			break;
		case 0x0002041E:
			/* Thai Kedmanee (non-ShiftLock) */
			break;
		case 0x0003041E:
			/* Thai Pattachote (non-ShiftLock) */
			break;
		case 0x00000451:
			/* Tibetan (PRC) */
			break;
		case 0x00000442:
			/* Turkmen */
			break;
		case 0x00020422:
			/* Ukrainian (Enhanced) */
			break;
		case 0x00000420:
			/* Urdu */
			break;
		case 0x00050409:
			/* US English Table for IBM Arabic 238_L */
			break;
		case 0x00000480:
			/* Uyghur (Legacy) */
			break;
		case 0x00010480:
			/* Uyghur */
			break;
		case 0x0000042A:
			/* Vietnamese */
			break;
		case 0x00000485:
			/* Yakut */
			break;
		case 0x0000046A:
			/* Yoruba */
			break;
#endif
	}
}
#endif

#if ItnlKyBdFix
LOCALVAR uimr CurKyBdLytNm = 0;
#endif

#if ItnlKyBdFix
LOCALFUNC bool tStrIsHex(TCHAR *s, int n, uimr *r)
{
	short i;
	TCHAR c1;
	TCHAR *p = s;
	uimr v = 0;

	for (i = n; --i >= 0; ) {
		v <<= 4;
		c1 = *p++;
		if ((c1 >= '0') && (c1 <= '9')) {
			v += c1 - '0';
		} else if ((c1 >= 'A') && (c1 <= 'F')) {
			v += c1 - ('A' - 10);
		} else if ((c1 >= 'a') && (c1 <= 'f')) {
			v += c1 - ('a' - 10);
		} else {
			return false;
		}
	}

	*r = v;
	return true;
}
#endif

#if ItnlKyBdFix
LOCALFUNC bool GetKeyboardLayoutHex(uimr *r)
{
	TCHAR s[KL_NAMELENGTH];
	bool IsOk = false;

	if (! GetKeyboardLayoutName(s)) {
		/* ReportWinLastError(); */
	} else {
		size_t n = _tcslen(s);

		if (8 != n) {
			/* fail */
		} else {
			IsOk = tStrIsHex(s, n, r);
		}
	}

	return IsOk;
}
#endif

#if ItnlKyBdFix
void CheckKeyboardLayout(void)
{
	uimr sv;

	if (! GetKeyboardLayoutHex(&sv)) {
	} else if (sv == CurKyBdLytNm) {
		/* no change */
	} else {
		CurKyBdLytNm = sv;

		VkMapFromLayout(sv);
	}
}

void InitCheckKeyboardLayout(void)
{
	uimr sv;

	if (! GetKeyboardLayoutHex(&sv)) {
		sv = 0x00000409;
	}

	CurKyBdLytNm = sv;

	VkMapFromLayout(sv);
}
#endif

