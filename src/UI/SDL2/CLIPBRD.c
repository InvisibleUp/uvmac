#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h>
#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "UI/COMOSGLU.h"
#include "STRCONST.h"

/* --- clipboard --- */

#if IncludeHostTextClipExchange
static uimr MacRoman2UniCodeSize(uint8_t *s, uimr L)
{
	uimr i;
	uint8_t x;
	uimr n;
	uimr v = 0;

	for (i = 0; i < L; ++i) {
		x = *s++;
		if (x < 128) {
			n = 1;
		} else {
			switch (x) {
				case 0x80: n = 2; break;
					/* LATIN CAPITAL LETTER A WITH DIAERESIS */
				case 0x81: n = 2; break;
					/* LATIN CAPITAL LETTER A WITH RING ABOVE */
				case 0x82: n = 2; break;
					/* LATIN CAPITAL LETTER C WITH CEDILLA */
				case 0x83: n = 2; break;
					/* LATIN CAPITAL LETTER E WITH ACUTE */
				case 0x84: n = 2; break;
					/* LATIN CAPITAL LETTER N WITH TILDE */
				case 0x85: n = 2; break;
					/* LATIN CAPITAL LETTER O WITH DIAERESIS */
				case 0x86: n = 2; break;
					/* LATIN CAPITAL LETTER U WITH DIAERESIS */
				case 0x87: n = 2; break;
					/* LATIN SMALL LETTER A WITH ACUTE */
				case 0x88: n = 2; break;
					/* LATIN SMALL LETTER A WITH GRAVE */
				case 0x89: n = 2; break;
					/* LATIN SMALL LETTER A WITH CIRCUMFLEX */
				case 0x8A: n = 2; break;
					/* LATIN SMALL LETTER A WITH DIAERESIS */
				case 0x8B: n = 2; break;
					/* LATIN SMALL LETTER A WITH TILDE */
				case 0x8C: n = 2; break;
					/* LATIN SMALL LETTER A WITH RING ABOVE */
				case 0x8D: n = 2; break;
					/* LATIN SMALL LETTER C WITH CEDILLA */
				case 0x8E: n = 2; break;
					/* LATIN SMALL LETTER E WITH ACUTE */
				case 0x8F: n = 2; break;
					/* LATIN SMALL LETTER E WITH GRAVE */
				case 0x90: n = 2; break;
					/* LATIN SMALL LETTER E WITH CIRCUMFLEX */
				case 0x91: n = 2; break;
					/* LATIN SMALL LETTER E WITH DIAERESIS */
				case 0x92: n = 2; break;
					/* LATIN SMALL LETTER I WITH ACUTE */
				case 0x93: n = 2; break;
					/* LATIN SMALL LETTER I WITH GRAVE */
				case 0x94: n = 2; break;
					/* LATIN SMALL LETTER I WITH CIRCUMFLEX */
				case 0x95: n = 2; break;
					/* LATIN SMALL LETTER I WITH DIAERESIS */
				case 0x96: n = 2; break;
					/* LATIN SMALL LETTER N WITH TILDE */
				case 0x97: n = 2; break;
					/* LATIN SMALL LETTER O WITH ACUTE */
				case 0x98: n = 2; break;
					/* LATIN SMALL LETTER O WITH GRAVE */
				case 0x99: n = 2; break;
					/* LATIN SMALL LETTER O WITH CIRCUMFLEX */
				case 0x9A: n = 2; break;
					/* LATIN SMALL LETTER O WITH DIAERESIS */
				case 0x9B: n = 2; break;
					/* LATIN SMALL LETTER O WITH TILDE */
				case 0x9C: n = 2; break;
					/* LATIN SMALL LETTER U WITH ACUTE */
				case 0x9D: n = 2; break;
					/* LATIN SMALL LETTER U WITH GRAVE */
				case 0x9E: n = 2; break;
					/* LATIN SMALL LETTER U WITH CIRCUMFLEX */
				case 0x9F: n = 2; break;
					/* LATIN SMALL LETTER U WITH DIAERESIS */
				case 0xA0: n = 3; break;
					/* DAGGER */
				case 0xA1: n = 2; break;
					/* DEGREE SIGN */
				case 0xA2: n = 2; break;
					/* CENT SIGN */
				case 0xA3: n = 2; break;
					/* POUND SIGN */
				case 0xA4: n = 2; break;
					/* SECTION SIGN */
				case 0xA5: n = 3; break;
					/* BULLET */
				case 0xA6: n = 2; break;
					/* PILCROW SIGN */
				case 0xA7: n = 2; break;
					/* LATIN SMALL LETTER SHARP S */
				case 0xA8: n = 2; break;
					/* REGISTERED SIGN */
				case 0xA9: n = 2; break;
					/* COPYRIGHT SIGN */
				case 0xAA: n = 3; break;
					/* TRADE MARK SIGN */
				case 0xAB: n = 2; break;
					/* ACUTE ACCENT */
				case 0xAC: n = 2; break;
					/* DIAERESIS */
				case 0xAD: n = 3; break;
					/* NOT EQUAL TO */
				case 0xAE: n = 2; break;
					/* LATIN CAPITAL LETTER AE */
				case 0xAF: n = 2; break;
					/* LATIN CAPITAL LETTER O WITH STROKE */
				case 0xB0: n = 3; break;
					/* INFINITY */
				case 0xB1: n = 2; break;
					/* PLUS-MINUS SIGN */
				case 0xB2: n = 3; break;
					/* LESS-THAN OR EQUAL TO */
				case 0xB3: n = 3; break;
					/* GREATER-THAN OR EQUAL TO */
				case 0xB4: n = 2; break;
					/* YEN SIGN */
				case 0xB5: n = 2; break;
					/* MICRO SIGN */
				case 0xB6: n = 3; break;
					/* PARTIAL DIFFERENTIAL */
				case 0xB7: n = 3; break;
					/* N-ARY SUMMATION */
				case 0xB8: n = 3; break;
					/* N-ARY PRODUCT */
				case 0xB9: n = 2; break;
					/* GREEK SMALL LETTER PI */
				case 0xBA: n = 3; break;
					/* INTEGRAL */
				case 0xBB: n = 2; break;
					/* FEMININE ORDINAL INDICATOR */
				case 0xBC: n = 2; break;
					/* MASCULINE ORDINAL INDICATOR */
				case 0xBD: n = 2; break;
					/* GREEK CAPITAL LETTER OMEGA */
				case 0xBE: n = 2; break;
					/* LATIN SMALL LETTER AE */
				case 0xBF: n = 2; break;
					/* LATIN SMALL LETTER O WITH STROKE */
				case 0xC0: n = 2; break;
					/* INVERTED QUESTION MARK */
				case 0xC1: n = 2; break;
					/* INVERTED EXCLAMATION MARK */
				case 0xC2: n = 2; break;
					/* NOT SIGN */
				case 0xC3: n = 3; break;
					/* SQUARE ROOT */
				case 0xC4: n = 2; break;
					/* LATIN SMALL LETTER F WITH HOOK */
				case 0xC5: n = 3; break;
					/* ALMOST EQUAL TO */
				case 0xC6: n = 3; break;
					/* INCREMENT */
				case 0xC7: n = 2; break;
					/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
				case 0xC8: n = 2; break;
					/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
				case 0xC9: n = 3; break;
					/* HORIZONTAL ELLIPSIS */
				case 0xCA: n = 2; break;
					/* NO-BREAK SPACE */
				case 0xCB: n = 2; break;
					/* LATIN CAPITAL LETTER A WITH GRAVE */
				case 0xCC: n = 2; break;
					/* LATIN CAPITAL LETTER A WITH TILDE */
				case 0xCD: n = 2; break;
					/* LATIN CAPITAL LETTER O WITH TILDE */
				case 0xCE: n = 2; break;
					/* LATIN CAPITAL LIGATURE OE */
				case 0xCF: n = 2; break;
					/* LATIN SMALL LIGATURE OE */
				case 0xD0: n = 3; break;
					/* EN DASH */
				case 0xD1: n = 3; break;
					/* EM DASH */
				case 0xD2: n = 3; break;
					/* LEFT DOUBLE QUOTATION MARK */
				case 0xD3: n = 3; break;
					/* RIGHT DOUBLE QUOTATION MARK */
				case 0xD4: n = 3; break;
					/* LEFT SINGLE QUOTATION MARK */
				case 0xD5: n = 3; break;
					/* RIGHT SINGLE QUOTATION MARK */
				case 0xD6: n = 2; break;
					/* DIVISION SIGN */
				case 0xD7: n = 3; break;
					/* LOZENGE */
				case 0xD8: n = 2; break;
					/* LATIN SMALL LETTER Y WITH DIAERESIS */
				case 0xD9: n = 2; break;
					/* LATIN CAPITAL LETTER Y WITH DIAERESIS */
				case 0xDA: n = 3; break;
					/* FRACTION SLASH */
				case 0xDB: n = 3; break;
					/* EURO SIGN */
				case 0xDC: n = 3; break;
					/* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
				case 0xDD: n = 3; break;
					/* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
				case 0xDE: n = 3; break;
					/* LATIN SMALL LIGATURE FI */
				case 0xDF: n = 3; break;
					/* LATIN SMALL LIGATURE FL */
				case 0xE0: n = 3; break;
					/* DOUBLE DAGGER */
				case 0xE1: n = 2; break;
					/* MIDDLE DOT */
				case 0xE2: n = 3; break;
					/* SINGLE LOW-9 QUOTATION MARK */
				case 0xE3: n = 3; break;
					/* DOUBLE LOW-9 QUOTATION MARK */
				case 0xE4: n = 3; break;
					/* PER MILLE SIGN */
				case 0xE5: n = 2; break;
					/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
				case 0xE6: n = 2; break;
					/* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
				case 0xE7: n = 2; break;
					/* LATIN CAPITAL LETTER A WITH ACUTE */
				case 0xE8: n = 2; break;
					/* LATIN CAPITAL LETTER E WITH DIAERESIS */
				case 0xE9: n = 2; break;
					/* LATIN CAPITAL LETTER E WITH GRAVE */
				case 0xEA: n = 2; break;
					/* LATIN CAPITAL LETTER I WITH ACUTE */
				case 0xEB: n = 2; break;
					/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
				case 0xEC: n = 2; break;
					/* LATIN CAPITAL LETTER I WITH DIAERESIS */
				case 0xED: n = 2; break;
					/* LATIN CAPITAL LETTER I WITH GRAVE */
				case 0xEE: n = 2; break;
					/* LATIN CAPITAL LETTER O WITH ACUTE */
				case 0xEF: n = 2; break;
					/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
				case 0xF0: n = 3; break;
					/* Apple logo */
				case 0xF1: n = 2; break;
					/* LATIN CAPITAL LETTER O WITH GRAVE */
				case 0xF2: n = 2; break;
					/* LATIN CAPITAL LETTER U WITH ACUTE */
				case 0xF3: n = 2; break;
					/* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
				case 0xF4: n = 2; break;
					/* LATIN CAPITAL LETTER U WITH GRAVE */
				case 0xF5: n = 2; break;
					/* LATIN SMALL LETTER DOTLESS I */
				case 0xF6: n = 2; break;
					/* MODIFIER LETTER CIRCUMFLEX ACCENT */
				case 0xF7: n = 2; break;
					/* SMALL TILDE */
				case 0xF8: n = 2; break;
					/* MACRON */
				case 0xF9: n = 2; break;
					/* BREVE */
				case 0xFA: n = 2; break;
					/* DOT ABOVE */
				case 0xFB: n = 2; break;
					/* RING ABOVE */
				case 0xFC: n = 2; break;
					/* CEDILLA */
				case 0xFD: n = 2; break;
					/* DOUBLE ACUTE ACCENT */
				case 0xFE: n = 2; break;
					/* OGONEK */
				case 0xFF: n = 2; break;
					/* CARON */
				default: n = 1; break;
					/* shouldn't get here */
			}
		}
		v += n;
	}

	return v;
}
#endif

#if IncludeHostTextClipExchange
static void MacRoman2UniCodeData(uint8_t *s, uimr L, char *t)
{
	uimr i;
	uint8_t x;

	for (i = 0; i < L; ++i) {
		x = *s++;
		if (x < 128) {
			*t++ = x;
		} else {
			switch (x) {
				case 0x80: *t++ = 0xC3; *t++ = 0x84; break;
					/* LATIN CAPITAL LETTER A WITH DIAERESIS */
				case 0x81: *t++ = 0xC3; *t++ = 0x85; break;
					/* LATIN CAPITAL LETTER A WITH RING ABOVE */
				case 0x82: *t++ = 0xC3; *t++ = 0x87; break;
					/* LATIN CAPITAL LETTER C WITH CEDILLA */
				case 0x83: *t++ = 0xC3; *t++ = 0x89; break;
					/* LATIN CAPITAL LETTER E WITH ACUTE */
				case 0x84: *t++ = 0xC3; *t++ = 0x91; break;
					/* LATIN CAPITAL LETTER N WITH TILDE */
				case 0x85: *t++ = 0xC3; *t++ = 0x96; break;
					/* LATIN CAPITAL LETTER O WITH DIAERESIS */
				case 0x86: *t++ = 0xC3; *t++ = 0x9C; break;
					/* LATIN CAPITAL LETTER U WITH DIAERESIS */
				case 0x87: *t++ = 0xC3; *t++ = 0xA1; break;
					/* LATIN SMALL LETTER A WITH ACUTE */
				case 0x88: *t++ = 0xC3; *t++ = 0xA0; break;
					/* LATIN SMALL LETTER A WITH GRAVE */
				case 0x89: *t++ = 0xC3; *t++ = 0xA2; break;
					/* LATIN SMALL LETTER A WITH CIRCUMFLEX */
				case 0x8A: *t++ = 0xC3; *t++ = 0xA4; break;
					/* LATIN SMALL LETTER A WITH DIAERESIS */
				case 0x8B: *t++ = 0xC3; *t++ = 0xA3; break;
					/* LATIN SMALL LETTER A WITH TILDE */
				case 0x8C: *t++ = 0xC3; *t++ = 0xA5; break;
					/* LATIN SMALL LETTER A WITH RING ABOVE */
				case 0x8D: *t++ = 0xC3; *t++ = 0xA7; break;
					/* LATIN SMALL LETTER C WITH CEDILLA */
				case 0x8E: *t++ = 0xC3; *t++ = 0xA9; break;
					/* LATIN SMALL LETTER E WITH ACUTE */
				case 0x8F: *t++ = 0xC3; *t++ = 0xA8; break;
					/* LATIN SMALL LETTER E WITH GRAVE */
				case 0x90: *t++ = 0xC3; *t++ = 0xAA; break;
					/* LATIN SMALL LETTER E WITH CIRCUMFLEX */
				case 0x91: *t++ = 0xC3; *t++ = 0xAB; break;
					/* LATIN SMALL LETTER E WITH DIAERESIS */
				case 0x92: *t++ = 0xC3; *t++ = 0xAD; break;
					/* LATIN SMALL LETTER I WITH ACUTE */
				case 0x93: *t++ = 0xC3; *t++ = 0xAC; break;
					/* LATIN SMALL LETTER I WITH GRAVE */
				case 0x94: *t++ = 0xC3; *t++ = 0xAE; break;
					/* LATIN SMALL LETTER I WITH CIRCUMFLEX */
				case 0x95: *t++ = 0xC3; *t++ = 0xAF; break;
					/* LATIN SMALL LETTER I WITH DIAERESIS */
				case 0x96: *t++ = 0xC3; *t++ = 0xB1; break;
					/* LATIN SMALL LETTER N WITH TILDE */
				case 0x97: *t++ = 0xC3; *t++ = 0xB3; break;
					/* LATIN SMALL LETTER O WITH ACUTE */
				case 0x98: *t++ = 0xC3; *t++ = 0xB2; break;
					/* LATIN SMALL LETTER O WITH GRAVE */
				case 0x99: *t++ = 0xC3; *t++ = 0xB4; break;
					/* LATIN SMALL LETTER O WITH CIRCUMFLEX */
				case 0x9A: *t++ = 0xC3; *t++ = 0xB6; break;
					/* LATIN SMALL LETTER O WITH DIAERESIS */
				case 0x9B: *t++ = 0xC3; *t++ = 0xB5; break;
					/* LATIN SMALL LETTER O WITH TILDE */
				case 0x9C: *t++ = 0xC3; *t++ = 0xBA; break;
					/* LATIN SMALL LETTER U WITH ACUTE */
				case 0x9D: *t++ = 0xC3; *t++ = 0xB9; break;
					/* LATIN SMALL LETTER U WITH GRAVE */
				case 0x9E: *t++ = 0xC3; *t++ = 0xBB; break;
					/* LATIN SMALL LETTER U WITH CIRCUMFLEX */
				case 0x9F: *t++ = 0xC3; *t++ = 0xBC; break;
					/* LATIN SMALL LETTER U WITH DIAERESIS */
				case 0xA0: *t++ = 0xE2; *t++ = 0x80; *t++ = 0xA0; break;
					/* DAGGER */
				case 0xA1: *t++ = 0xC2; *t++ = 0xB0; break;
					/* DEGREE SIGN */
				case 0xA2: *t++ = 0xC2; *t++ = 0xA2; break;
					/* CENT SIGN */
				case 0xA3: *t++ = 0xC2; *t++ = 0xA3; break;
					/* POUND SIGN */
				case 0xA4: *t++ = 0xC2; *t++ = 0xA7; break;
					/* SECTION SIGN */
				case 0xA5: *t++ = 0xE2; *t++ = 0x80; *t++ = 0xA2; break;
					/* BULLET */
				case 0xA6: *t++ = 0xC2; *t++ = 0xB6; break;
					/* PILCROW SIGN */
				case 0xA7: *t++ = 0xC3; *t++ = 0x9F; break;
					/* LATIN SMALL LETTER SHARP S */
				case 0xA8: *t++ = 0xC2; *t++ = 0xAE; break;
					/* REGISTERED SIGN */
				case 0xA9: *t++ = 0xC2; *t++ = 0xA9; break;
					/* COPYRIGHT SIGN */
				case 0xAA: *t++ = 0xE2; *t++ = 0x84; *t++ = 0xA2; break;
					/* TRADE MARK SIGN */
				case 0xAB: *t++ = 0xC2; *t++ = 0xB4; break;
					/* ACUTE ACCENT */
				case 0xAC: *t++ = 0xC2; *t++ = 0xA8; break;
					/* DIAERESIS */
				case 0xAD: *t++ = 0xE2; *t++ = 0x89; *t++ = 0xA0; break;
					/* NOT EQUAL TO */
				case 0xAE: *t++ = 0xC3; *t++ = 0x86; break;
					/* LATIN CAPITAL LETTER AE */
				case 0xAF: *t++ = 0xC3; *t++ = 0x98; break;
					/* LATIN CAPITAL LETTER O WITH STROKE */
				case 0xB0: *t++ = 0xE2; *t++ = 0x88; *t++ = 0x9E; break;
					/* INFINITY */
				case 0xB1: *t++ = 0xC2; *t++ = 0xB1; break;
					/* PLUS-MINUS SIGN */
				case 0xB2: *t++ = 0xE2; *t++ = 0x89; *t++ = 0xA4; break;
					/* LESS-THAN OR EQUAL TO */
				case 0xB3: *t++ = 0xE2; *t++ = 0x89; *t++ = 0xA5; break;
					/* GREATER-THAN OR EQUAL TO */
				case 0xB4: *t++ = 0xC2; *t++ = 0xA5; break;
					/* YEN SIGN */
				case 0xB5: *t++ = 0xC2; *t++ = 0xB5; break;
					/* MICRO SIGN */
				case 0xB6: *t++ = 0xE2; *t++ = 0x88; *t++ = 0x82; break;
					/* PARTIAL DIFFERENTIAL */
				case 0xB7: *t++ = 0xE2; *t++ = 0x88; *t++ = 0x91; break;
					/* N-ARY SUMMATION */
				case 0xB8: *t++ = 0xE2; *t++ = 0x88; *t++ = 0x8F; break;
					/* N-ARY PRODUCT */
				case 0xB9: *t++ = 0xCF; *t++ = 0x80; break;
					/* GREEK SMALL LETTER PI */
				case 0xBA: *t++ = 0xE2; *t++ = 0x88; *t++ = 0xAB; break;
					/* INTEGRAL */
				case 0xBB: *t++ = 0xC2; *t++ = 0xAA; break;
					/* FEMININE ORDINAL INDICATOR */
				case 0xBC: *t++ = 0xC2; *t++ = 0xBA; break;
					/* MASCULINE ORDINAL INDICATOR */
				case 0xBD: *t++ = 0xCE; *t++ = 0xA9; break;
					/* GREEK CAPITAL LETTER OMEGA */
				case 0xBE: *t++ = 0xC3; *t++ = 0xA6; break;
					/* LATIN SMALL LETTER AE */
				case 0xBF: *t++ = 0xC3; *t++ = 0xB8; break;
					/* LATIN SMALL LETTER O WITH STROKE */
				case 0xC0: *t++ = 0xC2; *t++ = 0xBF; break;
					/* INVERTED QUESTION MARK */
				case 0xC1: *t++ = 0xC2; *t++ = 0xA1; break;
					/* INVERTED EXCLAMATION MARK */
				case 0xC2: *t++ = 0xC2; *t++ = 0xAC; break;
					/* NOT SIGN */
				case 0xC3: *t++ = 0xE2; *t++ = 0x88; *t++ = 0x9A; break;
					/* SQUARE ROOT */
				case 0xC4: *t++ = 0xC6; *t++ = 0x92; break;
					/* LATIN SMALL LETTER F WITH HOOK */
				case 0xC5: *t++ = 0xE2; *t++ = 0x89; *t++ = 0x88; break;
					/* ALMOST EQUAL TO */
				case 0xC6: *t++ = 0xE2; *t++ = 0x88; *t++ = 0x86; break;
					/* INCREMENT */
				case 0xC7: *t++ = 0xC2; *t++ = 0xAB; break;
					/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
				case 0xC8: *t++ = 0xC2; *t++ = 0xBB; break;
					/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
				case 0xC9: *t++ = 0xE2; *t++ = 0x80; *t++ = 0xA6; break;
					/* HORIZONTAL ELLIPSIS */
				case 0xCA: *t++ = 0xC2; *t++ = 0xA0; break;
					/* NO-BREAK SPACE */
				case 0xCB: *t++ = 0xC3; *t++ = 0x80; break;
					/* LATIN CAPITAL LETTER A WITH GRAVE */
				case 0xCC: *t++ = 0xC3; *t++ = 0x83; break;
					/* LATIN CAPITAL LETTER A WITH TILDE */
				case 0xCD: *t++ = 0xC3; *t++ = 0x95; break;
					/* LATIN CAPITAL LETTER O WITH TILDE */
				case 0xCE: *t++ = 0xC5; *t++ = 0x92; break;
					/* LATIN CAPITAL LIGATURE OE */
				case 0xCF: *t++ = 0xC5; *t++ = 0x93; break;
					/* LATIN SMALL LIGATURE OE */
				case 0xD0: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x93; break;
					/* EN DASH */
				case 0xD1: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x94; break;
					/* EM DASH */
				case 0xD2: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x9C; break;
					/* LEFT DOUBLE QUOTATION MARK */
				case 0xD3: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x9D; break;
					/* RIGHT DOUBLE QUOTATION MARK */
				case 0xD4: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x98; break;
					/* LEFT SINGLE QUOTATION MARK */
				case 0xD5: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x99; break;
					/* RIGHT SINGLE QUOTATION MARK */
				case 0xD6: *t++ = 0xC3; *t++ = 0xB7; break;
					/* DIVISION SIGN */
				case 0xD7: *t++ = 0xE2; *t++ = 0x97; *t++ = 0x8A; break;
					/* LOZENGE */
				case 0xD8: *t++ = 0xC3; *t++ = 0xBF; break;
					/* LATIN SMALL LETTER Y WITH DIAERESIS */
				case 0xD9: *t++ = 0xC5; *t++ = 0xB8; break;
					/* LATIN CAPITAL LETTER Y WITH DIAERESIS */
				case 0xDA: *t++ = 0xE2; *t++ = 0x81; *t++ = 0x84; break;
					/* FRACTION SLASH */
				case 0xDB: *t++ = 0xE2; *t++ = 0x82; *t++ = 0xAC; break;
					/* EURO SIGN */
				case 0xDC: *t++ = 0xE2; *t++ = 0x80; *t++ = 0xB9; break;
					/* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
				case 0xDD: *t++ = 0xE2; *t++ = 0x80; *t++ = 0xBA; break;
					/* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
				case 0xDE: *t++ = 0xEF; *t++ = 0xAC; *t++ = 0x81; break;
					/* LATIN SMALL LIGATURE FI */
				case 0xDF: *t++ = 0xEF; *t++ = 0xAC; *t++ = 0x82; break;
					/* LATIN SMALL LIGATURE FL */
				case 0xE0: *t++ = 0xE2; *t++ = 0x80; *t++ = 0xA1; break;
					/* DOUBLE DAGGER */
				case 0xE1: *t++ = 0xC2; *t++ = 0xB7; break;
					/* MIDDLE DOT */
				case 0xE2: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x9A; break;
					/* SINGLE LOW-9 QUOTATION MARK */
				case 0xE3: *t++ = 0xE2; *t++ = 0x80; *t++ = 0x9E; break;
					/* DOUBLE LOW-9 QUOTATION MARK */
				case 0xE4: *t++ = 0xE2; *t++ = 0x80; *t++ = 0xB0; break;
					/* PER MILLE SIGN */
				case 0xE5: *t++ = 0xC3; *t++ = 0x82; break;
					/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
				case 0xE6: *t++ = 0xC3; *t++ = 0x8A; break;
					/* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
				case 0xE7: *t++ = 0xC3; *t++ = 0x81; break;
					/* LATIN CAPITAL LETTER A WITH ACUTE */
				case 0xE8: *t++ = 0xC3; *t++ = 0x8B; break;
					/* LATIN CAPITAL LETTER E WITH DIAERESIS */
				case 0xE9: *t++ = 0xC3; *t++ = 0x88; break;
					/* LATIN CAPITAL LETTER E WITH GRAVE */
				case 0xEA: *t++ = 0xC3; *t++ = 0x8D; break;
					/* LATIN CAPITAL LETTER I WITH ACUTE */
				case 0xEB: *t++ = 0xC3; *t++ = 0x8E; break;
					/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
				case 0xEC: *t++ = 0xC3; *t++ = 0x8F; break;
					/* LATIN CAPITAL LETTER I WITH DIAERESIS */
				case 0xED: *t++ = 0xC3; *t++ = 0x8C; break;
					/* LATIN CAPITAL LETTER I WITH GRAVE */
				case 0xEE: *t++ = 0xC3; *t++ = 0x93; break;
					/* LATIN CAPITAL LETTER O WITH ACUTE */
				case 0xEF: *t++ = 0xC3; *t++ = 0x94; break;
					/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
				case 0xF0: *t++ = 0xEF; *t++ = 0xA3; *t++ = 0xBF; break;
					/* Apple logo */
				case 0xF1: *t++ = 0xC3; *t++ = 0x92; break;
					/* LATIN CAPITAL LETTER O WITH GRAVE */
				case 0xF2: *t++ = 0xC3; *t++ = 0x9A; break;
					/* LATIN CAPITAL LETTER U WITH ACUTE */
				case 0xF3: *t++ = 0xC3; *t++ = 0x9B; break;
					/* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
				case 0xF4: *t++ = 0xC3; *t++ = 0x99; break;
					/* LATIN CAPITAL LETTER U WITH GRAVE */
				case 0xF5: *t++ = 0xC4; *t++ = 0xB1; break;
					/* LATIN SMALL LETTER DOTLESS I */
				case 0xF6: *t++ = 0xCB; *t++ = 0x86; break;
					/* MODIFIER LETTER CIRCUMFLEX ACCENT */
				case 0xF7: *t++ = 0xCB; *t++ = 0x9C; break;
					/* SMALL TILDE */
				case 0xF8: *t++ = 0xC2; *t++ = 0xAF; break;
					/* MACRON */
				case 0xF9: *t++ = 0xCB; *t++ = 0x98; break;
					/* BREVE */
				case 0xFA: *t++ = 0xCB; *t++ = 0x99; break;
					/* DOT ABOVE */
				case 0xFB: *t++ = 0xCB; *t++ = 0x9A; break;
					/* RING ABOVE */
				case 0xFC: *t++ = 0xC2; *t++ = 0xB8; break;
					/* CEDILLA */
				case 0xFD: *t++ = 0xCB; *t++ = 0x9D; break;
					/* DOUBLE ACUTE ACCENT */
				case 0xFE: *t++ = 0xCB; *t++ = 0x9B; break;
					/* OGONEK */
				case 0xFF: *t++ = 0xCB; *t++ = 0x87; break;
					/* CARON */
				default: *t++ = '?'; break;
					/* shouldn't get here */
			}
		}
	}
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC MacErr_t HTCEexport(tPbuf i)
{
	MacErr_t err;
	char *p;
	uint8_t * s = PbufDat[i];
	uimr L = PbufSize[i];
	uimr sz = MacRoman2UniCodeSize(s, L);

	if (NULL == (p = malloc(sz + 1))) {
		err = mnvm_miscErr;
	} else {
		MacRoman2UniCodeData(s, L, p);
		p[sz] = 0;

		if (0 != SDL_SetClipboardText(p)) {
			err = mnvm_miscErr;
		} else {
			err = mnvm_noErr;
		}
		free(p);
	}

	return err;
}
#endif

#if IncludeHostTextClipExchange
static MacErr_t UniCodeStrLength(char *s, uimr *r)
{
	MacErr_t err;
	uint8_t t;
	uint8_t t2;
	char *p = s;
	uimr L = 0;

label_retry:
	if (0 == (t = *p++)) {
		err = mnvm_noErr;
		/* done */
	} else
	if (0 == (0x80 & t)) {
		/* One-byte code */
		L += 1;
		goto label_retry;
	} else
	if (0 == (0x40 & t)) {
		/* continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (t2 = *p++)) {
		err = mnvm_miscErr;
	} else
	if (0x80 != (0xC0 & t2)) {
		/* not a continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (0x20 & t)) {
		/* two bytes */
		L += 2;
		goto label_retry;
	} else
	if (0 == (t2 = *p++)) {
		err = mnvm_miscErr;
	} else
	if (0x80 != (0xC0 & t2)) {
		/* not a continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (0x10 & t)) {
		/* three bytes */
		L += 3;
		goto label_retry;
	} else
	if (0 == (t2 = *p++)) {
		err = mnvm_miscErr;
	} else
	if (0x80 != (0xC0 & t2)) {
		/* not a continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (0x08 & t)) {
		/* four bytes */
		L += 5;
		goto label_retry;
	} else
	{
		err = mnvm_miscErr;
		/* longer code not supported yet */
	}

	*r = L;
	return err;
}
#endif

#if IncludeHostTextClipExchange
static uint8_t UniCodePoint2MacRoman(uint32_t x)
{
/*
	adapted from
		http://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT
*/
	uint8_t y;

	if (x < 128) {
		y = x;
	} else {
		switch (x) {
			case 0x00C4: y = 0x80; break;
				/* LATIN CAPITAL LETTER A WITH DIAERESIS */
			case 0x00C5: y = 0x81; break;
				/* LATIN CAPITAL LETTER A WITH RING ABOVE */
			case 0x00C7: y = 0x82; break;
				/* LATIN CAPITAL LETTER C WITH CEDILLA */
			case 0x00C9: y = 0x83; break;
				/* LATIN CAPITAL LETTER E WITH ACUTE */
			case 0x00D1: y = 0x84; break;
				/* LATIN CAPITAL LETTER N WITH TILDE */
			case 0x00D6: y = 0x85; break;
				/* LATIN CAPITAL LETTER O WITH DIAERESIS */
			case 0x00DC: y = 0x86; break;
				/* LATIN CAPITAL LETTER U WITH DIAERESIS */
			case 0x00E1: y = 0x87; break;
				/* LATIN SMALL LETTER A WITH ACUTE */
			case 0x00E0: y = 0x88; break;
				/* LATIN SMALL LETTER A WITH GRAVE */
			case 0x00E2: y = 0x89; break;
				/* LATIN SMALL LETTER A WITH CIRCUMFLEX */
			case 0x00E4: y = 0x8A; break;
				/* LATIN SMALL LETTER A WITH DIAERESIS */
			case 0x00E3: y = 0x8B; break;
				/* LATIN SMALL LETTER A WITH TILDE */
			case 0x00E5: y = 0x8C; break;
				/* LATIN SMALL LETTER A WITH RING ABOVE */
			case 0x00E7: y = 0x8D; break;
				/* LATIN SMALL LETTER C WITH CEDILLA */
			case 0x00E9: y = 0x8E; break;
				/* LATIN SMALL LETTER E WITH ACUTE */
			case 0x00E8: y = 0x8F; break;
				/* LATIN SMALL LETTER E WITH GRAVE */
			case 0x00EA: y = 0x90; break;
				/* LATIN SMALL LETTER E WITH CIRCUMFLEX */
			case 0x00EB: y = 0x91; break;
				/* LATIN SMALL LETTER E WITH DIAERESIS */
			case 0x00ED: y = 0x92; break;
				/* LATIN SMALL LETTER I WITH ACUTE */
			case 0x00EC: y = 0x93; break;
				/* LATIN SMALL LETTER I WITH GRAVE */
			case 0x00EE: y = 0x94; break;
				/* LATIN SMALL LETTER I WITH CIRCUMFLEX */
			case 0x00EF: y = 0x95; break;
				/* LATIN SMALL LETTER I WITH DIAERESIS */
			case 0x00F1: y = 0x96; break;
				/* LATIN SMALL LETTER N WITH TILDE */
			case 0x00F3: y = 0x97; break;
				/* LATIN SMALL LETTER O WITH ACUTE */
			case 0x00F2: y = 0x98; break;
				/* LATIN SMALL LETTER O WITH GRAVE */
			case 0x00F4: y = 0x99; break;
				/* LATIN SMALL LETTER O WITH CIRCUMFLEX */
			case 0x00F6: y = 0x9A; break;
				/* LATIN SMALL LETTER O WITH DIAERESIS */
			case 0x00F5: y = 0x9B; break;
				/* LATIN SMALL LETTER O WITH TILDE */
			case 0x00FA: y = 0x9C; break;
				/* LATIN SMALL LETTER U WITH ACUTE */
			case 0x00F9: y = 0x9D; break;
				/* LATIN SMALL LETTER U WITH GRAVE */
			case 0x00FB: y = 0x9E; break;
				/* LATIN SMALL LETTER U WITH CIRCUMFLEX */
			case 0x00FC: y = 0x9F; break;
				/* LATIN SMALL LETTER U WITH DIAERESIS */
			case 0x2020: y = 0xA0; break;
				/* DAGGER */
			case 0x00B0: y = 0xA1; break;
				/* DEGREE SIGN */
			case 0x00A2: y = 0xA2; break;
				/* CENT SIGN */
			case 0x00A3: y = 0xA3; break;
				/* POUND SIGN */
			case 0x00A7: y = 0xA4; break;
				/* SECTION SIGN */
			case 0x2022: y = 0xA5; break;
				/* BULLET */
			case 0x00B6: y = 0xA6; break;
				/* PILCROW SIGN */
			case 0x00DF: y = 0xA7; break;
				/* LATIN SMALL LETTER SHARP S */
			case 0x00AE: y = 0xA8; break;
				/* REGISTERED SIGN */
			case 0x00A9: y = 0xA9; break;
				/* COPYRIGHT SIGN */
			case 0x2122: y = 0xAA; break;
				/* TRADE MARK SIGN */
			case 0x00B4: y = 0xAB; break;
				/* ACUTE ACCENT */
			case 0x00A8: y = 0xAC; break;
				/* DIAERESIS */
			case 0x2260: y = 0xAD; break;
				/* NOT EQUAL TO */
			case 0x00C6: y = 0xAE; break;
				/* LATIN CAPITAL LETTER AE */
			case 0x00D8: y = 0xAF; break;
				/* LATIN CAPITAL LETTER O WITH STROKE */
			case 0x221E: y = 0xB0; break;
				/* INFINITY */
			case 0x00B1: y = 0xB1; break;
				/* PLUS-MINUS SIGN */
			case 0x2264: y = 0xB2; break;
				/* LESS-THAN OR EQUAL TO */
			case 0x2265: y = 0xB3; break;
				/* GREATER-THAN OR EQUAL TO */
			case 0x00A5: y = 0xB4; break;
				/* YEN SIGN */
			case 0x00B5: y = 0xB5; break;
				/* MICRO SIGN */
			case 0x2202: y = 0xB6; break;
				/* PARTIAL DIFFERENTIAL */
			case 0x2211: y = 0xB7; break;
				/* N-ARY SUMMATION */
			case 0x220F: y = 0xB8; break;
				/* N-ARY PRODUCT */
			case 0x03C0: y = 0xB9; break;
				/* GREEK SMALL LETTER PI */
			case 0x222B: y = 0xBA; break;
				/* INTEGRAL */
			case 0x00AA: y = 0xBB; break;
				/* FEMININE ORDINAL INDICATOR */
			case 0x00BA: y = 0xBC; break;
				/* MASCULINE ORDINAL INDICATOR */
			case 0x03A9: y = 0xBD; break;
				/* GREEK CAPITAL LETTER OMEGA */
			case 0x00E6: y = 0xBE; break;
				/* LATIN SMALL LETTER AE */
			case 0x00F8: y = 0xBF; break;
				/* LATIN SMALL LETTER O WITH STROKE */
			case 0x00BF: y = 0xC0; break;
				/* INVERTED QUESTION MARK */
			case 0x00A1: y = 0xC1; break;
				/* INVERTED EXCLAMATION MARK */
			case 0x00AC: y = 0xC2; break;
				/* NOT SIGN */
			case 0x221A: y = 0xC3; break;
				/* SQUARE ROOT */
			case 0x0192: y = 0xC4; break;
				/* LATIN SMALL LETTER F WITH HOOK */
			case 0x2248: y = 0xC5; break;
				/* ALMOST EQUAL TO */
			case 0x2206: y = 0xC6; break;
				/* INCREMENT */
			case 0x00AB: y = 0xC7; break;
				/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
			case 0x00BB: y = 0xC8; break;
				/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
			case 0x2026: y = 0xC9; break;
				/* HORIZONTAL ELLIPSIS */
			case 0x00A0: y = 0xCA; break;
				/* NO-BREAK SPACE */
			case 0x00C0: y = 0xCB; break;
				/* LATIN CAPITAL LETTER A WITH GRAVE */
			case 0x00C3: y = 0xCC; break;
				/* LATIN CAPITAL LETTER A WITH TILDE */
			case 0x00D5: y = 0xCD; break;
				/* LATIN CAPITAL LETTER O WITH TILDE */
			case 0x0152: y = 0xCE; break;
				/* LATIN CAPITAL LIGATURE OE */
			case 0x0153: y = 0xCF; break;
				/* LATIN SMALL LIGATURE OE */
			case 0x2013: y = 0xD0; break;
				/* EN DASH */
			case 0x2014: y = 0xD1; break;
				/* EM DASH */
			case 0x201C: y = 0xD2; break;
				/* LEFT DOUBLE QUOTATION MARK */
			case 0x201D: y = 0xD3; break;
				/* RIGHT DOUBLE QUOTATION MARK */
			case 0x2018: y = 0xD4; break;
				/* LEFT SINGLE QUOTATION MARK */
			case 0x2019: y = 0xD5; break;
				/* RIGHT SINGLE QUOTATION MARK */
			case 0x00F7: y = 0xD6; break;
				/* DIVISION SIGN */
			case 0x25CA: y = 0xD7; break;
				/* LOZENGE */
			case 0x00FF: y = 0xD8; break;
				/* LATIN SMALL LETTER Y WITH DIAERESIS */
			case 0x0178: y = 0xD9; break;
				/* LATIN CAPITAL LETTER Y WITH DIAERESIS */
			case 0x2044: y = 0xDA; break;
				/* FRACTION SLASH */
			case 0x20AC: y = 0xDB; break;
				/* EURO SIGN */
			case 0x2039: y = 0xDC; break;
				/* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
			case 0x203A: y = 0xDD; break;
				/* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
			case 0xFB01: y = 0xDE; break;
				/* LATIN SMALL LIGATURE FI */
			case 0xFB02: y = 0xDF; break;
				/* LATIN SMALL LIGATURE FL */
			case 0x2021: y = 0xE0; break;
				/* DOUBLE DAGGER */
			case 0x00B7: y = 0xE1; break;
				/* MIDDLE DOT */
			case 0x201A: y = 0xE2; break;
				/* SINGLE LOW-9 QUOTATION MARK */
			case 0x201E: y = 0xE3; break;
				/* DOUBLE LOW-9 QUOTATION MARK */
			case 0x2030: y = 0xE4; break;
				/* PER MILLE SIGN */
			case 0x00C2: y = 0xE5; break;
				/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
			case 0x00CA: y = 0xE6; break;
				/* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
			case 0x00C1: y = 0xE7; break;
				/* LATIN CAPITAL LETTER A WITH ACUTE */
			case 0x00CB: y = 0xE8; break;
				/* LATIN CAPITAL LETTER E WITH DIAERESIS */
			case 0x00C8: y = 0xE9; break;
				/* LATIN CAPITAL LETTER E WITH GRAVE */
			case 0x00CD: y = 0xEA; break;
				/* LATIN CAPITAL LETTER I WITH ACUTE */
			case 0x00CE: y = 0xEB; break;
				/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
			case 0x00CF: y = 0xEC; break;
				/* LATIN CAPITAL LETTER I WITH DIAERESIS */
			case 0x00CC: y = 0xED; break;
				/* LATIN CAPITAL LETTER I WITH GRAVE */
			case 0x00D3: y = 0xEE; break;
				/* LATIN CAPITAL LETTER O WITH ACUTE */
			case 0x00D4: y = 0xEF; break;
				/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
			case 0xF8FF: y = 0xF0; break;
				/* Apple logo */
			case 0x00D2: y = 0xF1; break;
				/* LATIN CAPITAL LETTER O WITH GRAVE */
			case 0x00DA: y = 0xF2; break;
				/* LATIN CAPITAL LETTER U WITH ACUTE */
			case 0x00DB: y = 0xF3; break;
				/* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
			case 0x00D9: y = 0xF4; break;
				/* LATIN CAPITAL LETTER U WITH GRAVE */
			case 0x0131: y = 0xF5; break;
				/* LATIN SMALL LETTER DOTLESS I */
			case 0x02C6: y = 0xF6; break;
				/* MODIFIER LETTER CIRCUMFLEX ACCENT */
			case 0x02DC: y = 0xF7; break;
				/* SMALL TILDE */
			case 0x00AF: y = 0xF8; break;
				/* MACRON */
			case 0x02D8: y = 0xF9; break;
				/* BREVE */
			case 0x02D9: y = 0xFA; break;
				/* DOT ABOVE */
			case 0x02DA: y = 0xFB; break;
				/* RING ABOVE */
			case 0x00B8: y = 0xFC; break;
				/* CEDILLA */
			case 0x02DD: y = 0xFD; break;
				/* DOUBLE ACUTE ACCENT */
			case 0x02DB: y = 0xFE; break;
				/* OGONEK */
			case 0x02C7: y = 0xFF; break;
				/* CARON */
			default: y = '?'; break;
				/* unrecognized */
		}
	}

	return y;
}
#endif

#if IncludeHostTextClipExchange
static void UniCodeStr2MacRoman(char *s, char *r)
{
	MacErr_t err;
	uint8_t t;
	uint8_t t2;
	uint8_t t3;
	uint8_t t4;
	uint32_t v;
	char *p = s;
	char *q = r;

label_retry:
	if (0 == (t = *p++)) {
		err = mnvm_noErr;
		/* done */
	} else
	if (0 == (0x80 & t)) {
		*q++ = t;
		goto label_retry;
	} else
	if (0 == (0x40 & t)) {
		/* continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (t2 = *p++)) {
		err = mnvm_miscErr;
	} else
	if (0x80 != (0xC0 & t2)) {
		/* not a continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (0x20 & t)) {
		/* two bytes */
		v = t & 0x1F;
		v = (v << 6) | (t2 & 0x3F);
		*q++ = UniCodePoint2MacRoman(v);
		goto label_retry;
	} else
	if (0 == (t3 = *p++)) {
		err = mnvm_miscErr;
	} else
	if (0x80 != (0xC0 & t3)) {
		/* not a continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (0x10 & t)) {
		/* three bytes */
		v = t & 0x0F;
		v = (v << 6) | (t3 & 0x3F);
		v = (v << 6) | (t2 & 0x3F);
		*q++ = UniCodePoint2MacRoman(v);
		goto label_retry;
	} else
	if (0 == (t4 = *p++)) {
		err = mnvm_miscErr;
	} else
	if (0x80 != (0xC0 & t4)) {
		/* not a continuation code, error */
		err = mnvm_miscErr;
	} else
	if (0 == (0x08 & t)) {
		/* four bytes */
		v = t & 0x07;
		v = (v << 6) | (t4 & 0x3F);
		v = (v << 6) | (t3 & 0x3F);
		v = (v << 6) | (t2 & 0x3F);
		*q++ = UniCodePoint2MacRoman(v);
		goto label_retry;
	} else
	{
		err = mnvm_miscErr;
		/* longer code not supported yet */
	}
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC MacErr_t HTCEimport(tPbuf *r)
{
	MacErr_t err;
	uimr L;
	char *s = NULL;
	tPbuf t = NotAPbuf;

	if (NULL == (s = SDL_GetClipboardText())) {
		err = mnvm_miscErr;
	} else
	if (mnvm_noErr != (err =
		UniCodeStrLength(s, &L)))
	{
		/* fail */
	} else
	if (mnvm_noErr != (err =
		PbufNew(L, &t)))
	{
		/* fail */
	} else
	{
		err = mnvm_noErr;

		UniCodeStr2MacRoman(s, PbufDat[t]);
		*r = t;
		t = NotAPbuf;
	}

	if (NotAPbuf != t) {
		PbufDispose(t);
	}
	if (NULL != s) {
		SDL_free(s);
	}

	return err;
}
#endif
