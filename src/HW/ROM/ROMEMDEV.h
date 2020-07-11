#include <stdint.h>
#include <stdbool.h>

extern uint8_t * ROM;
extern bool ROM_loaded;

uint32_t Calc_Checksum(uint8_t *rom, uint32_t len);
bool ROM_IsValid(void);
bool WaitForRom(void);
