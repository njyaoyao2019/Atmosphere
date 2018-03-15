#include "i2c.h"
#include "t210.h"
#include "mc.h"
//#include "emc.h"
#include "pmc.h"
#include "util.h"
#include "fuse.h"

#include "sdram.inl"

static u32 _get_sdram_id()
{
	return (fuse_read_odm(4) & 0x38) >> 3;
}

static void _sdram_config(const u32 *_cfg)
{
	const u32 *_cfg_120 = _cfg + 0x120;
	const u32 *_cfg_100 = _cfg + 0x100;

	PMC(0x45C) = (((4 * _cfg[0x12F] >> 2) + 0x80000000) ^ 0xFFFF) & 0xC000FFFF;
	sleep(_cfg[0x111]);

	u32 req = (4 * _cfg_120[0x10] >> 2) + 0x80000000;
	PMC(APBDEV_PMC_IO_DPD4_REQ) = (req ^ 0x3FFF0000) >> 16 << 16;
	sleep(_cfg_100[0x12]);
	PMC(APBDEV_PMC_IO_DPD4_REQ) = (req ^ 0xFFFF) & 0xC000FFFF;
	sleep(_cfg_100[0x12]);
	PMC(APBDEV_PMC_WEAK_BIAS) = 0;
	sleep(1);

	CLOCK(0x98) = _cfg[4];
	CLOCK(0x9C) = 0;
	CLOCK(0x90) = (_cfg[2] << 8) | (*((u16 *)_cfg + 0xA) << 20) | _cfg[1] | 0x40000000;

	u32 wait_end = TMR(0x10) + 300;
	while (!((CLOCK(0x90) >> 27) & 1))
	{
		if (TMR(0x10) >= wait_end)
			goto break_nosleep;
	}
	sleep(10);
break_nosleep:

	CLOCK(0x19C) = _cfg[0x16] & 0xFFFEFFFF | (_cfg[0x175] >> 11) & 0x10000;
	if (_cfg[0x17])
		CLOCK(0x664) = _cfg[0x17];
	if (_cfg[0x1A])
		CLOCK(0x44C) = 0x40000000;
	CLOCK(0x328) = 0x2000001;
	CLOCK(0x284) = 0x4000;
	CLOCK(0x30C) = 0x2000001;
	EMC(0xC34) = _cfg_120[0x13];
	EMC(0xC38) = _cfg_120[0x14];
	EMC(0xCF0) = _cfg_120[0x15];
	EMC(0x28) = 1;
	sleep(1);
	EMC(0x8) = _cfg[0xA9] | 2 * _cfg[0xAA];
	if (_cfg[0xA])
		*(vu32 *)_cfg[0xA] = _cfg[0xB];
	EMC(0x584) = _cfg[0x7B];
	EMC(0x380) = _cfg[0x7D];
	EMC(0x384) = _cfg[0x7E];
	EMC(0x388) = _cfg[0x7F];
	EMC(0x38C) = _cfg[0x80];
	EMC(0x390) = _cfg[0x81];
	EMC(0x394) = _cfg[0x82];
	EMC(0x398) = _cfg[0x83];
	EMC(0x39C) = _cfg[0x84];
	EMC(0x3A0) = _cfg[0x85];
	EMC(0x3A4) = _cfg[0x86];
	EMC(0x3A8) = _cfg[0x87];
	EMC(0x3AC) = _cfg[0x88];
	EMC(0x3B0) = _cfg[0x89];
	EMC(0xC80) = _cfg[0x14A];
	EMC(0xC84) = _cfg[0x14B];
	EMC(0xC88) = _cfg[0x14C];
	EMC(0x330) = (_cfg_120[0x16] | 0xFEEDFEED) & 0x1FFF1FFF;
	EMC(0x5F0) = _cfg[0x149];
	EMC(0x5C8) = _cfg[0x7C];
	EMC(0x404) = _cfg_100[0x18];
	EMC(0x408) = _cfg_100[0x19];
	EMC(0x40C) = _cfg_100[0x1A];
	EMC(0x410) = _cfg_100[0x1B];
	EMC(0x418) = _cfg_100[0x1C];
	EMC(0x41C) = _cfg_100[0x1D];
	EMC(0x420) = _cfg_100[0x1E];
	EMC(0x424) = _cfg_100[0x1F];
	if (_cfg[0xE])
		*(vu32 *)_cfg[0xE] = _cfg[0xF];
	EMC(0x30C) = _cfg[0x31];
	EMC(0x578) = _cfg[0x32];
	EMC(0x2F4) = _cfg[0x33];
	EMC(0x458) = _cfg[0x1D];
	EMC(0x45C) = _cfg[0x1E];
	EMC(0x5B0) = _cfg[0x1F];
	EMC(0x5B4) = _cfg[0x20];
	EMC(0x5CC) = _cfg[0x21];
	EMC(0x574) = _cfg[0x22];
	EMC(0x2DC) = _cfg[0x23];
	EMC(0xC48) = _cfg[0x2A];
	EMC(0xC70) = _cfg[0x2B];
	EMC(0xC74) = _cfg[0x2C];
	EMC(0xC4C) = _cfg[0x2D];
	EMC(0xC78) = _cfg[0x2E];
	EMC(0x464) = _cfg[0x26];
	EMC(0xC44) = _cfg[0x2F];
	EMC(0x5E4) = _cfg_120[0xD];
	EMC(0x5E8) = _cfg_120[0xE];
	EMC(0x2C8) = _cfg[0xB0];
	EMC(0x588) = _cfg_120[1];
	EMC(0x58C) = _cfg_120[2];
	EMC(0x594) = _cfg_120[3];
	EMC(0x598) = _cfg_120[4];
	EMC(0x59C) = _cfg_120[5];
	EMC(0x5A0) = _cfg_120[6];
	EMC(0x5A4) = _cfg_120[7];
	EMC(0x5A8) = _cfg_120[8];
	EMC(0x5AC) = _cfg_120[9];
	EMC(0x5B8) = _cfg_120[0xA];
	EMC(0x5BC) = _cfg_120[0xB];
	EMC(0x5C4) = _cfg_120[0xC];
	EMC(0x330) = (_cfg_120[0x16] | 0xFE40FE40) & 0x1FFF1FFF;
	EMC(0xC40) = _cfg_120[0x12];
	EMC(0x318) = _cfg_120[0x17];
	EMC(0x334) = _cfg_120[0x18] & 0xFF7FFF7F;
	EMC(0x31C) = _cfg_120[0x19];
	EMC(0xC3C) = _cfg_120[0x1A];
	EMC(0xC54) = _cfg_120[0x1B];
	EMC(0xC50) = _cfg_120[0x1C];
	EMC(0xC64) = _cfg_120[0x1F];
	EMC(0xC5C) = _cfg_120[0x1D];
	EMC(0xC58) = _cfg_120[0x1E];
	EMC(0xC60) = _cfg[0x141];
	EMC(0x49C) = _cfg[0x142];
	EMC(0x720) = _cfg[0x143];
	EMC(0x724) = _cfg[0x144];
	EMC(0x728) = _cfg[0x145];
	EMC(0x72C) = _cfg[0x146];
	EMC(0x730) = _cfg[0x147];
	EMC(0x734) = _cfg[0x148];
	EMC(0x740) = _cfg[0x14D];
	EMC(0x744) = _cfg[0x14E];
	EMC(0x748) = _cfg[0x14F];
	EMC(0x74C) = _cfg[0x150];
	EMC(0x750) = _cfg[0x151];
	EMC(0x754) = _cfg[0x152];
	EMC(0x760) = _cfg[0x153];
	EMC(0x770) = _cfg[0x154];
	EMC(0x774) = _cfg[0x155];
	EMC(0x778) = _cfg[0x156];
	EMC(0x780) = _cfg[0x157];
	EMC(0x784) = _cfg[0x158];
	EMC(0x788) = _cfg[0x159];
	EMC(0xBE0) = _cfg[0xB6];
	EMC(0xBE4) = _cfg[0xB7];
	EMC(0xBF0) = _cfg[0xB8];
	EMC(0xBF4) = _cfg[0xB9];
	EMC(0xCF4) = _cfg[0xBA];
	EMC(0x600) = _cfg[0xBD];
	EMC(0x604) = _cfg[0xBE];
	EMC(0x608) = _cfg[0xBF];
	EMC(0x60C) = _cfg[0xC0];
	EMC(0x610) = _cfg[0xC1];
	EMC(0x614) = _cfg[0xC2];
	EMC(0x620) = _cfg[0xC3];
	EMC(0x624) = _cfg[0xC4];
	EMC(0x628) = _cfg[0xC5];
	EMC(0x62C) = _cfg[0xC6];
	EMC(0x630) = _cfg[0xC7];
	EMC(0x634) = _cfg[0xC8];
	EMC(0x330) = _cfg_120[0x16];
	EMC(0x640) = _cfg[0xC9];
	EMC(0x644) = _cfg[0xCA];
	EMC(0x648) = _cfg[0xCB];
	EMC(0x64C) = _cfg[0xCC];
	EMC(0x650) = _cfg[0xCD];
	EMC(0x654) = _cfg[0xCE];
	EMC(0x660) = _cfg[0xCF];
	EMC(0x664) = _cfg[0xD0];
	EMC(0x668) = _cfg[0xD1];
	EMC(0x66C) = _cfg[0xD2];
	EMC(0x670) = _cfg[0xD3];
	EMC(0x674) = _cfg[0xD4];
	EMC(0x680) = _cfg[0xD5];
	EMC(0x684) = _cfg[0xD6];
	EMC(0x688) = _cfg[0xD7];
	EMC(0x68C) = _cfg[0xD8];
	EMC(0x690) = _cfg[0xD9];
	EMC(0x694) = _cfg[0xDA];
	EMC(0x6A0) = _cfg[0xDB];
	EMC(0x6A4) = _cfg[0xDC];
	EMC(0x6A8) = _cfg[0xDD];
	EMC(0x6AC) = _cfg[0xDE];
	EMC(0x6B0) = _cfg[0xDF];
	EMC(0x6B4) = _cfg[0xE0];
	EMC(0x6C0) = _cfg[0xE1];
	EMC(0x6C4) = _cfg[0xE2];
	EMC(0x6C8) = _cfg[0xE3];
	EMC(0x6CC) = _cfg[0xE4];
	EMC(0x6E0) = _cfg[0xE5];
	EMC(0x6E4) = _cfg[0xE6];
	EMC(0x6E8) = _cfg[0xE7];
	EMC(0x6EC) = _cfg[0xE8];
	EMC(0xC00) = _cfg[0xE9];
	EMC(0xC04) = _cfg[0xEA];
	EMC(0xC08) = _cfg[0xEB];
	EMC(0xC0C) = _cfg[0xEC];
	EMC(0xC10) = _cfg[0xED];
	EMC(0xC20) = _cfg[0xEE];
	EMC(0xC24) = _cfg[0xEF];
	EMC(0xC28) = _cfg[0xF0];
	EMC(0xC68) = (*((u8 *)_cfg + 0x500) | 0xFFFFFFFE) & 0xF;
	if (_cfg[0xC])
		*(vu32 *)_cfg[0xC] = _cfg[0xD];
	EMC(0x28) = 1;
	MC(0x648) = _cfg[0x180];
	MC(0x978) = _cfg[0x181];
	MC(0x64C) = _cfg[0x182];
	MC(0x418) = _cfg[0x183];
	MC(0x590) = _cfg[0x184];
	MC(0x984) = _cfg[0x185];
	MC(0x988) = _cfg[0x186];
	MC(0x54) = _cfg[0x15A];
	MC(0x58) = _cfg[0x15B];
	MC(0x5C) = _cfg[0x15C];
	MC(0x60) = _cfg[0x15D];
	MC(0x64) = _cfg[0x15E];
	MC(0x68) = _cfg[0x15F];
	MC(0x6C) = _cfg[0x160];
	MC(0x50) = _cfg[0x161];
	MC(0x670) = _cfg[0x187];
	MC(0x9D4) = _cfg[0x188];
	MC(0x674) = _cfg[0x189];
	MC(0x9A0) = _cfg[0x1D6];
	MC(0x9A8) = _cfg[0x1D7];
	MC(0x9A4) = _cfg[0x1D8];
	MC(0x90) = _cfg[0x162];
	MC(0x94) = _cfg[0x163];
	MC(0x6F0) = _cfg[0x164];
	MC(0x6F4) = _cfg[0x165];
	MC(0x98) = _cfg[0x166];
	MC(0x9C) = _cfg[0x167];
	MC(0xA0) = _cfg[0x168];
	MC(0xA4) = _cfg[0x169];
	MC(0xA8) = _cfg[0x16A];
	MC(0xAC) = _cfg[0x16B];
	MC(0xB0) = _cfg[0x16C];
	MC(0xB4) = _cfg[0x16D];
	MC(0xB8) = _cfg[0x16E];
	MC(0xBC) = _cfg[0x16F];
	MC(0x6C4) = _cfg[0x17D];
	MC(0xC0) = _cfg[0x170];
	MC(0xC4) = _cfg[0x171];
	MC(0x6C0) = _cfg[0x172];
	MC(0xD0) = _cfg[0x173];
	MC(0xD4) = _cfg[0x174];
	MC(0xD8) = _cfg[0x175];
	MC(0xDC) = _cfg[0x176];
	MC(0xC8) = _cfg[0x177];
	MC(0xE0) = _cfg[0x178];
	MC(0xE8) = _cfg[0x179];
	MC(0x968) = _cfg[0x17A];
	MC(0xEC) = _cfg[0x17B];
	MC(0x9DC) = _cfg[0x17C];
	MC(0xFC) = 1;
	MC(0xF4) = _cfg[0x17E];
	MC(0x100) = _cfg[0x17F];
	EMC(0x10) = _cfg[0x34];
	EMC(0x140) = _cfg_100[7];
	EMC(0x700) = _cfg[0x27];
	EMC(0x704) = _cfg[0x28];
	EMC(0x708) = _cfg[0x29];
	EMC(0x2F8) = _cfg[0x24];
	EMC(0x300) = _cfg[0x25];
	EMC(0x2A8) = _cfg[0x1B];
	EMC(0x2A4) = _cfg[0x1C];
	sleep(_cfg[0x30]);
	if (_cfg[0x10])
		*(vu32 *)_cfg[0x10] = _cfg[0x11];
	EMC(0x2B8) = _cfg[0xA4];
	EMC(0x560) = _cfg[0xA5];
	EMC(0x55C) = _cfg[0xBB];
	EMC(0x554) = _cfg[0xBC];
	EMC(0xF0) = _cfg[0xAB];
	EMC(0xF4) = _cfg[0xAC];
	EMC(0xC8) = _cfg[0xA1];
	EMC(0xC4) = _cfg[0xA2];
	EMC(0x104) = _cfg[0x7A];
	EMC(0x2C) = _cfg[0x3A];
	EMC(0x30) = _cfg[0x3B];
	EMC(0x590) = _cfg[0x3C];
	EMC(0x580) = _cfg[0x3D];
	EMC(0xC0) = _cfg[0x3E];
	EMC(0x34) = _cfg[0x3F];
	EMC(0x38) = _cfg[0x40];
	EMC(0xAC) = _cfg[0x47];
	EMC(0x144) = _cfg[0x41];
	EMC(0x148) = _cfg[0x42];
	EMC(0x3C) = _cfg[0x43];
	EMC(0x40) = _cfg[0x44];
	EMC(0x44) = _cfg[0x45];
	EMC(0x48) = _cfg[0x46];
	EMC(0x5C0) = _cfg[0x48];
	EMC(0x4C) = _cfg[0x49];
	EMC(0x50) = _cfg[0x4A];
	EMC(0x54) = _cfg[0x4B];
	EMC(0x58) = _cfg[0x4C];
	EMC(0xB8) = _cfg[0x4D];
	EMC(0x5C) = _cfg[0x4E];
	EMC(0x4E0) = _cfg[0x4F];
	EMC(0x498) = _cfg[0x50];
	EMC(0x494) = _cfg[0x51];
	EMC(0x2D0) = _cfg[0x52];
	EMC(0x490) = _cfg[0x53];
	EMC(0x48C) = _cfg[0x54];
	EMC(0x60) = _cfg[0x55];
	EMC(0x568) = _cfg[0x56];
	EMC(0x468) = _cfg[0x57];
	EMC(0x46C) = _cfg[0x58];
	EMC(0x14C) = _cfg[0x59];
	EMC(0x150) = _cfg[0x5A];
	EMC(0x154) = _cfg[0x5B];
	EMC(0x56C) = _cfg[0x5C];
	EMC(0xC68) = _cfg[0x140];
	EMC(0x8) = _cfg[0xA9];
	EMC(0x64) = _cfg[0x5D];
	EMC(0x428) = 0;
	EMC(0x68) = _cfg[0x5E];
	EMC(0x6C) = _cfg[0x5F];
	EMC(0x2CC) = _cfg[0x60];
	EMC(0x2D8) = _cfg[0x61];
	EMC(0x2D4) = _cfg[0x62];
	EMC(0x564) = _cfg[0x63];
	EMC(0x70) = _cfg[0x64];
	EMC(0x74) = _cfg[0x65];
	EMC(0x3DC) = _cfg[0x66];
	EMC(0x78) = _cfg[0x67];
	EMC(0x7C) = _cfg[0x68];
	EMC(0x80) = _cfg[0x69];
	EMC(0x84) = _cfg[0x6A];
	EMC(0x88) = _cfg[0x6B];
	EMC(0x8C) = _cfg[0x6C];
	EMC(0x11C) = _cfg[0x6D];
	EMC(0x118) = _cfg[0x6E];
	EMC(0xB4) = _cfg[0x6F];
	EMC(0x90) = _cfg[0x70];
	EMC(0x3E4) = _cfg[0x71];
	EMC(0x94) = _cfg[0x72];
	EMC(0x158) = _cfg[0x73];
	EMC(0x15C) = _cfg[0x74];
	EMC(0x98) = _cfg[0x75];
	EMC(0x9C) = _cfg[0x76];
	EMC(0xA0) = _cfg[0x77];
	EMC(0xA4) = _cfg[0x78];
	EMC(0xA8) = _cfg[0x79];
	EMC(0xB0) = _cfg[0xF2];
	EMC(0x2BC) = _cfg[0xAF];
	EMC(0x2C0) = _cfg[0xB1];
	EMC(0x100) = _cfg[0x8A] & 0xFFFFFFFD;
	EMC(0x120) = _cfg[0x8B];
	EMC(0x440) = _cfg_120[0xF];
	EMC(0x444) = _cfg_120[0x10];
	EMC(0x448) = _cfg_120[0x11];
	EMC(0x124) = _cfg_100[0x17];
	EMC(0x480) = *_cfg_120;
	EMC(0xC) = ((_cfg[0xA3] & 4 | 0x3C00000) & 0xFFFFFFF7 | _cfg[0xA3] & 8) & 0xFFFFFFFD | _cfg[0xA3] & 2;
	if ((_cfg[0x1D4] & 0x80000000) != 0)
	{
		*(vu32 *)(4 * _cfg[0x1D4] + 0x70000000) = _cfg[0x1D5];
		MC(0xFC) = 1;
	}
	PMC(0x45C) = ((4 * _cfg_120[0xF] >> 2) + 0x40000000) & 0xCFFF0000;
	sleep(_cfg_100[0x11]);
	if (!_cfg[0x1B])
		EMC(0x2A4) = _cfg[0x1C] | 0x200;
	EMC(0x334) = _cfg_120[0x18];
	if (_cfg[0xFA] << 31)
	{
		if (*_cfg == 2)
			EMC(0x2E4) = 8 * _cfg[0xF4];
		if (*_cfg == 3)
		{
			EMC(0x2E4) = _cfg[0xF4];
			EMC(0x2E8) = _cfg[0xF5];
		}
	}
	EMC(0x28) = 1;
	sleep(_cfg[0x39]);
	PMC(0x4E4) &= 0xFFF8007F;
	sleep(_cfg_100[0x15]);
	if (*_cfg == 2)
	{
		EMC(0x24) = (_cfg[0x37] << 16) | (_cfg[0x38] << 12);
		sleep(_cfg[0x36] + 200);
		EMC(0x24) = ((_cfg[0x37] << 16) | (_cfg[0x38] << 12)) + 0x100;
		sleep(_cfg[0x36] + 500);
	}
	if (*_cfg == 3)
	{
		EMC(0x24) = (_cfg[0x37] << 16) | (_cfg[0x38] << 12);
		sleep(_cfg[0x36] + 200);
		EMC(0x24) = ((_cfg[0x37] << 16) | (_cfg[0x38] << 12)) + 0x100;
		sleep(_cfg[0x36] + 2000);
	}
	EMC(0x24) = ((_cfg[0x37] << 16) | (_cfg[0x38] << 12)) + 0x101;
	sleep(_cfg[0x35]);
	if (*_cfg != 3)
		EMC(0xDC) = (_cfg[0xB2] << 30) + 1;
	if (*_cfg == 1)
		sleep(_cfg[0x36] + 200);
	if (*_cfg == 3)
	{
		if (_cfg[0x12])
			*(vu32 *)_cfg[0x12] = _cfg[0x13];
		EMC(0x134) = _cfg[0x91];
		EMC(0xE8) = _cfg[0x90];
		EMC(0x138) = _cfg[0x92];
		EMC(0x13C) = _cfg[0x93];
		EMC(0x4A4) = _cfg[0x94];
		EMC(0x4C4) = _cfg[0x9A];
		EMC(0x4AC) = _cfg[0x95];
		EMC(0x4BC) = _cfg[0x98];
		EMC(0x4B0) = _cfg[0x96];
		EMC(0x4C0) = _cfg[0x99];
		if (_cfg[0xFA] << 31)
		{
			EMC(0x2EC) = _cfg[0xF7];
			sleep(_cfg[0xF9]);
			EMC(0x2EC) = _cfg[0xF7] ^ 3;
			if (!(_cfg[0xB2] & 2))
			{
				EMC(0x2EC) = _cfg[0xF8];
				sleep(_cfg[0xF9]);
				EMC(0x2EC) = _cfg[0xF8] ^ 3;
			}
		}
	}
	PMC(0x1D0) = _cfg_100[0xF];
	if (_cfg[0] == 1 || _cfg[0] == 2 || _cfg[0] == 3)
	{
		EMC(0x2E0) = _cfg[0xF3];
		EMC(0x2E4) = _cfg[0xF4];
		EMC(0x2E8) = _cfg[0xF5];
	}
	if (_cfg[0x14])
		*(vu32 *)_cfg[0x14] = _cfg[0x15];
	EMC(0x28) = 1;
	if (_cfg_100[8])
		EMC(0xD4) = ((1 << _cfg_100[8] << 8) - 0xFD) | (_cfg[0x38] << 30);
	EMC(0x20) = _cfg[0xB2] | 0x80000000;
	EMC(0x3E0) = _cfg[0xAD];
	EMC(0x5F4) = _cfg[0xA8];
	EMC(0xC) = _cfg[0xA3];
	EMC(0x310) = _cfg[0xB4];
	EMC(0x314) = _cfg[0xB5];
	EMC(0x3D8) = _cfg[0xB3];
	EMC(0x100) = _cfg[0x8A] | 2;
	EMC(0x28) = 1;
	EMC(0x558) = _cfg[0xA6];
	EMC(0x4D8) = _cfg[0xA7];
	SYSREG(AHB_ARBITRATION_XBAR_CTRL) = SYSREG(AHB_ARBITRATION_XBAR_CTRL) & 0xFFFEFFFF | (*((u16 *)_cfg + 0x15C) << 16);
	MC(0x650) = _cfg[0x18A];
	MC(0x678) = _cfg[0x18B];
	MC(0x9AC) = _cfg[0x1D9];
	MC(MC_EMEM_CFG_ACCESS_CTRL) = 1; //Disable write access to a bunch of MC registers.
}

void sdram_init()
{
	u32 sdram_id = _get_sdram_id();
	const u32 *cfg = _dram_cfgs[sdram_id]; //TODO: sdram_id should be in [0,4].

	i2c_send_byte(I2C_5, 0x3C, 0x22, 0x05);
	i2c_send_byte(I2C_5, 0x3C, 0x17, 40); //40 = (1000 * 1100 - 600000) / 12500

	PMC(APBDEV_PMC_VDDP_SEL) = cfg[0x10C];
	sleep(cfg[0x10D]);
	PMC(APBDEV_PMC_DDR_PWR) = PMC(0xE8);
	PMC(APBDEV_PMC_NO_IOPOWER) = cfg[0x114];
	PMC(APBDEV_PMC_REG_SHORT) = cfg[0x113];
	PMC(APBDEV_PMC_DDR_CNTRL) = cfg[0x116];

	if (cfg[8])
		*(vu32 *)cfg[8] = cfg[9];

	_sdram_config(cfg);
}
