#ifndef SOUNDFX_H
#define SOUNDFX_H


#include <psxapi.h>
#include <psxspu.h>
#include <hwregs_c.h>

/* .VAG header structure */

typedef struct {
	uint32_t magic;			// 0x70474156 ("VAGp") for mono files
	uint32_t version;
	uint32_t interleave;	// Unused in mono files
	uint32_t size;			// Big-endian, in bytes
	uint32_t sample_rate;	// Big-endian, in Hertz
	uint16_t _reserved[5];
	uint16_t channels;		// Unused in mono files
	char     name[16];
} VAG_Header;

typedef struct {
    int addr;
    int sample_rate;
} SFX;


/* Helper functions */

void initSFX();
void initReverb();

int upload_sample(const void *data, int size);

uint8_t play_sample(int addr, int sample_rate, int16_t volume);
uint8_t play_sample_stereo(int addr, int sample_rate, int16_t l, int16_t r);

void play_background();

void update_channel_volume(uint8_t channel, uint16_t volL, uint16_t volR);

#endif // SOUNDFX_H