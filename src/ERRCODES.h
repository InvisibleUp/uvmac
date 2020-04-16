/*
	error codes returned by Mini vMac extensions
	(passed back to the emulated 68k code).
*/

#ifndef ERRCODES_H
#define ERRCODES_H

typedef enum MacErr {
	mnvm_noErr            = 0x0000, /*  0 - No Error */
	mnvm_miscErr          = 0xFFFF, /*  1 - Should probably replace these */
	mnvm_controlErr       = 0xFFEF, /* 17 - I/O System Errors */
	mnvm_statusErr        = 0xFFEE, /* 18 - Driver can't respond to Status call */
	mnvm_closErr          = 0xFFE8, /* 24 - I/O System Errors */
	mnvm_eofErr           = 0xFFD9, /* 39 - End of file */
	mnvm_tmfoErr          = 0xFFD6, /* 42 - too many files open */
	mnvm_fnfErr           = 0xFFD5, /* 43 - File not found */
	mnvm_wPrErr           = 0xFFD4, /* 44 - diskette is write protected */
	mnvm_vLckdErr         = 0xFFD2, /* 46 - volume is locked */
	mnvm_dupFNErr         = 0xFFD0, /* 48 - duplicate filename */
	mnvm_opWrErr          = 0xFFCF, /* 49 - file already open with with write permission */
	mnvm_paramErr         = 0xFFCE, /* 50 - error in parameter list */
	mnvm_permErr          = 0xFFCA, /* 54 - permissions error (on file open) */
	mnvm_nsDrvErr         = 0xFFC8, /* 56 - No Such Drive */
	mnvm_wrPermErr        = 0xFFC3, /* 61 - write permissions error */
	mnvm_offLinErr        = 0xFFBF, /* 65 - off-line drive */
	mnvm_dirNFErr         = 0xFF88, /* 120 - directory not found */
	mnvm_afpAccessDenied  = 0xEC78, /* 5000 - Insufficient access privileges for operation */
} MacErr_t;

#endif // ERRCODES_H
