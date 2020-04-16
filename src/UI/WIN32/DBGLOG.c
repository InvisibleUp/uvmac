/* --- sending debugging info to file --- */

#include <windows.h>
#include <tchar.h>
#include "SYSDEPNS.h"
#include "UI/WIN32/OSGLUWIN.h"

#if dbglog_HAVE

LOCALVAR HANDLE dbglog_File = INVALID_HANDLE_VALUE;

LOCALFUNC bool dbglog_open0(void)
{
	TCHAR pathName[_MAX_PATH];
	TCHAR Child0[] = TEXT("\\dbglog.txt");
	size_t newlen;

	if (GetAppDir(pathName)) {
		newlen = _tcslen(pathName) + _tcslen(Child0);
		if (newlen + 1 < _MAX_PATH) {
			_tcscat(pathName, Child0);

			dbglog_File = CreateFile(
				pathName, /* pointer to name of the file */
				GENERIC_READ + GENERIC_WRITE,
					/* access (read-write) mode */
				0, /* share mode */
				NULL, /* pointer to security descriptor */
				OPEN_ALWAYS, /* how to create */
				FILE_ATTRIBUTE_NORMAL, /* file attributes */
				NULL /* handle to file with attributes to copy */
			);
			if (INVALID_HANDLE_VALUE == dbglog_File) {
				/* report error (how?) */
			} else if (SetFilePointer(
				dbglog_File, /* handle of file */
				0, /* number of bytes to move file pointer */
				nullpr,
					/* address of high-order word of distance to move */
				FILE_BEGIN /* how to move */
				) != 0)
			{
				/* report error (how?) */
			}
		}
	}

	return (INVALID_HANDLE_VALUE != dbglog_File);
}

LOCALPROC dbglog_write0(char *s, uimr L)
{
	DWORD BytesWritten;

	if (INVALID_HANDLE_VALUE != dbglog_File) {
		if (! WriteFile(dbglog_File, /* handle of file to read */
			(LPVOID)s, /* address of buffer that receives data */
			(DWORD)L, /* number of bytes to read */
			&BytesWritten, /* address of number of bytes read */
			nullpr) /* address of structure for data */
			|| ((uint32_t)BytesWritten != L))
		{
			/* report error (how?) */
		}
	}
}

LOCALPROC dbglog_close0(void)
{
	if (INVALID_HANDLE_VALUE != dbglog_File) {
		if (! SetEndOfFile(dbglog_File)) {
			/* report error (how?) */
		}
		(void) CloseHandle(dbglog_File);
		dbglog_File = INVALID_HANDLE_VALUE;
	}
}

#endif
