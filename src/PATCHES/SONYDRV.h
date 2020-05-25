#define UseSonyPatch \
	((CurEmMd <= kEmMd_Classic) || (CurEmMd == kEmMd_II) \
		|| (CurEmMd == kEmMd_IIx))

#if CurEmMd <= kEmMd_Twig43
#define Sony_DriverBase 0x1836
#elif CurEmMd <= kEmMd_Twiggy
#define Sony_DriverBase 0x16E4
#elif CurEmMd <= kEmMd_128K
#define Sony_DriverBase 0x1690
#elif CurEmMd <= kEmMd_Plus
#define Sony_DriverBase 0x17D30
#elif CurEmMd <= kEmMd_Classic
#define Sony_DriverBase 0x34680
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define Sony_DriverBase 0x2D72C
#endif

void Sony_Install();