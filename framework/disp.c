#include "disp.h"

#include "pmath.h"

RenderContext ctx;

#define addPrimRev(ot,p) setaddr(p, getaddr(ot)), setaddr(ot, p)

#define SCREEN_XRES (320)
#define SCREEN_YRES (240)

void setup_context(RenderContext *ctx, int w, int h, int r, int g, int b) {
	// Place the two framebuffers vertically in VRAM.
	SetDefDrawEnv(&(ctx->buffers[0].draw_env), 0, 0, w*(ctx->render_mode+1), h*(ctx->render_mode+1));
	SetDefDispEnv(&(ctx->buffers[0].disp_env), 0, 0, w*(ctx->render_mode+1), h*(ctx->render_mode+1));
	if (ctx->render_mode) {
		SetDefDrawEnv(&(ctx->buffers[1].draw_env), 0, 0, w*(ctx->render_mode+1), h*(ctx->render_mode+1));
		SetDefDispEnv(&(ctx->buffers[1].disp_env), 0, 0, w*(ctx->render_mode+1), h*(ctx->render_mode+1));
	} else {
		SetDefDrawEnv(&(ctx->buffers[1].draw_env), 0, h, w*(ctx->render_mode+1), h*(ctx->render_mode+1));
		SetDefDispEnv(&(ctx->buffers[1].disp_env), 0, h, w*(ctx->render_mode+1), h*(ctx->render_mode+1));
	}

	// Set the default background color and enable auto-clearing.
	setRGB0(&(ctx->buffers[0].draw_env), r, g, b);
	setRGB0(&(ctx->buffers[1].draw_env), r, g, b);
	ctx->buffers[0].draw_env.isbg = 1;
	ctx->buffers[1].draw_env.isbg = 1;

	ctx->buffers[0].disp_env.isinter = ctx->render_mode;
	ctx->buffers[1].disp_env.isinter = ctx->render_mode;

	// Initialize the first buffer and clear its OT so that it can be used for
	// drawing.
	ctx->active_buffer = 0;
	ctx->next_packet   = ctx->buffers[0].buffer;
	ClearOTagR(ctx->buffers[0].ot, OT_LENGTH);
	ClearOTagR(ctx->buffers[0].ui_ot, UI_OT_LENGTH);

	ctx->screen_clip = (RECT*)(0x1F800000);
	setRECT(ctx->screen_clip, -10, -10, w*(ctx->render_mode+1)+10, h*(ctx->render_mode+1)+10);

	// Turn on the video output.
	SetDispMask(1);
}

void update_render_mode(uint8_t mode) {
	ctx.render_mode = mode;
	ResetGraph( 0 );
    setup_context(&ctx, SCREEN_XRES, SCREEN_YRES, 0, 0, 0);

	gte_SetGeomScreen( SCREEN_XRES/((1-ctx.render_mode) + 1) );
	gte_SetGeomOffset( SCREEN_XRES/((1-ctx.render_mode) + 1) , SCREEN_YRES/((1-ctx.render_mode) + 1) );
}

// static int last = 0;
// static int vdiff = 0;
uint8_t flip_buffers(RenderContext *ctx) {
	// Wait for the GPU to finish drawing, then wait for vblank in order to
	// prevent screen tearing.
	DrawSync(0);
	// vdiff = VSync(-1) - last;
	// last = VSync(-1);
	VSync(0);

	RenderBuffer *draw_buffer = &(ctx->buffers[ctx->active_buffer]);
	RenderBuffer *disp_buffer = &(ctx->buffers[ctx->active_buffer ^ 1]);

	// Display the framebuffer the GPU has just finished drawing and start
	// rendering the display list that was filled up in the main loop.
	PutDrawEnv(&(draw_buffer->draw_env));
	PutDispEnv(&(disp_buffer->disp_env));

	DrawOTag(&(draw_buffer->ot[OT_LENGTH - 1]));
	DrawOTag(&(draw_buffer->ui_ot[UI_OT_LENGTH - 1]));

	// Switch over to the next buffer, clear it and reset the packet allocation
	// pointer.
	ctx->active_buffer ^= 1;
	ctx->next_packet    = disp_buffer->buffer;
	// ClearOTagR(disp_buffer->ot, OT_LENGTH);
	ClearOTagR(disp_buffer->ot, OT_LENGTH);
	ClearOTagR(disp_buffer->ui_ot, UI_OT_LENGTH);

	ctx->poly_amount = 0;
	return 1; 
	// broken due to double buffering and resolution changes anyway
	// return vdiff;
}

void *new_primitive(RenderContext *ctx) {
	// Place the primitive after all previously allocated primitives, then
	// insert it into the OT and bump the allocation pointer.
	uint8_t *prim = ctx->next_packet;
	return (void *) prim;
}

void *new_primitive_reserved(RenderContext *ctx, size_t size) {
	// Place the primitive after all previously allocated primitives, then
	// insert it into the OT and bump the allocation pointer.
	uint8_t *prim = ctx->next_packet;
	ctx->next_packet += size;

	return (void *) prim;
}

void push_primitive(RenderContext *ctx, int z, size_t size, uint8_t *prim) {
	RenderBuffer *buffer = &(ctx->buffers[ctx->active_buffer]);
	addPrim(&(buffer->ot[z]), prim);

	ctx->next_packet += size;

	// Make sure we haven't yet run out of space for future primitives.
	assert(ctx->next_packet <= &(buffer->buffer[BUFFER_LENGTH]));
}

void push_primitive_pre(RenderContext *ctx, int z, uint8_t *prim) {
	RenderBuffer *buffer = &(ctx->buffers[ctx->active_buffer]);
	addPrim(&(buffer->ot[z]), prim);
}

void push_ui_elem(RenderContext *ctx, int z, size_t size, uint8_t *prim) {
	RenderBuffer *buffer = &(ctx->buffers[ctx->active_buffer]);
	addPrim(&(buffer->ui_ot[z]), prim);

	ctx->next_packet += size;

	// Make sure we haven't yet run out of space for future primitives.
	assert(ctx->next_packet <= &(buffer->buffer[BUFFER_LENGTH]));
}

void initDisplay() {
	ResetGraph( 0 );
    // setup_context(&ctx, SCREEN_XRES, SCREEN_YRES, 10, 0, 40);
	ctx.render_mode = 0;
    setup_context(&ctx, SCREEN_XRES, SCREEN_YRES, 0, 0, 0);
	// Initialize the GPU and load the default font texture provided by
	// PSn00bSDK at (960, 0) in VRAM.
	FntLoad(960, 0);

	InitGeom();
	// gte_SetFarColor(10, 0, 40);
	gte_SetFarColor(0, 0, 0);
	gte_SetGeomScreen( SCREEN_XRES/((1-ctx.render_mode) + 1) );
	gte_SetGeomOffset( SCREEN_XRES/((1-ctx.render_mode) + 1) , SCREEN_YRES/((1-ctx.render_mode) + 1) );
	
}

// A simple helper for drawing text using PSn00bSDK's debug font API. Note that
// FntSort() requires the debug font texture to be uploaded to VRAM beforehand
// by calling FntLoad().

void draw_text(RenderContext *ctx, int x, int y, int z, const char *text) {
	RenderBuffer *buffer = &(ctx->buffers[ctx->active_buffer]);

	ctx->next_packet = (uint8_t *)
		FntSort(&(buffer->ui_ot[z]), ctx->next_packet, x, y, text);

	ctx->poly_amount += 1;

	assert(ctx->next_packet <= &(buffer->buffer[BUFFER_LENGTH]));
}