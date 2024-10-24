#ifndef DISP_H
#define DISP_H

#include <sys/types.h>
#include <assert.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <inline_c.h>

// Length of the ordering table, i.e. the range Z coordinates can have, 0-63 in
// this case. Larger values will allow for more granularity with depth (useful
// when drawing a complex 3D scene) at the expense of RAM usage and performance.
// #define OT_LENGTH (1<<10)
#define OT_LENGTH (800)
#define UI_OT_LENGTH (4)

// Size of the buffer GPU commands and primitives are written to. If the program
// crashes due to too many primitives being drawn, increase this value.
// #define BUFFER_LENGTH (1<<17)
#define BUFFER_LENGTH (100000)

typedef struct {
	DISPENV disp_env;
	DRAWENV draw_env;

	uint32_t ot[OT_LENGTH];
	uint32_t ui_ot[UI_OT_LENGTH];
	uint8_t  buffer[BUFFER_LENGTH];
} RenderBuffer;

typedef struct {
	RenderBuffer buffers[2];
	uint8_t*     next_packet;
	int          active_buffer;
	RECT* 		 screen_clip;
	uint16_t	 poly_amount;
	uint16_t	 render_mode;

} RenderContext;

void setup_context(RenderContext *ctx, int w, int h, int r, int g, int b);
void update_render_mode(uint8_t mode);

uint8_t flip_buffers(RenderContext *ctx);

void *new_primitive(RenderContext *ctx);
void *new_primitive_reserved(RenderContext *ctx, size_t size);

void push_primitive(RenderContext *ctx, int z, size_t size, uint8_t *prim);
void push_primitive_pre(RenderContext *ctx, int z, uint8_t *prim);
void push_ui_elem(RenderContext *ctx, int z, size_t size, uint8_t *prim);

void initDisplay();

void draw_text(RenderContext *ctx, int x, int y, int z, const char *text);

extern RenderContext ctx;

#endif // _DISP_H