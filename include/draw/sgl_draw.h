/*************************************************************************
	> File Name: sgl_draw.h
	> Author: mlxh
	> Mail: mlxh_gto@163.com 
	> Created Time: Mon 06 Jul 2026 12:15:46 PM CST
 ************************************************************************/

#ifndef SGL_DRAW_H
#define SGL_DRAW_H
#include <stdint.h>
#include <stdio.h>

/*OoOoOoOoOoOoOoOoOoO
		DEFINE
 OoOoOoOoOoOoOoOoOoO*/
#ifndef SGLDEF
#define SGLDEF static inline
#endif // SGLDEF
#define SGL_CANVAS_NULL ((SGL_Canvas){0})
#define SGL_PIXEL_AT(sc, x, y) (sc).pixels[(y)*(sc).stride + (x)]

#define SGL_SWAP(T, a, b)do{ T t = a; a = b; b = t;}while(0)
#define SGL_SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define SGL_ABS(T, x)  (SGL_SIGN(T,x)*(x))

// color
#define SGL_RED(color)	 ((color) >> (3*8)) & 0xFF
#define SGL_GREEN(color) ((color) >> (2*8)) & 0xFF
#define SGL_BLUE(color)	 ((color) >> (1*8)) & 0xFF
#define SGL_ALPHA(color) ((color) >> (0*8)) & 0xFF
#define SGL_RGBA(r,g,b,a) ( (((r)&0xFF)<<(3*8)) | (((g)&0xFF)<<(2*8)) | (((b)&0xFF)<<(1*8)) | (((a)&0xFF)<<(0*8)) )

#ifndef SGL_DRAW_INFO
#define SGL_DRAW_INFO(fmt,...)  fprintf(stdout,"DRAW INFO %s %d: " fmt "\n" ,__func__,__LINE__,##__VA_ARGS__)
#define SGL_DRAW_ERROR(fmt,...) fprintf(stderr,"DRAW ERROR %s %d: " fmt "\n",__func__,__LINE__,##__VA_ARGS__)
#endif // SGL_DRAW_INFO

typedef struct {
	uint32_t * pixels;
	size_t width;
	size_t height;
	size_t stride;
} SGL_Canvas;

void sgl_canvas_export_ppm(const SGL_Canvas canvas, const char * filename);

#endif // SGL_DRAW_H

#if defined(SGL_DRAW_IMPLEMENTATION) && !defined(SGL_DRAW_IMPLEMENTATION_DOWN)
#define SGL_DRAW_IMPLEMENTATION_DOWN

SGLDEF SGL_Canvas sgl_canvas(uint32_t *pixels, size_t width, size_t height, size_t stride)
{
	return (SGL_Canvas) {
		.pixels = pixels,
		.width	= width,
		.height = height,
		.stride	= stride,
	};
}

SGLDEF void sgl_fill(SGL_Canvas canvas, uint32_t color)
{
	for(size_t y = 0;y<canvas.height;++y){
		for(size_t x = 0;x<canvas.width;++x){
			SGL_PIXEL_AT(canvas,x,y) = color;
		}
	}
}

SGLDEF bool sgl_in_bounds(SGL_Canvas canvas, int x, int y)
{
	return 0<=x && x<(int)canvas.width && 0<=y && y<(int)canvas.height;
}

typedef enum {
	COMP_RED = 0,
	COMP_GREEN,
	COMP_BLUE,
	COMP_ALPHA,
	COMP_COUNT,
} Comp_Index;

SGLDEF uint8_t sgl_comp_mix(uint8_t src, uint8_t bg, uint8_t a)
{
	return (uint8_t)(((uint16_t)src * a + (uint16_t)bg * (255 - a)) / 255);
}

SGLDEF void sgl_mix_color(uint32_t *color, uint32_t bg_color)
{
	uint8_t r = SGL_RED(*color);
	uint8_t g = SGL_GREEN(*color);
	uint8_t b = SGL_BLUE(*color);
	uint8_t a = SGL_ALPHA(*color);

	uint8_t bg_r = SGL_RED(bg_color);
	uint8_t bg_g = SGL_GREEN(bg_color);
	uint8_t bg_b = SGL_BLUE(bg_color);

	uint8_t out_r = sgl_comp_mix(r, bg_r, a);
	uint8_t out_g = sgl_comp_mix(g, bg_g, a);
	uint8_t out_b = sgl_comp_mix(b, bg_b, a);

	*color = SGL_RGBA(out_r, out_g, out_b, a);
}

SGLDEF void sgl_point(SGL_Canvas canvas, int dx, int dy, uint32_t color)
{
	if(!sgl_in_bounds(canvas, dx, dy)) {
		SGL_DRAW_INFO("Out of area of canvas : x[%d] y[%d]",dx,dy);
		return;
	}
	sgl_mix_color(&color,SGL_PIXEL_AT(canvas,(size_t)dx,(size_t)dy));
	SGL_PIXEL_AT(canvas,(size_t)dx,(size_t)dy) = color;
}

SGLDEF void sgl_rect(SGL_Canvas canvas, int x1, int y1, int x2, int y2, uint32_t color)
{
	int dy = y1;
	int dx = x1;
	for(dy = y1; dy<y2; ++dy){
		for(dx = x1; dx<x2; ++dx){
			sgl_point(canvas,dx,dy,color);
		}
	}
}

SGLDEF bool sgl_point_is_in_circle(int cx, int cy, int r, int x, int y)
{
	int px = x - cx, py = y - cy;
	return (px * px + py * py) <= (r * r);
}

SGLDEF void sgl_circle(SGL_Canvas canvas, int cx, int cy, int r, uint32_t color)
{
	int x1 = cx - r, x2 = cx + r;
	int y1 = cy - r, y2 = cy + r;
	for(int dy = y1; dy<=y2; ++dy){
		for(int dx = x1; dx<=x2; ++dx){
			if(sgl_point_is_in_circle(cx,cy,r,dx,dy)){
				sgl_point(canvas,dx,dy,color);
			}
		}
	}
}

SGLDEF void sgl_circle_optimized(SGL_Canvas canvas, int cx, int cy, int r, uint32_t color)
{
    if (r < 0) return;
    if (r == 0) {
        sgl_point(canvas, cx, cy, color);
        return;
    }

    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (x <= y) {
        sgl_rect(canvas, cx - x, cy - y, cx + x + 1, cy - y + 1, color);
        sgl_rect(canvas, cx - x, cy + y, cx + x + 1, cy + y + 1, color);
        sgl_rect(canvas, cx - y, cy - x, cx + y + 1, cy - x + 1, color);
        sgl_rect(canvas, cx - y, cy + x, cx + y + 1, cy + x + 1, color);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

SGLDEF void sgl_line(SGL_Canvas canvas, int x1, int y1, int x2, int y2, uint32_t color)
{
	int dx = x2 - x1;
	int dy = y2 - y1;

	if(dx == 0 && dy == 0){
		sgl_point(canvas,x1,y1,color);
		return;
	}

	if(SGL_ABS(int, dx) > SGL_ABS(int, dy)){
		if(x1 > x2){
			SGL_SWAP(int, x1, x2);
			SGL_SWAP(int, y1, y2);
		}

		for(int x = x1; x <= x2; ++x ){
			int y = dy*(x-x1)/dx + y1;
			sgl_point(canvas,x,y,color);
		}

	}else{
		if(y1 > y2){
			SGL_SWAP(int, x1, x2);
			SGL_SWAP(int, y1, y2);
		}

		for(int y = y1; y<=y2; ++y){
			int x = dx*(y-y1)/dy + x1;
			sgl_point(canvas,x,y,color);
		}
	}
}

SGLDEF void sgl_draw_hor_line(SGL_Canvas canvas, int x1, int x2, int y, int32_t color)
{
	if(x1>x2) SGL_SWAP(int, x1, x2);
	for(int x = x1; x <= x2; ++x){
		sgl_point(canvas,x,y,color);
	}
}

SGLDEF void sgl_fill_flat_bottom_triangle(SGL_Canvas canvas, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
	float invslope1 = (float)(x2 - x1) / (y2 - y1);
	float invslope2 = (float)(x3 - x1) / (y3 - y1);

	float curx1 = x1;
	float curx2 = x1;

	// y = kx + c ==> x1 = (y-c)/k
	// y + 1 ==> x2 = ((y+1) - c )/k
	// x2 - x1 = 1/k
	for (int y = y1; y <= y2; ++y) {
		sgl_draw_hor_line(canvas, (int)curx1, (int)curx2, y, color);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}

SGLDEF void sgl_fill_flat_top_triangle(SGL_Canvas canvas, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
	float invslope1 = (float)(x3 - x1) / (y3 - y1);
	float invslope2 = (float)(x3 - x2) / (y3 - y2);

	float curx1 = x3;
	float curx2 = x3;

	for (int y = y3; y > y1; --y) {
		sgl_draw_hor_line(canvas, (int)curx1, (int)curx2, y, color);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}

SGLDEF void sgl_fill_triangle(SGL_Canvas canvas, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
	if(y1 > y2) { SGL_SWAP(int, y1, y2); SGL_SWAP(int, x1, x2); }
	if(y2 > y3) { SGL_SWAP(int, y2, y3); SGL_SWAP(int, x2, x3); }
	if(y1 > y2) { SGL_SWAP(int, y1, y2); SGL_SWAP(int, x1, x2); }

	if (y2 == y3) {
		sgl_fill_flat_bottom_triangle(canvas, x1, y1, x2, y2, x3, y3, color);
	} else if (y1 == y2) {
		sgl_fill_flat_top_triangle(canvas, x1, y1, x2, y2, x3, y3, color);
	} else {
		int x4 = (int)(x1 + ((float)(y2 - y1) / (float)(y3 - y1)) * (x3 - x1));

		sgl_fill_flat_bottom_triangle(canvas, x1, y1, x2, y2, x4, y2, color);
		sgl_fill_flat_top_triangle(canvas, x2, y2, x4, y2, x3, y3, color);
	}
}


void sgl_canvas_export_ppm(const SGL_Canvas canvas, const char * filename)
{
	if(canvas.pixels == NULL || filename == NULL){
		SGL_DRAW_ERROR("Invalid arguments specified for PPM disk serialization.");
		return;
	}
	FILE *f = fopen(filename, "wb");
	if(!f){
		SGL_DRAW_ERROR("Faild to create image file disk node target :%s",filename);
		return;
	}

	fprintf(f, "P6\n%d %d\n255\n",(int)canvas.width,(int)canvas.height);
	size_t total_pixels = canvas.width * canvas.height;
	for(size_t i = 0; i < total_pixels; ++i){
		uint8_t r = (canvas.pixels[i] >> 24) & 0xFF;
		uint8_t g = (canvas.pixels[i] >> 16) & 0xFF;
		uint8_t b = (canvas.pixels[i] >> 8) & 0xFF;

		fputc(r,f);
		fputc(g,f);
		fputc(b,f);
	}
	fclose(f);

	SGL_DRAW_INFO("Frame asset securely pushed down to disk path: %s",filename);
}


#endif //SGL_DRAW_IMPLEMENTATION_DOWN

