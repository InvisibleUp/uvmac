/*
	ADBSHARE.h

	Copyright (C) 2008 Paul C. Pratt

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
	Apple Desktop Bus SHAREd code
	shared by emulation of different implementations of ADB
*/

#ifdef ADBSHARE_H
#error "header already included"
#else
#define ADBSHARE_H
#endif


/*
	ReportAbnormalID unused 0x0D08 - 0x0DFF
*/

#define ADB_MaxSzDatBuf 8

static uint8_t ADB_SzDatBuf;
static bool ADB_TalkDatBuf = false;
static uint8_t ADB_DatBuf[ADB_MaxSzDatBuf];
static uint8_t ADB_CurCmd = 0;
static uint8_t NotSoRandAddr = 1;

static uint8_t MouseADBAddress;
static bool SavedCurMouseButton = false;
static uint16_t MouseADBDeltaH = 0;
static uint16_t MouseADBDeltaV = 0;

static void ADB_DoMouseTalk(void)
{
	switch (ADB_CurCmd & 3) {
		case 0:
			{
				EvtQEl *p;
				uint16_t partH;
				uint16_t partV;
				bool overflow = false;
				bool MouseButtonChange = false;

				if (nullpr != (p = EvtQOutP())) {
					if (EvtQElKindMouseDelta == p->kind) {
						MouseADBDeltaH += p->u.pos.h;
						MouseADBDeltaV += p->u.pos.v;
						EvtQOutDone();
					}
				}
				partH = MouseADBDeltaH;
				partV = MouseADBDeltaV;

				if ((int16_t)MouseADBDeltaH < 0) {
					partH = - partH;
				}
				if ((int16_t)MouseADBDeltaV < 0) {
					partV = - partV;
				}
				if ((partH >> 6) > 0) {
					overflow = true;
					partH = (1 << 6) - 1;
				}
				if ((partV >> 6) > 0) {
					overflow = true;
					partV = (1 << 6) - 1;
				}
				if ((int16_t)MouseADBDeltaH < 0) {
					partH = - partH;
				}
				if ((int16_t)MouseADBDeltaV < 0) {
					partV = - partV;
				}
				MouseADBDeltaH -= partH;
				MouseADBDeltaV -= partV;
				if (! overflow) {
					if (nullpr != (p = EvtQOutP())) {
						if (EvtQElKindMouseButton == p->kind) {
							SavedCurMouseButton = p->u.press.down;
							MouseButtonChange = true;
							EvtQOutDone();
						}
					}
				}
				if ((0 != partH) || (0 != partV) || MouseButtonChange) {
					ADB_SzDatBuf = 2;
					ADB_TalkDatBuf = true;
					ADB_DatBuf[0] = (SavedCurMouseButton ? 0x00 : 0x80)
						| (partV & 127);
					ADB_DatBuf[1] = /* 0x00 */ 0x80 | (partH & 127);
				}
			}
			ADBMouseDisabled = 0;
			break;
		case 3:
			ADB_SzDatBuf = 2;
			ADB_TalkDatBuf = true;
			ADB_DatBuf[0] = 0x60 | (NotSoRandAddr & 0x0f);
			ADB_DatBuf[1] = 0x01;
			NotSoRandAddr += 1;
			break;
		default:
			ReportAbnormalID(0x0D01, "Talk to unknown mouse register");
			break;
	}
}

static void ADB_DoMouseListen(void)
{
	switch (ADB_CurCmd & 3) {
		case 3:
			if (ADB_DatBuf[1] == 0xFE) {
				/* change address */
				MouseADBAddress = (ADB_DatBuf[0] & 0x0F);
			} else {
				ReportAbnormalID(0x0D02,
					"unknown listen op to mouse register 3");
			}
			break;
		default:
			ReportAbnormalID(0x0D03,
				"listen to unknown mouse register");
			break;
	}
}

static uint8_t KeyboardADBAddress;

static bool CheckForADBkeyEvt(uint8_t *NextADBkeyevt)
{
	int i;
	bool KeyDown;

	if (! FindKeyEvent(&i, &KeyDown)) {
		return false;
	} else {
#if dbglog_HAVE && 0
		if (KeyDown) {
			dbglog_WriteNote("Got a KeyDown");
		}
#endif
		switch (i) {
			case MKC_Control:
				i = 0x36;
				break;
			case MKC_Left:
				i = 0x3B;
				break;
			case MKC_Right:
				i = 0x3C;
				break;
			case MKC_Down:
				i = 0x3D;
				break;
			case MKC_Up:
				i = 0x3E;
				break;
			default:
				/* unchanged */
				break;
		}
		*NextADBkeyevt = (KeyDown ? 0x00 : 0x80) | i;
		return true;
	}
}

static void ADB_DoKeyboardTalk(void)
{
	switch (ADB_CurCmd & 3) {
		case 0:
			{
				uint8_t NextADBkeyevt;

				if (CheckForADBkeyEvt(&NextADBkeyevt)) {
					ADB_SzDatBuf = 2;
					ADB_TalkDatBuf = true;
					ADB_DatBuf[0] = NextADBkeyevt;
					if (! CheckForADBkeyEvt(&NextADBkeyevt)) {
						ADB_DatBuf[1] = 0xFF;
					} else {
						ADB_DatBuf[1] = NextADBkeyevt;
					}
				}
			}
			break;
		case 3:
			ADB_SzDatBuf = 2;
			ADB_TalkDatBuf = true;
			ADB_DatBuf[0] = 0x60 | (NotSoRandAddr & 0x0f);
			ADB_DatBuf[1] = 0x01;
			NotSoRandAddr += 1;
			break;
		default:
			ReportAbnormalID(0x0D04,
				"Talk to unknown keyboard register");
			break;
	}
}

static void ADB_DoKeyboardListen(void)
{
	switch (ADB_CurCmd & 3) {
		case 3:
			if (ADB_DatBuf[1] == 0xFE) {
				/* change address */
				KeyboardADBAddress = (ADB_DatBuf[0] & 0x0F);
			} else {
				ReportAbnormalID(0x0D05,
					"unknown listen op to keyboard register 3");
			}
			break;
		default:
			ReportAbnormalID(0x0D06,
				"listen to unknown keyboard register");
			break;
	}
}

static bool CheckForADBanyEvt(void)
{
	EvtQEl *p = EvtQOutP();
	if (nullpr != p) {
		switch (p->kind) {
			case EvtQElKindMouseButton:
			case EvtQElKindMouseDelta:
			case EvtQElKindKey:
				return true;
				break;
			default:
				break;
		}
	}

	return (0 != MouseADBDeltaH) && (0 != MouseADBDeltaV);
}

static void ADB_DoTalk(void)
{
	uint8_t Address = ADB_CurCmd >> 4;
	if (Address == MouseADBAddress) {
		ADB_DoMouseTalk();
	} else if (Address == KeyboardADBAddress) {
		ADB_DoKeyboardTalk();
	}
}

static void ADB_EndListen(void)
{
	uint8_t Address = ADB_CurCmd >> 4;
	if (Address == MouseADBAddress) {
		ADB_DoMouseListen();
	} else if (Address == KeyboardADBAddress) {
		ADB_DoKeyboardListen();
	}
}

static void ADB_DoReset(void)
{
	MouseADBAddress = 3;
	KeyboardADBAddress = 2;
}

static void ADB_Flush(void)
{
	uint8_t Address = ADB_CurCmd >> 4;

	if ((Address == KeyboardADBAddress)
		|| (Address == MouseADBAddress))
	{
		ADB_SzDatBuf = 2;
		ADB_TalkDatBuf = true;
		ADB_DatBuf[0] = 0x00;
		ADB_DatBuf[1] = 0x00;
	} else {
		ReportAbnormalID(0x0D07, "Unhandled ADB Flush");
	}
}
