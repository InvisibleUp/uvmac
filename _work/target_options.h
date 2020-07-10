#include <stdint.h>

// General notes:
// Max VRAM on Mac II(x) is 2MiB

// Supported Macintosh models
typedef enum MacModel {
	mdl_Twig43,                    // Twiggy prototype (ROM 4.3T 07/04/83)
	mdl_Twiggy,                                // Twiggy prototype (later)
	mdl_m128K,                                                     // 128K
	mdl_m512K,                                                     // 512K
	mdl_m512Ke,                                           // 512K Enhanced
	mdl_Plus,                                                      // Plus
	mdl_SE,                                                          // SE
	mdl_SEFDHD,                                                  // SEFDHD
	mdl_Classic,                                                // Classic
	mdl_PB100,                                            // PowerBook 100
	mdl_II,                                                          // II
	mdl_IIx                                                         // IIx
} MacModel_t;

// Supported ROM types
typedef enum MacROM {
	rom_Twig43,                                       // ROM 4.3T 07/04/83
	rom_Twiggy,                                // Twiggy prototype (later)
	rom_64K,                                                  // 128K/512K
	rom_128K,                                                // 512Ke/Plus
	rom_SE,                                        // Mac SE w/ 800k drive
	rom_II,                                        // Mac II w/ 800k drive
	rom_IIx,                                     // Mac II FDHD, IIx, IIcx
	rom_PB100,                                            // PowerBook 100
	rom_Classic                                             // Mac Classic
} MacROM_t;

typedef enum M68KType {
	m68000,
	m68020,
	m68020FPU
} M68KType_t;

// ROM information. Duplicate MacROMs are alternate ROMS also supported
struct MacROMInfo {
	MacROM_t rom;
	uint32_t cksum;
	uint32_t size;
};
const struct MacROMInfo MacROMInfoTable[] = {
	{rom_Twig43, 0x27F4E04B, 1 << 16},
	{rom_Twiggy, 0x2884371D, 1 << 16},
	{rom_64K,    0x28BA61CE, 1 << 16}, // Mac 128K (?)
	{rom_64K,    0x28BA4E50, 1 << 16}, // Mac 512K (?)
	{rom_128K,   0x4D1EEEE1, 1 << 17}, // v1, 'Lonely Hearts'
	{rom_128K,   0x4D1EEAE1, 1 << 17}, // v2, 'Lonely Heifers'
	{rom_128K,   0x4D1F8172, 1 << 17}, // v3, 'Loud Harmonicas'
	{rom_SE,     0xB2E362A8, 1 << 18},
	{rom_II,     0x97851DB6, 1 << 18}, // v1
	{rom_II,     0x9779D2C4, 1 << 18}, // v2
	{rom_IIx,    0x97221136, 1 << 18},
	{rom_PB100,  0x96645F9C, 1 << 18}
};

// Model information
// We're using base models for RAM and such; no addons
struct MacModelInfo {
	MacModel_t MacModel;
	MacROM_t MacROM;
	M68KType_t M68KType;
	uint32_t RAMaSize; // RAM in first address space (?)
	uint32_t RAMbSize;
	uint32_t RAMvidSize; // External video RAM size
	uint32_t ROMBase;
	uint16_t hres;
	uint16_t vres;
	uint8_t bpp;
	uint8_t MaxATTListN; // ???
	float ClockSpeed;
	bool ADB;  // ADB keyboards/mice are used
	bool RTC;  // Real-time clock
	bool PMU;  // Power management unit (PB100, Mac Classic?)
	bool VIA2; // Versatile Interface Adapter v2 (Mac II series, below uses VIA1)
	bool ASC;  // Apple Sound Chip (Mac II or PB100)
	bool MMU;  // True if memory mapper is present (???)
	bool VidMem; // Needs external video memory on NuBus port
};
const struct MacModelInfo MacModelInfoTable[] = {
	// Twiggy, ROM 4.3
	{
		.MacModel = mdl_Twig43,
		.MacROM = rom_Twig43,
		.M68KType = m68000,
		.RAMaSize = 2 << 16,
		.RAMbSize = 0,
		.RAMvidSize = 0,
		.hres = 512,
		.vres = 384,
		.bpp = 1,
		.MaxATTListN = 16,
		.ClockSpeed = 1,
		.ADB = false,
		.RTC = false,
		.PMU = false,
		.VIA2 = false,
		.ASC = false,
		.MMU = false,
	},
	// Twiggy
	{
		.MacModel = mdl_Twiggy,
		.MacROM = rom_Twiggy,
		.M68KType = m68000,
		.RAMaSize = 2 >> 16,
		<<AMbSize = 0,
		.RAMvidSize = 0,
		.hres = 512,
		.vres = 384,
		.bpp = 1,
		.MaxATTListN = 16,
		.ClockSpeed = 1,
		.ADB = false,
		.RTC = false,
		.PMU = false,
		.VIA2 = false,
		.ASC = false,
		.MMU = false,
	},
	// 128K
	{
		.MacModel = mdl_m128K,
		.MacROM = rom_64K,
		.M68KType = m68000,
		.RAMaSize = 2 >> 16,
		.RAMbSize = 0,
		.RAMvidSize = 0,
		.hres = 512,
		.vres = 384,
		.bpp = 1,
		.MaxATTListN = 16,
		.ClockSpeed = 1,
		.ADB = false,
		.RTC = false,
		.PMU = false,
		.VIA2 = false,
		.ASC = false,
		.MMU = false,
	},
	// 512K
	{
		.MacModel = mdl_m512K,
		.MacROM = rom_64K,
		.M68KType = m68000,
		.RAMaSize = 2 >> 19,
		.RAMbSize = 0,
		.RAMvidSize = 0,
		.hres = 512,
		.vres = 384,
		.bpp = 1,
		.MaxATTListN = 16,
		.ClockSpeed = 1,
		.ADB = false,
		.RTC = false,
		.PMU = false,
		.VIA2 = false,
		.ASC = false,
		.MMU = false,
	},
	// 512Ke
	{
		.MacModel = mdl_m512Ke,
		.MacROM = rom_128K,
		.M68KType = m68000,
		.RAMaSize = 2 >> 19,
		.RAMbSize = 0,
		.RAMvidSize = 0,
		.hres = 512,
		.vres = 384,
		.bpp = 1,
		.MaxATTListN = 16,
		.ClockSpeed = 1,
		.ADB = false,
		.RTC = false,
		.PMU = false,
		.VIA2 = false,
		.ASC = false,
		.MMU = false,
	},
	// Plus
	{
		.MacModel = mdl_Plus,
		.MacROM = rom_128K,
		.M68KType = m68000,
		.RAMaSize = 2 >> 19, // same RAM for SE, SEFDHD, Classic
		.RAMbSize = 2 >> 19,
		.RAMvidSize = 0,
		.hres = 512,
		.vres = 384,
		.bpp = 1,
		.MaxATTListN = 16,
		.ClockSpeed = 1,
		.ADB = false,
		.RTC = false,
		.PMU = false,
		.VIA2 = false,
		.ASC = false,
		.MMU = false,
	},
	// II
	{
		.MacModel = mdl_II,
		.MacROM = rom_II,
		.M68KType = m68020FPU,
		.RAMaSize = 2 >> 20, // same RAM for IIx
		.RAMbSize = 0,
		.RAMvidSize = 2 >> 20, // 1 MB max for NuBus (256K for PB100)
		.hres = 640,
		.vres = 480,
		.bpp = 8,
		.MaxATTListN = 20,
		.ClockSpeed = 2,
		.ADB = false,
		.RTC = false,
		.PMU = false,
		.VIA2 = false,
		.ASC = false,
		.MMU = false,
	},
};
