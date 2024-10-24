/*
 * PSn00bSDK SPU .VAG playback example
 * (C) 2021-2022 Lameguy64, spicyjpeg - MPL licensed
 *
 * I don't know how to license, this is basically just the example above
 * with my own needs crammed in - Achrostel
 */

#include "soundfx.h"

#include <stdio.h>

// The first 4 KB of SPU RAM are reserved for capture buffers and psxspu
// additionally uploads a dummy sample (16 bytes) at 0x1000 by default, so the
// samples must be placed after those.
#define ALLOC_START_ADDR 0x1010

static int next_channel     = 0;
static int next_sample_addr = ALLOC_START_ADDR;

extern const uint8_t FS0[];
extern const uint8_t FS1[];
extern const uint8_t FS2[];
extern const uint8_t FS3[];
extern const uint8_t collect_vag_data[];
extern const uint8_t warning_vag_data[];
extern const uint8_t light_vag_data[];
extern const uint8_t metal_vag_data[];
extern const uint8_t use_ammo_vag_data[];
extern const uint8_t ding_vag_data[];

static SFX foot_step0;
static SFX foot_step1;
static SFX foot_step2;
static SFX foot_step3;
SFX steps[4];
SFX collect_sfx;
SFX warning_sfx;
SFX metal_sfx;
SFX use_ammo_sfx;
SFX ding_sfx;

static SFX backgroundhum_mus;

static void dumpSPU(unsigned int start, unsigned int until) {
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	for (int read_addr = start; read_addr < until; read_addr += 32) {
		SpuSetTransferStartAddr(read_addr);

		unsigned char data[32];
		SpuRead((uint32_t *) data, 32);
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

		printf("%06x - ", (unsigned int) read_addr);
		for (int i = 0; i < 32; ++i) {	
			printf("%02x ", data[i]);
		}
		printf("\n", NULL);
	}
}

void initSFX() {
	// dumpSPU(0x1000, 0x1100);
    SpuInit();
	// printf("\n----\n", NULL);
	// dumpSPU(0x1000, 0x1100);

	// Upload the samples to the SPU and parse their headers.
	const VAG_Header *FS0_vag = (const VAG_Header *) FS0;
	int FS0_addr = upload_sample(FS0_vag+1, __builtin_bswap32(FS0_vag->size));
	int FS0_sr   = __builtin_bswap32(FS0_vag->sample_rate);
	const VAG_Header *FS1_vag = (const VAG_Header *) FS1;
	int FS1_addr = upload_sample(FS1_vag+1, __builtin_bswap32(FS1_vag->size));
	int FS1_sr   = __builtin_bswap32(FS1_vag->sample_rate);
	const VAG_Header *FS2_vag = (const VAG_Header *) FS2;
	int FS2_addr = upload_sample(FS2_vag+1, __builtin_bswap32(FS2_vag->size));
	int FS2_sr   = __builtin_bswap32(FS2_vag->sample_rate);
	const VAG_Header *FS3_vag = (const VAG_Header *) FS3;
	int FS3_addr = upload_sample(FS3_vag+1, __builtin_bswap32(FS3_vag->size));
	int FS3_sr   = __builtin_bswap32(FS3_vag->sample_rate);

    foot_step0.addr = FS0_addr; foot_step0.sample_rate = FS0_sr;
    foot_step1.addr = FS1_addr; foot_step1.sample_rate = FS1_sr;
    foot_step2.addr = FS2_addr; foot_step2.sample_rate = FS2_sr;
    foot_step3.addr = FS3_addr; foot_step3.sample_rate = FS3_sr;
    steps[0] = foot_step0;
    steps[1] = foot_step1;
    steps[2] = foot_step2;
    steps[3] = foot_step3;

    
	const VAG_Header *collect_vag = (const VAG_Header *) collect_vag_data;
	int collect_addr = upload_sample(collect_vag+1, __builtin_bswap32(collect_vag->size));
	int collect_sr   = __builtin_bswap32(collect_vag->sample_rate);
    collect_sfx.addr = collect_addr; collect_sfx.sample_rate = collect_sr;

	const VAG_Header *warning_vag = (const VAG_Header *) warning_vag_data;
	int warning_addr = upload_sample(warning_vag+1, __builtin_bswap32(warning_vag->size));
	int warning_sr   = __builtin_bswap32(warning_vag->sample_rate);
    warning_sfx.addr = warning_addr; warning_sfx.sample_rate = warning_sr;

	const VAG_Header *metal_vag = (const VAG_Header *) metal_vag_data;
	int metal_addr = upload_sample(metal_vag+1, __builtin_bswap32(metal_vag->size));
	int metal_sr   = __builtin_bswap32(metal_vag->sample_rate);
    metal_sfx.addr = metal_addr; metal_sfx.sample_rate = metal_sr;
	
	const VAG_Header *ammo_vag = (const VAG_Header *) use_ammo_vag_data;
	int ammo_addr = upload_sample(ammo_vag+1, __builtin_bswap32(ammo_vag->size));
	int ammo_sr   = __builtin_bswap32(ammo_vag->sample_rate);
    use_ammo_sfx.addr = ammo_addr; use_ammo_sfx.sample_rate = ammo_sr;
	
	const VAG_Header *ding_vag = (const VAG_Header *) ding_vag_data;
	int ding_addr = upload_sample(ding_vag+1, __builtin_bswap32(ding_vag->size));
	int ding_sr   = __builtin_bswap32(ding_vag->sample_rate);
    ding_sfx.addr = ding_addr; ding_sfx.sample_rate = ding_sr;

	const VAG_Header *light_vag = (const VAG_Header *) light_vag_data;
	int light_addr = upload_sample(light_vag+1, __builtin_bswap32(light_vag->size));
	int light_sr   = __builtin_bswap32(light_vag->sample_rate);
    backgroundhum_mus.addr = light_addr; backgroundhum_mus.sample_rate = light_sr;

	initReverb();

	// printf("\n----\n", NULL);
	// dumpSPU(0x1000, 0x1040);

}

void initReverb() {
	// int _size = 0xF6C0;
	// uint16_t rev[] = { 
	//   	0x033D, 0x0231, 0x7E00, 0x5000, 0xB400, 0xB000, 0x4C00, 0xB000,
	// 	0x6000, 0x5400, 0x1ED6, 0x1A31, 0x1D14, 0x183B, 0x1BC2, 0x16B2,
	// 	0x1A32, 0x15EF, 0x15EE, 0x1055, 0x1334, 0x0F2D, 0x11F6, 0x0C5D,
	// 	0x1056, 0x0AE1, 0x0AE0, 0x07A2, 0x0464, 0x0232, 0x8000, 0x8000,
	// };

	// SPU_REVERB_VOL_L = 0x0000;
	// SPU_REVERB_VOL_R = 0x0000;
	// SPU_REVERB_ADDR = 0xfffe;

	SPU_CTRL |= 1<<7;

	int _size = 0x1F40;
	uint16_t settings[] = { 
		0x0033, 0x0025, 0x70F0, 0x4FA8, 0xBCE0, 0x4410, 0xC0F0, 0x9C00,
		0x5280, 0x4EC0, 0x03E4, 0x031B, 0x03A4, 0x02AF, 0x0372, 0x0266,
		0x031C, 0x025D, 0x025C, 0x018E, 0x022F, 0x0135, 0x01D2, 0x00B7,
		0x018F, 0x00B5, 0x00B4, 0x0080, 0x004C, 0x0026, 0x8000, 0x8000,
	};

	SPU_REVERB_ADDR = (0x80000-_size);
	// printf("R %x\n", (unsigned int)((0x80000-_size)));

	SPU_REVERB_VOL_L = 0x7fff;
	SPU_REVERB_VOL_R = 0x7fff;

	#define SPU_REVERB_SETTINGS(N) _MMIO16((IOBASE | 0x1dc0) + (2 * (N)))
	for (int i = 0; i < 32; ++i) {
		SPU_REVERB_SETTINGS(i) = settings[i];
	}
	#undef SPU_REVERB_SETTINGS

	
	// printf("\n-REV-\n", NULL);
	// dumpSPU(SPU_REVERB_ADDR, SPU_REVERB_ADDR+_size);
}

int upload_sample(const void *data, int size) {
	// Round the size up to the nearest multiple of 64, as SPU DMA transfers
	// are done in 64-byte blocks.
	int addr = next_sample_addr;
	size = (size + 63) & ~63;

	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferStartAddr(addr);

	SpuWrite((const uint32_t *) data, size);
	for (int i = 50; i; --i)
		printf("wait %d TRANSFER %04x\n", i, SPU_STAT);

	// printf("SA %x\n", (unsigned int)next_sample_addr);
	next_sample_addr = addr + size;
	// printf("SE %x\n\n", (unsigned int)next_sample_addr);
	return addr;
}

uint8_t play_sample(int addr, int sample_rate, int16_t volume) {
	int ch = next_channel;

	// Make sure the channel is stopped.
	SpuSetKey(0, 1 << ch);

	// Set the channel's sample rate and start address. Note that the SPU
	// expects the sample rate to be in 4.12 fixed point format (with
	// 1.0 = 44100 Hz) and the address in 8-byte units; psxspu.h provides the
	// getSPUSampleRate() and getSPUAddr() macros to convert values to these
	// units.
	SPU_CH_FREQ(ch) = getSPUSampleRate(sample_rate);
	SPU_CH_ADDR(ch) = getSPUAddr(addr);

	// Set the channel's volume and ADSR parameters (0x80ff and 0x1fee are
	// dummy values that disable the ADSR envelope entirely).
	SPU_CH_VOL_L(ch) = volume;
	SPU_CH_VOL_R(ch) = volume;
	SPU_CH_ADSR1(ch) = 0x00ff;
	SPU_CH_ADSR2(ch) = 0x0000;

	SPU_REVERB_ON1 |= 1 << ch;

	// Start the channel.
	SpuSetKey(1, 1 << ch);

	next_channel = (ch + 1) % 16;
	
	return ch;
}

uint8_t play_sample_stereo(int addr, int sample_rate, int16_t l, int16_t r) {
    int ch = next_channel;

	// Make sure the channel is stopped.
	SpuSetKey(0, 1 << ch);

	// Set the channel's sample rate and start address. Note that the SPU
	// expects the sample rate to be in 4.12 fixed point format (with
	// 1.0 = 44100 Hz) and the address in 8-byte units; psxspu.h provides the
	// getSPUSampleRate() and getSPUAddr() macros to convert values to these
	// units.
	SPU_CH_FREQ(ch) = getSPUSampleRate(sample_rate);
	SPU_CH_ADDR(ch) = getSPUAddr(addr);

	// Set the channel's volume and ADSR parameters (0x80ff and 0x1fee are
	// dummy values that disable the ADSR envelope entirely).
	SPU_CH_VOL_L(ch) = l;
	SPU_CH_VOL_R(ch) = r;
	SPU_CH_ADSR1(ch) = 0x00ff;
	SPU_CH_ADSR2(ch) = 0x0000;
	
	SPU_REVERB_ON1 |= 1 << ch;

	// Start the channel.
	SpuSetKey(1, 1 << ch);

	next_channel = (ch + 1) % 16;

	return ch;
}

void play_background() {
    int ch = 23;
    // Make sure the channel is stopped.
	SpuSetKey(0, 1 << ch);

	// Set the channel's sample rate and start address. Note that the SPU
	// expects the sample rate to be in 4.12 fixed point format (with
	// 1.0 = 44100 Hz) and the address in 8-byte units; psxspu.h provides the
	// getSPUSampleRate() and getSPUAddr() macros to convert values to these
	// units.
	SPU_CH_FREQ(ch) = getSPUSampleRate(backgroundhum_mus.sample_rate);
	SPU_CH_ADDR(ch) = getSPUAddr(backgroundhum_mus.addr);
	SPU_CH_LOOP_ADDR(ch) = getSPUAddr(backgroundhum_mus.addr);

	// Set the channel's volume and ADSR parameters (0x80ff and 0x1fee are
	// dummy values that disable the ADSR envelope entirely).
	SPU_CH_VOL_L(ch) = 0x02ff;
	SPU_CH_VOL_R(ch) = 0x02ff;
	SPU_CH_ADSR1(ch) = 0x00ff;
	SPU_CH_ADSR2(ch) = 0x0000;

	SPU_REVERB_ON2 |= 1<<(ch-16);

	// Start the channel.
	SpuSetKey(1, 1 << ch);
}

void update_channel_volume(uint8_t channel, uint16_t volL, uint16_t volR) {
	SPU_CH_VOL_L(channel) = volL;
	SPU_CH_VOL_R(channel) = volR;
}