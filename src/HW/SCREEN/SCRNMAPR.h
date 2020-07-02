/* SCRNMAPR.h */
#include <stdint.h>
#pragma once

typedef struct {
    uint16_t top;
    uint16_t left;
    uint16_t right;
    uint16_t bottom;
} rect_t;

typedef uint32_t color_t;

// Copy a rectangular bitmap region, scaling and converting color depth as needed
void ScrnMapr_DoMap(
    rect_t bounds,
    const uint8_t *src, uint8_t *dst, uint8_t src_depth, uint8_t dst_depth,
    const uint8_t *map, uint8_t scale
);
