#include <stdint.h>
#include "GLOBGLUE.h"
#include "HW/ROM/ROMEMDEV.h"
#include "UTIL/ENDIANAC.h"

uint8_t * ROM = nullpr;
bool ROM_loaded = false;

uint32_t Calc_Checksum(uint8_t *rom, uint32_t len)
{
	uint32_t i;
	uint32_t CheckSum = 0;
	uint8_t * p = 4 + rom;

	for (i = (len - 4) >> 1; --i >= 0; ) {
		CheckSum += do_get_mem_word(p);
		p += 2;
	}

	return CheckSum;
}

bool ROM_IsValid(void)
{
	/*if(CheckRomCheckSum) {
		uint32_t CheckSum = Calc_Checksum();

		if (CheckSum != do_get_mem_long(ROM)) {
			// Check against internal checksum
			WarnMsgCorruptedROM();
			return mnvm_miscErr;
		} else if (!(
			CheckSum == kRomCheckSum1 ||
			CheckSum == kRomCheckSum2 ||
			CheckSum == kRomCheckSum3
		)) {
			// Unsupported ROM
			WarnMsgUnsupportedROM();
			return mnvm_miscErr;
		}
	}*/

	ROM_loaded = true;
	SpeedStopped = false;

	return true;
}

// Loop for when there's nothing to do but tell the user they're missing a ROM
// TODO: Hijack this for config mode?
bool WaitForRom(void)
{
	return true;
}
