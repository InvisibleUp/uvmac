/*
	HW/SCREEN/SCRNMAPR.h

	Copyright (C) 2012 Paul C. Pratt

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
	SCReeN MAPpeR
*/

#include "SCRNMAPR.h"
#include <assert.h>

void ScrnMapr_DoMap(
    rect_t region, rect_t bounds,
    const color_t *src, color_t *dst,
	uint8_t src_depth, uint8_t dst_depth,
    const color_t *map, uint8_t scale
) {
	/* check of parameters */
	assert(src_depth >= 0);
	assert(src_depth <= 3);
	assert(dst_depth >= src_depth);

	/* define variables */
	int x, y, sx, sy; // loop vars
	uint16_t line_width = bounds.right - bounds.left;

	for (y = region.top; y < region.bottom; y += 1)
	{
		for (sy = 0; sy < scale - 1; sy += 1)
		{
			for (x = region.left; x < region.right; x += 1)
			{
				color_t color = src[(y+sy)*line_width + x];
				for (sx = 0; sx < scale - 1; sx += 1)
				{
					dst[(y+sy)*line_width + x*scale + sx] = map[color];
				}
			}
		}
	}
}
