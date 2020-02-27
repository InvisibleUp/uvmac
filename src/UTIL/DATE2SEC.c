/*
	UTIL/DATE2SEC.h
	Copyright (C) 2003 Bradford L. Barrett, Paul C. Pratt

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
	DATE 2(to) SEConds

	convert year/month/day/hour/minute/second
	to number of seconds since the beginning
	of 1904, the format for storing dates
	on the Macintosh.

	The function jdate is from the program Webalizer
	by Bradford L. Barrett.
*/


/*
	The function jdate was found at the end of the file
	webalizer.c in the program webalizer at
	"www.mrunix.net/webalizer/".
	Here is copyright info from the top of that file:

	webalizer - a web server log analysis program

	Copyright (C) 1997-2000  Bradford L. Barrett (brad@mrunix.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version, and provided that the above
	copyright and permission notice is included with all distributed
	copies of this or derived software.
*/

#include "SYSDEPNS.h"
#include "DATE2SEC.h"

uint32_t jdate(int day, int month, int year)
{
	uint32_t days;                      /* value returned */
	int mtable[] = {
		0,    31,  59,  90, 120, 151,
		181, 212, 243, 273, 304, 334
	};

	/*
		First, calculate base number including leap
		and Centenial year stuff
	*/

	days = (((uint32_t)year * 365) + day + mtable[month - 1]
		+ ((year + 4) / 4) - ((year / 100) - (year / 400)));

	/* now adjust for leap year before March 1st */

	if ((year % 4 == 0)
		&& (! ((year % 100 == 0) && (year % 400 != 0)))
		&& (month < 3))
	{
		--days;
	}

	/* done, return with calculated value */

	return (days + 5);
}

uint32_t Date2MacSeconds(int second, int minute, int hour,
	int day, int month, int year)
{
	uint32_t curjdate;
	uint32_t basejdate;

	curjdate = jdate(day, month, year);
	basejdate = jdate(1, 1, 1904);
	return (((curjdate - basejdate) * 24 + hour) * 60
		+ minute) * 60 + second;
}
