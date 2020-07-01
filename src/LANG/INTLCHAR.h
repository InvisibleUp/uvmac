/*
	LANG/INTLCHAR.h

	Copyright (C) 2010 Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

/*
	InterNaTionAL CHARacters
*/

/* TODO: Move this to a PCF, BDF, or FON bitmap font format */

/* master copy of private font data */
/*
	Data in commments:
	Mini vMac Cell name
	Mac Roman (Octal)
	windows-1252 code page
	Unicode
	plain ascii
	ClStrAppendSubstCStr encoding
	HTML character entity
*/
#pragma once

#ifndef INTLCHAR_H
#define INTLCHAR_H

#include "UI/COMOSGLU.h"
#include "UI/MYOSGLUE.h"

#define ClStrMaxLength 512

extern const uint8_t CellData[];

enum {
	kCellUpA,
	kCellUpB,
	kCellUpC,
	kCellUpD,
	kCellUpE,
	kCellUpF,
	kCellUpG,
	kCellUpH,
	kCellUpI,
	kCellUpJ,
	kCellUpK,
	kCellUpL,
	kCellUpM,
	kCellUpN,
	kCellUpO,
	kCellUpP,
	kCellUpQ,
	kCellUpR,
	kCellUpS,
	kCellUpT,
	kCellUpU,
	kCellUpV,
	kCellUpW,
	kCellUpX,
	kCellUpY,
	kCellUpZ,
	kCellLoA,
	kCellLoB,
	kCellLoC,
	kCellLoD,
	kCellLoE,
	kCellLoF,
	kCellLoG,
	kCellLoH,
	kCellLoI,
	kCellLoJ,
	kCellLoK,
	kCellLoL,
	kCellLoM,
	kCellLoN,
	kCellLoO,
	kCellLoP,
	kCellLoQ,
	kCellLoR,
	kCellLoS,
	kCellLoT,
	kCellLoU,
	kCellLoV,
	kCellLoW,
	kCellLoX,
	kCellLoY,
	kCellLoZ,
	kCellDigit0,
	kCellDigit1,
	kCellDigit2,
	kCellDigit3,
	kCellDigit4,
	kCellDigit5,
	kCellDigit6,
	kCellDigit7,
	kCellDigit8,
	kCellDigit9,
	kCellExclamation,
	kCellAmpersand,
	kCellApostrophe,
	kCellLeftParen,
	kCellRightParen,
	kCellComma,
	kCellHyphen,
	kCellPeriod,
	kCellSlash,
	kCellColon,
	kCellSemicolon,
	kCellQuestion,
	kCellEllipsis,
	kCellUnderscore,
	kCellLeftDQuote,
	kCellRightDQuote,
	kCellLeftSQuote,
	kCellRightSQuote,
	kCellCopyright,
	kCellSpace,

#if NeedIntlChars
	kCellUpADiaeresis,
	kCellUpARing,
	kCellUpCCedilla,
	kCellUpEAcute,
	kCellUpNTilde,
	kCellUpODiaeresis,
	kCellUpUDiaeresis,
	kCellLoAAcute,
	kCellLoAGrave,
	kCellLoACircumflex,
	kCellLoADiaeresis,
	kCellLoATilde,
	kCellLoARing,
	kCellLoCCedilla,
	kCellLoEAcute,
	kCellLoEGrave,
	kCellLoECircumflex,
	kCellLoEDiaeresis,
	kCellLoIAcute,
	kCellLoIGrave,
	kCellLoICircumflex,
	kCellLoIDiaeresis,
	kCellLoNTilde,
	kCellLoOAcute,
	kCellLoOGrave,
	kCellLoOCircumflex,
	kCellLoODiaeresis,
	kCellLoOTilde,
	kCellLoUAcute,
	kCellLoUGrave,
	kCellLoUCircumflex,
	kCellLoUDiaeresis,

	kCellUpAE,
	kCellUpOStroke,

	kCellLoAE,
	kCellLoOStroke,
	kCellInvQuestion,
	kCellInvExclam,

	kCellUpAGrave,
	kCellUpATilde,
	kCellUpOTilde,
	kCellUpLigatureOE,
	kCellLoLigatureOE,

	kCellLoYDiaeresis,
	kCellUpYDiaeresis,

	kCellUpACircumflex,
	kCellUpECircumflex,
	kCellUpAAcute,
	kCellUpEDiaeresis,
	kCellUpEGrave,
	kCellUpIAcute,
	kCellUpICircumflex,
	kCellUpIDiaeresis,
	kCellUpIGrave,
	kCellUpOAcute,
	kCellUpOCircumflex,

	kCellUpOGrave,
	kCellUpUAcute,
	kCellUpUCircumflex,
	kCellUpUGrave,
	kCellSharpS,

	kCellUpACedille,
	kCellLoACedille,
	kCellUpCAcute,
	kCellLoCAcute,
	kCellUpECedille,
	kCellLoECedille,
	kCellUpLBar,
	kCellLoLBar,
	kCellUpNAcute,
	kCellLoNAcute,
	kCellUpSAcute,
	kCellLoSAcute,
	kCellUpZAcute,
	kCellLoZAcute,
	kCellUpZDot,
	kCellLoZDot,
	kCellMidDot,
	kCellUpCCaron,
	kCellLoCCaron,
	kCellLoECaron,
	kCellLoRCaron,
	kCellLoSCaron,
	kCellLoTCaron,
	kCellLoZCaron,
	kCellUpYAcute,
	kCellLoYAcute,
	kCellLoUDblac,
	kCellLoURing,
	kCellUpDStroke,
	kCellLoDStroke,
#endif

	kCellUpperLeft,
	kCellUpperMiddle,
	kCellUpperRight,
	kCellMiddleLeft,
	kCellMiddleRight,
	kCellLowerLeft,
	kCellLowerMiddle,
	kCellLowerRight,
	kCellGraySep,
	kCellIcon00,
	kCellIcon01,
	kCellIcon02,
	kCellIcon03,
	kCellIcon10,
	kCellIcon11,
	kCellIcon12,
	kCellIcon13,
	kCellIcon20,
	kCellIcon21,
	kCellIcon22,
	kCellIcon23,
	kNumCells
};

#ifndef NeedCell2MacAsciiMap
#define NeedCell2MacAsciiMap 1
#endif

/* Mac Roman character set */
extern const char Cell2MacAsciiMap[];
/* Windows character set (windows-1252 code page) */
extern const uint8_t Cell2WinAsciiMap[];
/* Plain ascii - remove accents when possible */
extern const char Cell2PlainAsciiMap[];
/* UTF-16 character set */
extern const uint16_t Cell2UnicodeMap[];

#ifndef kStrCntrlKyName
#define kStrCntrlKyName "control"
#endif
#ifndef kControlModeKey
#define kControlModeKey kStrCntrlKyName
#endif
#ifndef kUnMappedKey
#define kUnMappedKey kStrCntrlKyName
#endif

char * GetSubstitutionStr(char x);
int ClStrSizeSubstCStr(char *s);
void ClStrAppendChar(int *L0, uint8_t *r, uint8_t c);
void ClStrAppendSubstCStr(int *L, uint8_t *r, char *s);
void ClStrFromSubstCStr(int *L, uint8_t *r, char *s);

#endif // INTLCHAR_H
