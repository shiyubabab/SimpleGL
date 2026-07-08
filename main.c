/*************************************************************************
	> File Name: main.c
	> Author: mlxh
	> Mail: mlxh_gto@163.com 
	> Created Time: Wed 24 Jun 2026 10:54:00 PM CST
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#define SGL_DRAW_IMPLEMENTATION
#include "draw/sgl_draw.h"

#define WIDTH  800
#define HEIGHT 800

uint32_t pixels[WIDTH * HEIGHT];
 
int main(void)
{
	SGL_Canvas canvas = sgl_canvas(pixels,WIDTH,HEIGHT,WIDTH);
	sgl_fill(canvas,0x000000FF);

#ifdef TEST_RECT
	int h = 80;
	int w = 100;
	sgl_rect(canvas,WIDTH/2-w/2,HEIGHT/2-h/2,WIDTH/2-w/2 + w,HEIGHT/2-h/2 + h,0xFF0000FF);
	sgl_canvas_export_ppm(canvas,"./output/rect_ex.ppm");
#endif

#ifdef TEST_CIRCLE
	int r = 40;
	int d = 2*r;
	int col = WIDTH / d;
	int row = HEIGHT/ d;

#define LERP(a,b,p) ((a)+((b)-(a))*(p))

	for(int i = 0; i < row; ++i){
		for(int j = 0; j < col; ++j){
			int cx = j*d+r, cy = i*d+r;
			float cxd = (float)j/(col-1), cyd = (float)i/(row-1) ;
			float p = cxd + cyd / 2.0f;
			sgl_circle(canvas,cx,cy,(int)LERP(r/8,r/2,p),0xFF0000FF);
		}
	}
	sgl_canvas_export_ppm(canvas,"./output/circle_ex.ppm");
#endif


#ifdef TEST_LINE
	sgl_line(canvas,0,0,799,799,0xFF0000FF);
	sgl_line(canvas,799,0,0,799,0x00FF00FF);
	sgl_line(canvas,400,0,400,799,0x0000FFFF);
	sgl_line(canvas,0,400,799,400,0xFF0000FF);

	sgl_canvas_export_ppm(canvas,"./output/line_ex.ppm");
#endif

#ifdef TEST_TRIANGLE
	sgl_fill_triangle(canvas,WIDTH/2,0,0,HEIGHT/2,WIDTH,HEIGHT,0xFF0000FF);
	sgl_canvas_export_ppm(canvas,"./output/triangle_ex.ppm");
#endif


#ifdef TEST_ALPHA
	sgl_rect(canvas,0,0,WIDTH*3/4,HEIGHT*3/4,0xFF0000FF);
	sgl_rect(canvas,WIDTH*1/4,HEIGHT*1/4,WIDTH-1,HEIGHT-1,0x00FF00AA);
	sgl_circle(canvas,WIDTH/2,HEIGHT/2,WIDTH*1/4,0x0000FF20);
	sgl_fill_triangle(canvas,WIDTH/2,0,0,HEIGHT/2,WIDTH,HEIGHT,0x00505520);
	sgl_canvas_export_ppm(canvas,"./output/alpha_ex.ppm");
#endif

	return 0;
}
