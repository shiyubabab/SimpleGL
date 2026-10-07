/*************************************************************************
	> File Name: sgl_draw.h
	> Author: mlxh
	> Mail: mlxh_gto@163.com 
	> Created Time: Mon 06 Jul 2026 12:15:46 PM CST
 ************************************************************************/

#ifndef SGL_DRAW_H
#define SGL_DRAW_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

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
#define SGL_RED(color)	 (((color) >> (3*8)) & 0xFF)
#define SGL_GREEN(color) (((color) >> (2*8)) & 0xFF)
#define SGL_BLUE(color)	 (((color) >> (1*8)) & 0xFF)
#define SGL_ALPHA(color) (((color) >> (0*8)) & 0xFF)
#define SGL_RGBA(r,g,b,a) ( (((r)&0xFF)<<(3*8)) | (((g)&0xFF)<<(2*8)) | (((b)&0xFF)<<(1*8)) | (((a)&0xFF)<<(0*8)) )

#ifndef SGL_DRAW_INFO
#define SGL_DRAW_INFO(fmt,...)  fprintf(stdout,"DRAW INFO %s %d: " fmt "\n" ,__func__,__LINE__,##__VA_ARGS__)
#define SGL_DRAW_ERROR(fmt,...) fprintf(stderr,"DRAW ERROR %s %d: " fmt "\n",__func__,__LINE__,##__VA_ARGS__)
#endif // SGL_DRAW_INFO

#ifndef SGL_CLAMP
#define SGL_CLAMP(x,low,hight) (((x) < (low))? (low) : (((x) > (hight))? (hight) : (x)))
#endif // SGL_CLAMP

typedef struct {
	uint32_t * pixels;
	size_t width;
	size_t height;
	size_t stride;
} SGL_Canvas;

void sgl_canvas_export_ppm(const SGL_Canvas canvas, const char * filename);

SGLDEF SGL_Canvas sgl_canvas(uint32_t *pixels, size_t width, size_t height, size_t stride)
{
	return (SGL_Canvas) {
		.pixels = pixels,
		.width	= width,
		.height = height,
		.stride	= stride,
	};
}

SGLDEF float* sgl_create_gaussian_kernel_1d(int radius, float sigma)
{
	int size = 2 * radius + 1;
	float* kernel = (float *)malloc(sizeof(float) * size);
	if(!kernel) return NULL;

	float sum = 0.0f;
	for(int i = -radius; i <= radius; ++i){
		float val = expf(-(float)(i*i) / (2.0f * sigma *sigma));
		kernel[i + radius] = val;
		sum += val;
	}

	for(int i = 0; i < size;++i){
		kernel[i] /= sum;
	}

	return kernel;
}

SGLDEF void sgl_gaussian_blur(SGL_Canvas canvas, int radius, float sigma)
{
	if(canvas.pixels == NULL || radius<= 0 || sigma <= 0.0f) return;

	int width = (int)canvas.width;
	int height = (int)canvas.height;

	float *kernel = sgl_create_gaussian_kernel_1d(radius,sigma);
	if(!kernel) return;
	
	uint32_t *temp = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
	if(!temp){
		free(kernel);
		return;
	}

	for(int y = 0;y<height;++y){
		for(int x = 0;x<width;++x){
			float r = 0.0f,g = 0.0f,b=0.0f,a=0.0f;
			for(int k = -radius;k<=radius;++k){
				int ix = SGL_CLAMP(x + k, 0, width - 1);
				uint32_t pixel = SGL_PIXEL_AT(canvas,ix,y);
				float weight = kernel[k + radius];

				r += (float)SGL_RED(pixel) * weight;
				g += (float)SGL_GREEN(pixel) * weight;
				b += (float)SGL_BLUE(pixel) * weight;
				a += (float)SGL_ALPHA(pixel) * weight;
			}

			temp[y * width + x] = SGL_RGBA((uint8_t)r,(uint8_t)g,(uint8_t)b,(uint8_t)a);
		}
	}

	for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

            for (int k = -radius; k <= radius; ++k) {
                int iy = SGL_CLAMP(y + k, 0, height - 1);
                uint32_t pixel = temp[iy * width + x];
                float weight = kernel[k + radius];

                r += (float)SGL_RED(pixel) * weight;
                g += (float)SGL_GREEN(pixel) * weight;
                b += (float)SGL_BLUE(pixel) * weight;
                a += (float)SGL_ALPHA(pixel) * weight;
            }

            SGL_PIXEL_AT(canvas, x, y) = SGL_RGBA((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
        }
    }

	free(kernel);
	free(temp);
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

SGLDEF uint32_t sgl_rgba_to_ycgco(uint32_t rgba)
{
	int r = (int)SGL_RED(rgba);
	int g = (int)SGL_GREEN(rgba);
	int b = (int)SGL_BLUE(rgba);
	uint8_t a = SGL_ALPHA(rgba);

	// YCgCo 正向变换 (带符号位的整数计算)
	int co = r - b;
	int t  = b + (co >> 1);
	int cg = g - t;
	int y  = t + (cg >> 1);

	// 映射到 [0, 255]
	uint8_t u_y  = (uint8_t)SGL_CLAMP(y, 0, 255);
	uint8_t u_cg = (uint8_t)SGL_CLAMP(cg + 128, 0, 255);
	uint8_t u_co = (uint8_t)SGL_CLAMP(co + 128, 0, 255);

	return SGL_RGBA(u_y, u_cg, u_co, a);
}

SGLDEF uint32_t sgl_ycgco_to_rgba(uint32_t ycgco)
{
    int y  = (int)SGL_RED(ycgco);
    int cg = (int)SGL_GREEN(ycgco) - 128;
    int co = (int)SGL_BLUE(ycgco) - 128;
    uint8_t a = SGL_ALPHA(ycgco);

    // YCgCo 逆向变换 (精准逆推公式)
    int t = y - (cg >> 1);
    int g = cg + t;
    int b = t - (co >> 1);
    int r = b + co;

    uint8_t u_r = (uint8_t)SGL_CLAMP(r, 0, 255);
    uint8_t u_g = (uint8_t)SGL_CLAMP(g, 0, 255);
    uint8_t u_b = (uint8_t)SGL_CLAMP(b, 0, 255);

    return SGL_RGBA(u_r, u_g, u_b, a);
}

/* 中值逻辑：clamp(a + b - c, min(a,b), max(a,b)) */
SGLDEF int32_t sgl_mmap_median_single(int32_t a, int32_t b, int32_t c)
{
	int32_t min_ab = (a < b) ? a : b;
	int32_t max_ab = (a > b) ? a : b;
	int32_t p = a + b - c;

	if (p < min_ab) return min_ab;
	if (p > max_ab) return max_ab;
	return p;
}

/**
 * @brief 对源 Canvas 进行 MMAP 预测，并将预测残差输出到 res Canvas
 * @param src 原始图像 Canvas
 * @param res 存储残差图的 Canvas (需与 src 尺寸一致)
 * @param visual_offset 可视化偏移量 (推荐传 128：将 0 残差映射为中性灰；若传 0 则输出绝对残差)
 */
SGLDEF void sgl_mmap_predict(SGL_Canvas src, SGL_Canvas res, int32_t visual_offset)
{
	if (!src.pixels || !res.pixels) return;
	if (src.width != res.width || src.height != res.height) return;

	int width = (int)src.width;
	int height = (int)src.height;

	// 为 R, G, B 三个通道分别申请上一行 (Line Buffer) 重建值，初始预设为 128 (8-bit 中点)
	uint8_t *prev_line_r = (uint8_t *)malloc(sizeof(uint8_t) * width);
	uint8_t *prev_line_g = (uint8_t *)malloc(sizeof(uint8_t) * width);
	uint8_t *prev_line_b = (uint8_t *)malloc(sizeof(uint8_t) * width);

	if (!prev_line_r || !prev_line_g || !prev_line_b) {
		free(prev_line_r); free(prev_line_g); free(prev_line_b);
		return;
	}

	for (int x = 0; x < width; ++x) {
		prev_line_r[x] = 128;
		prev_line_g[x] = 128;
		prev_line_b[x] = 128;
	}

	for (int y = 0; y < height; ++y) {
		// 每一行起始，左侧参考像素 a 默认置为 128
		uint8_t recon_a_r = 128;
		uint8_t recon_a_g = 128;
		uint8_t recon_a_b = 128;

		// 硬件 DSC 以 3 个像素为一个 Group 进行推进
		for (int x = 0; x < width; x += 3) {
			int group_len = (x + 3 <= width) ? 3 : (width - x);

			// ================= 取 RGB 3通道的参考点 (a, b, c, d) =================
			// R 通道
			int32_t ra = recon_a_r;
			int32_t rb = (y > 0) ? prev_line_r[x] : 128;
			int32_t rc = (y > 0 && x > 0) ? prev_line_r[x - 1] : ra;
			int32_t rd = (y > 0 && (x + 1) < width) ? prev_line_r[x + 1] : rb;

			// G 通道
			int32_t ga = recon_a_g;
			int32_t gb = (y > 0) ? prev_line_g[x] : 128;
			int32_t gc = (y > 0 && x > 0) ? prev_line_g[x - 1] : ga;
			int32_t gd = (y > 0 && (x + 1) < width) ? prev_line_g[x + 1] : gb;

			// B 通道
			int32_t ba = recon_a_b;
			int32_t bb = (y > 0) ? prev_line_b[x] : 128;
			int32_t bc = (y > 0 && x > 0) ? prev_line_b[x - 1] : ba;
			int32_t bd = (y > 0 && (x + 1) < width) ? prev_line_b[x + 1] : bb;

			// ================= 计算基准中值与右上方梯度 =================
			int32_t r_base = sgl_mmap_median_single(ra, rb, rc), r_grad = rd - rb;
			int32_t g_base = sgl_mmap_median_single(ga, gb, gc), g_grad = gd - gb;
			int32_t b_base = sgl_mmap_median_single(ba, bb, bc), b_grad = bd - bb;

			// ================= 遍历当前 Group 的 3 个像素 =================
			for (int i = 0; i < group_len; ++i) {
				int cur_x = x + i;
				uint32_t orig_pixel = SGL_PIXEL_AT(src, cur_x, y);

				uint8_t orig_r = SGL_RED(orig_pixel);
				uint8_t orig_g = SGL_GREEN(orig_pixel);
				uint8_t orig_b = SGL_BLUE(orig_pixel);
				uint8_t orig_a = SGL_ALPHA(orig_pixel);

				// 1. 推导当前像素的预测值 (带梯度修正)
				int32_t pred_r, pred_g, pred_b;
				if (i == 0) {
					pred_r = r_base;
					pred_g = g_base;
					pred_b = b_base;
				} else if (i == 1) {
					pred_r = r_base + (r_grad / 3);
					pred_g = g_base + (g_grad / 3);
					pred_b = b_base + (b_grad / 3);
				} else {
					pred_r = r_base + ((2 * r_grad) / 3);
					pred_g = g_base + ((2 * g_grad) / 3);
					pred_b = b_base + ((2 * b_grad) / 3);
				}

				pred_r = SGL_CLAMP(pred_r, 0, 255);
				pred_g = SGL_CLAMP(pred_g, 0, 255);
				pred_b = SGL_CLAMP(pred_b, 0, 255);

				// 2. 计算残差：Residual = Original - Predicted
				int32_t res_r = (int32_t)orig_r - pred_r;
				int32_t res_g = (int32_t)orig_g - pred_g;
				int32_t res_b = (int32_t)orig_b - pred_b;

				// 3. 将残差映射至 0~255 以便于写入 Canvas 可视化
				uint8_t out_r, out_g, out_b;
				if (visual_offset == 0) {
					// 模式 A：绝对值残差 (平滑区为纯黑，高频边缘亮起)
					out_r = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_r), 0, 255);
					out_g = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_g), 0, 255);
					out_b = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_b), 0, 255);
				} else {
					// 模式 B：带 128 偏移 (残差 0 呈现灰色 128，正负残差呈现亮/暗)
					out_r = (uint8_t)SGL_CLAMP(res_r + visual_offset, 0, 255);
					out_g = (uint8_t)SGL_CLAMP(res_g + visual_offset, 0, 255);
					out_b = (uint8_t)SGL_CLAMP(res_b + visual_offset, 0, 255);
				}

				SGL_PIXEL_AT(res, cur_x, y) = SGL_RGBA(out_r, out_g, out_b, orig_a);

				// 4. 更新 Line Buffer 与左侧重建点 recon_a（无损模式下重建值等于原始值）
				prev_line_r[cur_x] = orig_r;
				prev_line_g[cur_x] = orig_g;
				prev_line_b[cur_x] = orig_b;

				if (i == group_len - 1) {
					recon_a_r = orig_r;
					recon_a_g = orig_g;
					recon_a_b = orig_b;
				}
			}
		}
	}

	free(prev_line_r);
	free(prev_line_g);
	free(prev_line_b);
}

/**
 * @brief DSC 块预测 (Block Prediction)
 *        在左侧和上一行的已重建区域搜索最匹配的 1x3 像素块
 * @param src 原始 Canvas
 * @param res 存储残差图的 Canvas
 * @param search_vector_out 存储选择的矢量偏移 (便于调试分析 BP 命中率)
 * @param visual_offset 残差可视化偏移 (推荐 128 或 0)
 */
SGLDEF void sgl_block_predict(SGL_Canvas src, SGL_Canvas res, int32_t *search_vector_out, int32_t visual_offset)
{
	if (!src.pixels || !res.pixels) return;
	if (src.width != res.width || src.height != res.height) return;

	int width = (int)src.width;
	int height = (int)src.height;

	// DSC 预定义的候选矢量 (搜索位置偏移：dx, dy)
	// 包括：左侧像素、上一行同位置、上一行左右偏移等标准 BP 向量
	const int candidate_vectors[9][2] = {
		{-1,  0}, // Vector 0: 左侧相邻块
		{-3,  0}, // Vector 1: 左侧第3个像素
		{ 0, -1}, // Vector 2: 正上方
		{-1, -1}, // Vector 3: 左上方
		{ 1, -1}, // Vector 4: 右上方
		{-3, -1}, // Vector 5: 远左上方
		{ 3, -1}, // Vector 6: 远右上方
		{-6,  0}, // Vector 7: 远左
		{ 0, -2}  // Vector 8: 上上方
	};
	const int num_candidates = 9;

	for (int y = 0; y < height; ++y) {
		// DSC 硬件以 3 个像素为一个 Group 处理
		for (int x = 0; x < width; x += 3) {
			int group_len = (x + 3 <= width) ? 3 : (width - x);

			int best_vector_idx = -1;
			int32_t min_sad = 0x7FFFFFFF; // SAD (Absolute Difference Sum)

			// ================= 1. 矢量搜索：寻找 SAD 最小的参考块 =================
			for (int v = 0; v < num_candidates; ++v) {
				int ref_x = x + candidate_vectors[v][0];
				int ref_y = y + candidate_vectors[v][1];

				// 检查参考块的起始点和终点是否在有效区域内（只能参考已编码/重建的像素）
				if (ref_x < 0 || (ref_x + group_len) > width || ref_y < 0 || ref_y >= height) {
					continue;
				}

				// 计算 3 个像素在 R, G, B 通道上的 SAD 绝对误差和
				int32_t current_sad = 0;
				for (int i = 0; i < group_len; ++i) {
					uint32_t orig_p = SGL_PIXEL_AT(src, x + i, y);
					uint32_t ref_p  = SGL_PIXEL_AT(src, ref_x + i, ref_y);

					current_sad += SGL_ABS(int32_t, (int32_t)SGL_RED(orig_p)   - (int32_t)SGL_RED(ref_p));
					current_sad += SGL_ABS(int32_t, (int32_t)SGL_GREEN(orig_p) - (int32_t)SGL_GREEN(ref_p));
					current_sad += SGL_ABS(int32_t, (int32_t)SGL_BLUE(orig_p)  - (int32_t)SGL_BLUE(ref_p));
				}

				if (current_sad < min_sad) {
					min_sad = current_sad;
					best_vector_idx = v;
				}
			}

			// 保存当前 Group 的决策 Vector，方便外部分析 BP 命中率
			if (search_vector_out) {
				search_vector_out[(y * width + x) / 3] = best_vector_idx;
			}

			// ================= 2. 执行预测并生成残差 =================
			for (int i = 0; i < group_len; ++i) {
				int cur_x = x + i;
				uint32_t orig_pixel = SGL_PIXEL_AT(src, cur_x, y);

				int32_t pred_r = 128, pred_g = 128, pred_b = 128; // 无匹配时默认 128

				if (best_vector_idx != -1) {
					int ref_x = x + candidate_vectors[best_vector_idx][0] + i;
					int ref_y = y + candidate_vectors[best_vector_idx][1];
					uint32_t ref_pixel = SGL_PIXEL_AT(src, ref_x, ref_y);

					pred_r = SGL_RED(ref_pixel);
					pred_g = SGL_GREEN(ref_pixel);
					pred_b = SGL_BLUE(ref_pixel);
				}

				// 计算残差
				int32_t res_r = (int32_t)SGL_RED(orig_pixel)   - pred_r;
				int32_t res_g = (int32_t)SGL_GREEN(orig_pixel) - pred_g;
				int32_t res_b = (int32_t)SGL_BLUE(orig_pixel)  - pred_b;

				// 残差可视化输出
				uint8_t out_r, out_g, out_b;
				if (visual_offset == 0) {
					out_r = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_r), 0, 255);
					out_g = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_g), 0, 255);
					out_b = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_b), 0, 255);
				} else {
					out_r = (uint8_t)SGL_CLAMP(res_r + visual_offset, 0, 255);
					out_g = (uint8_t)SGL_CLAMP(res_g + visual_offset, 0, 255);
					out_b = (uint8_t)SGL_CLAMP(res_b + visual_offset, 0, 255);
				}

				SGL_PIXEL_AT(res, cur_x, y) = SGL_RGBA(out_r, out_g, out_b, SGL_ALPHA(orig_pixel));
			}
		}
	}
}

/**
 * @brief DSC 中点预测 (Midpoint Prediction / MPP)
 * @param src 原始图像 Canvas
 * @param res 存储残差图的 Canvas
 * @param visual_offset 残差可视化偏移 (推荐 128 或 0)
 */
SGLDEF void sgl_mpp_predict(SGL_Canvas src, SGL_Canvas res, int32_t visual_offset)
{
	if (!src.pixels || !res.pixels) return;
	if (src.width != res.width || src.height != res.height) return;

	int width = (int)src.width;
	int height = (int)src.height;

	// 对于标准 8-bit RGB，中点取 128
	const int32_t midpoint = 128;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint32_t orig_pixel = SGL_PIXEL_AT(src, x, y);

			int32_t orig_r = (int32_t)SGL_RED(orig_pixel);
			int32_t orig_g = (int32_t)SGL_GREEN(orig_pixel);
			int32_t orig_b = (int32_t)SGL_BLUE(orig_pixel);
			uint8_t orig_a = SGL_ALPHA(orig_pixel);

			// 1. MPP 核心：Residual = Original - Midpoint (128)
			int32_t res_r = orig_r - midpoint;
			int32_t res_g = orig_g - midpoint;
			int32_t res_b = orig_b - midpoint;

			// 2. 残差映射可视化
			uint8_t out_r, out_g, out_b;
			if (visual_offset == 0) {
				out_r = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_r), 0, 255);
				out_g = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_g), 0, 255);
				out_b = (uint8_t)SGL_CLAMP(SGL_ABS(int32_t, res_b), 0, 255);
			} else {
				out_r = (uint8_t)SGL_CLAMP(res_r + visual_offset, 0, 255);
				out_g = (uint8_t)SGL_CLAMP(res_g + visual_offset, 0, 255);
				out_b = (uint8_t)SGL_CLAMP(res_b + visual_offset, 0, 255);
			}

			SGL_PIXEL_AT(res, x, y) = SGL_RGBA(out_r, out_g, out_b, orig_a);
		}
	}
}

/**
 * @brief 综合预测选择器 (符合 VESA DSC 优先倾向与矢量 Bit 开销惩罚策略)
 * @param src 原始图像 Canvas
 * @param res 存储最终最优残差的 Canvas
 * @param mode_map 决策模式热力图 Canvas (红=MMAP, 绿=BP, 蓝=MPP)
 * @param visual_offset 可视化偏移量 (128 或 0)
 */
SGLDEF void sgl_dsc_adaptive_predict_reuse(SGL_Canvas src, SGL_Canvas res, SGL_Canvas *mode_map, int32_t visual_offset)
{
	if (!src.pixels || !res.pixels) return;
	if (src.width != res.width || src.height != res.height) return;

	int width = (int)src.width;
	int height = (int)src.height;

	// 1. 分配临时 Buffer 并构建三张残差 Canvas
	uint32_t *mmap_buf = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
	uint32_t *bp_buf   = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
	uint32_t *mpp_buf  = (uint32_t *)malloc(sizeof(uint32_t) * width * height);

	if (!mmap_buf || !bp_buf || !mpp_buf) {
		free(mmap_buf); free(bp_buf); free(mpp_buf);
		return;
	}

	SGL_Canvas res_mmap = sgl_canvas(mmap_buf, width, height, width);
	SGL_Canvas res_bp   = sgl_canvas(bp_buf, width, height, width);
	SGL_Canvas res_mpp  = sgl_canvas(mpp_buf, width, height, width);

	// 2. 复用独立的 3 个预测函数，以 0 偏移计算真实 SAD 绝对误差
	sgl_mmap_predict(src, res_mmap, 0);
	sgl_block_predict(src, res_bp, NULL, 0);
	sgl_mpp_predict(src, res_mpp, 0);

	// 3. Group 级 (3-Pixel) 裁决选择器
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; x += 3) {
			int group_len = (x + 3 <= width) ? 3 : (width - x);

			int32_t sad_mmap = 0, sad_bp = 0, sad_mpp = 0;

			// 统计当前 Group 内 3 个像素的通道 SAD 绝对误差和
			for (int i = 0; i < group_len; ++i) {
				uint32_t p_mmap = SGL_PIXEL_AT(res_mmap, x + i, y);
				uint32_t p_bp   = SGL_PIXEL_AT(res_bp, x + i, y);
				uint32_t p_mpp  = SGL_PIXEL_AT(res_mpp, x + i, y);

				sad_mmap += SGL_RED(p_mmap) + SGL_GREEN(p_mmap) + SGL_BLUE(p_mmap);
				sad_bp   += SGL_RED(p_bp)   + SGL_GREEN(p_bp)   + SGL_BLUE(p_bp);
				sad_mpp  += SGL_RED(p_mpp)  + SGL_GREEN(p_mpp)  + SGL_BLUE(p_mpp);
			}

			// ================= 关键修正：模式决策与开销惩罚 =================
			// 默认以 MMAP 为第一优先级（连续渐变区、平坦区最节省 Header/Vector 开销）
			SGL_Canvas *best_res_source = &res_mmap;
			uint32_t mode_color = SGL_RGBA(255, 0, 0, 255); // MMAP: 红色
			int32_t min_sad = sad_mmap;

			// 给 BP 增加开销惩罚项 (如 3 个像素累计惩罚 6)
			// 防止 BP 在纯色/平坦区仅凭“采样左侧像素”与 MMAP 抢占 0 SAD 导致无谓增加 Vector 开销
			int32_t bp_cost_penalty = 6;
			if ((sad_bp + bp_cost_penalty) < min_sad) {
				min_sad = sad_bp;
				best_res_source = &res_bp;
				mode_color = SGL_RGBA(0, 255, 0, 255); // BP: 绿色
			}

			// MPP 作为极端突变/高频噪点区下的兜底机制，同样赋予较高的开销惩罚
			int32_t mpp_cost_penalty = 12;
			if ((sad_mpp + mpp_cost_penalty) < min_sad) {
				min_sad = sad_mpp;
				best_res_source = &res_mpp;
				mode_color = SGL_RGBA(0, 0, 255, 255); // MPP: 蓝色
			}

			// 4. 将胜出的预测器残差写进目标 res Canvas
			for (int i = 0; i < group_len; ++i) {
				int cur_x = x + i;
				uint32_t abs_res_pixel = SGL_PIXEL_AT(*best_res_source, cur_x, y);

				if (visual_offset != 0) {
					uint8_t r = (uint8_t)SGL_CLAMP((int32_t)SGL_RED(abs_res_pixel) + visual_offset, 0, 255);
					uint8_t g = (uint8_t)SGL_CLAMP((int32_t)SGL_GREEN(abs_res_pixel) + visual_offset, 0, 255);
					uint8_t b = (uint8_t)SGL_CLAMP((int32_t)SGL_BLUE(abs_res_pixel) + visual_offset, 0, 255);
					SGL_PIXEL_AT(res, cur_x, y) = SGL_RGBA(r, g, b, SGL_ALPHA(abs_res_pixel));
				} else {
					SGL_PIXEL_AT(res, cur_x, y) = abs_res_pixel;
				}

				if (mode_map && mode_map->pixels) {
					SGL_PIXEL_AT(*mode_map, cur_x, y) = mode_color;
				}
			}
		}
	}

	free(mmap_buf);
	free(bp_buf);
	free(mpp_buf);
}

// QP 取值范围建议 0~15：QP=0 为无损，QP 越大压缩率越高、画质损失越大
static inline int32_t sgl_dsc_get_qstep(int32_t qp) {
    // 简单的指数或线性 Step 表（可根据需要调整）
    // QP:  0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15
    static const int32_t qstep_tbl[16] = {
        1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 80, 96, 112, 128
    };
    if (qp < 0) qp = 0;
    if (qp > 15) qp = 15;
    return qstep_tbl[qp];
}

// 残差量化
static inline int32_t sgl_dsc_quantize(int32_t res, int32_t qstep) {
    if (qstep <= 1) return res; // 无损
    int32_t offset = qstep >> 1;
    if (res >= 0) {
        return (res + offset) / qstep;
    } else {
        return (res - offset) / qstep;
    }
}

// 残差反量化
static inline int32_t sgl_dsc_dequantize(int32_t q_res, int32_t qstep) {
    if (qstep <= 1) return q_res;
    return q_res * qstep;
}

/**
 * @brief 带量化/反量化与闭环重建的自适应预测器
 * @param src 原始输入图像
 * @param res_out 输出的量化残差 Canvas (用于后续打包比特流)
 * @param recon_out 重建解压后的 Canvas (验证解码画质)
 * @param mode_map 模式分布热力图 Canvas
 * @param qp 量化参数 (0 为无损, >0 为有损压缩)
 */
SGLDEF void sgl_dsc_adaptive_predict_quantized(
    SGL_Canvas src,
    SGL_Canvas res_out,
    SGL_Canvas recon_out,
    SGL_Canvas *mode_map,
    int32_t qp)
{
    if (!src.pixels || !res_out.pixels || !recon_out.pixels) return;

    int width = (int)src.width;
    int height = (int)src.height;
    int32_t qstep = sgl_dsc_get_qstep(qp);

    // 1. 分配预测残差 Buffer
    uint32_t *mmap_buf = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
    uint32_t *bp_buf   = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
    uint32_t *mpp_buf  = (uint32_t *)malloc(sizeof(uint32_t) * width * height);

    if (!mmap_buf || !bp_buf || !mpp_buf) {
        free(mmap_buf); free(bp_buf); free(mpp_buf);
        return;
    }

    SGL_Canvas res_mmap = sgl_canvas(mmap_buf, width, height, width);
    SGL_Canvas res_bp   = sgl_canvas(bp_buf, width, height, width);
    SGL_Canvas res_mpp  = sgl_canvas(mpp_buf, width, height, width);

    // 注意：这里的预测应当基于重构成像（Reconstructed），简化版先基于原图计算候选 SAD
    sgl_mmap_predict(src, res_mmap, 0);
    sgl_block_predict(src, res_bp, NULL, 0);
    sgl_mpp_predict(src, res_mpp, 0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x += 3) {
            int group_len = (x + 3 <= width) ? 3 : (width - x);

            int32_t sad_mmap = 0, sad_bp = 0, sad_mpp = 0;

            for (int i = 0; i < group_len; ++i) {
                uint32_t p_mmap = SGL_PIXEL_AT(res_mmap, x + i, y);
                uint32_t p_bp   = SGL_PIXEL_AT(res_bp, x + i, y);
                uint32_t p_mpp  = SGL_PIXEL_AT(res_mpp, x + i, y);

                sad_mmap += SGL_RED(p_mmap) + SGL_GREEN(p_mmap) + SGL_BLUE(p_mmap);
                sad_bp   += SGL_RED(p_bp)   + SGL_GREEN(p_bp)   + SGL_BLUE(p_bp);
                sad_mpp  += SGL_RED(p_mpp)  + SGL_GREEN(p_mpp)  + SGL_BLUE(p_mpp);
            }

            // 决策与开销惩罚
            SGL_Canvas *best_res_source = &res_mmap;
            uint32_t mode_color = SGL_RGBA(255, 0, 0, 255); // MMAP: 红
            int32_t min_sad = sad_mmap;

            int32_t bp_cost_penalty = 6;
            if ((sad_bp + bp_cost_penalty) < min_sad) {
                min_sad = sad_bp;
                best_res_source = &res_bp;
                mode_color = SGL_RGBA(0, 255, 0, 255); // BP: 绿
            }

            int32_t mpp_cost_penalty = 12;
            if ((sad_mpp + mpp_cost_penalty) < min_sad) {
                min_sad = sad_mpp;
                best_res_source = &res_mpp;
                mode_color = SGL_RGBA(0, 0, 255, 255); // MPP: 蓝
            }

            // ================= 量化、反量化与像素重建 =================
            for (int i = 0; i < group_len; ++i) {
                int cur_x = x + i;

                // 1. 获取原始预测残差（RGB 原始 Signed Delta）
                uint32_t orig_p = SGL_PIXEL_AT(src, cur_x, y);
                uint32_t abs_res_pixel = SGL_PIXEL_AT(*best_res_source, cur_x, y);

                // 推导原始带符号残差 raw_res = src - pred
                // (由于 abs_res_pixel 存储的是 |src - pred|，这里演示完整 Quant/Dequant 流程)
                int32_t raw_r = (int32_t)SGL_RED(orig_p) - ((int32_t)SGL_RED(orig_p) - (int32_t)SGL_RED(abs_res_pixel));
                int32_t raw_g = (int32_t)SGL_GREEN(orig_p) - ((int32_t)SGL_GREEN(orig_p) - (int32_t)SGL_GREEN(abs_res_pixel));
                int32_t raw_b = (int32_t)SGL_BLUE(orig_p) - ((int32_t)SGL_BLUE(orig_p) - (int32_t)SGL_BLUE(abs_res_pixel));

                // 2. 残差量化 (Quantization)
                int32_t q_r = sgl_dsc_quantize(raw_r, qstep);
                int32_t q_g = sgl_dsc_quantize(raw_g, qstep);
                int32_t q_b = sgl_dsc_quantize(raw_b, qstep);

                // 将量化后的残差存入 res_out（为了直观可将 q_res 放回 unsigned byte 空间）
                SGL_PIXEL_AT(res_out, cur_x, y) = SGL_RGBA(
                    (uint8_t)SGL_CLAMP(q_r + 128, 0, 255),
                    (uint8_t)SGL_CLAMP(q_g + 128, 0, 255),
                    (uint8_t)SGL_CLAMP(q_b + 128, 0, 255),
                    255
                );

                // 3. 反量化 (Dequantization)
                int32_t deq_r = sgl_dsc_dequantize(q_r, qstep);
                int32_t deq_g = sgl_dsc_dequantize(q_g, qstep);
                int32_t deq_b = sgl_dsc_dequantize(q_b, qstep);

                // 4. 重建像素 (Reconstruction): Recon = Pred + Dequant_Res
                int32_t pred_r = (int32_t)SGL_RED(orig_p) - raw_r;
                int32_t pred_g = (int32_t)SGL_GREEN(orig_p) - raw_g;
                int32_t pred_b = (int32_t)SGL_BLUE(orig_p) - raw_b;

                uint8_t recon_r = (uint8_t)SGL_CLAMP(pred_r + deq_r, 0, 255);
                uint8_t recon_g = (uint8_t)SGL_CLAMP(pred_g + deq_g, 0, 255);
                uint8_t recon_b = (uint8_t)SGL_CLAMP(pred_b + deq_b, 0, 255);

                SGL_PIXEL_AT(recon_out, cur_x, y) = SGL_RGBA(recon_r, recon_g, recon_b, 255);

                if (mode_map && mode_map->pixels) {
                    SGL_PIXEL_AT(*mode_map, cur_x, y) = mode_color;
                }
            }
        }
    }

    free(mmap_buf); free(bp_buf); free(mpp_buf);
}

typedef enum {
	SGL_CABC_MODE_OFF = 0,	// 关闭
	SGL_CABC_MODE_UI,		// UI/文本模式(激进省电，容忍少量裁剪)
	SGL_CABC_MODE_PHOTO,	// 图片模式 (兼顾画质与省电)
	SGL_CABC_MODE_VIDEO,	// 视频模式 (注重色彩还原，平滑度极高)
} SGL_CABC_Mode;

typedef struct {
	SGL_CABC_Mode mode;
	float current_backlight;	// 当前平滑后的背光缩放系数 [0.1, 1.0]
	float target_backlight;		// 当前帧计算出的目标背光系数 [0.1, 1.0]
	float smoothing_alpha;		// IIR 低通滤波系数 (0.05 ~ 0.2, 越小过渡越平滑)
} SGL_CABC_Context;

SGLDEF SGL_CABC_Context sgl_cabc_init(SGL_CABC_Mode mode)
{
	SGL_CABC_Context ctx;
	ctx.mode = mode;
	ctx.current_backlight = 1.0f;
	ctx.target_backlight = 1.0f;

	switch(mode) {
		case SGL_CABC_MODE_UI:		ctx.smoothing_alpha = 0.20f; break;
		case SGL_CABC_MODE_PHOTO:	ctx.smoothing_alpha = 0.10f; break;
		case SGL_CABC_MODE_VIDEO:	ctx.smoothing_alpha = 0.05f; break;
		default:					ctx.smoothing_alpha = 1.00f; break;
	}
	return ctx;
}

SGLDEF float sgl_cabc_analyze_target_backlight(const SGL_Canvas src, SGL_CABC_Mode mode)
{
	if(!src.pixels || mode == SGL_CABC_MODE_OFF) return 1.0f;

	uint32_t histogram[256] = {0};
	size_t total_pixels = src.width * src.height;
	if(total_pixels == 0) return 1.0f;

	for(size_t y = 0; y < src.height; ++y){
		for(size_t x = 0; x < src.width; ++x){
			uint32_t pixel = SGL_PIXEL_AT(src, x, y);
			uint8_t r = SGL_RED(pixel);
			uint8_t g = SGL_GREEN(pixel);
			uint8_t b = SGL_BLUE(pixel);

			uint8_t luma = (uint8_t)((77 * r + 150 * g + 29 * b) >> 8);
			histogram[luma]++;
		}
	}

	// 根据模式设定允许的高光像素裁剪比例(Clipping Threshold)
	// UI模式允许 2% 的高光裁剪; PHOTO模式 0.5%; VIDEO 0.1%
	float clip_ratio = 0.005f;
	if (mode == SGL_CABC_MODE_UI)		clip_ratio = 0.020f;
	if (mode == SGL_CABC_MODE_VIDEO)	clip_ratio = 0.001f;

	size_t allowed_clip_count = (size_t)(total_pixels * clip_ratio);
	size_t accumulated_pixels = 0;
	uint8_t max_luma_threshold = 255;

	for(int i = 255; i >= 0; --i){
		accumulated_pixels += histogram[i];
		if(accumulated_pixels > allowed_clip_count){
			max_luma_threshold = (uint8_t)i; // 灰度值
			break;
		}
	}

	if(max_luma_threshold < 16) max_luma_threshold = 16;

	float target_backlight = (float)max_luma_threshold / 255.0f; // 背光需要调节的占比

	float min_backlight_limit = (mode == SGL_CABC_MODE_UI) ? 0.30f : 0.45f;
	return SGL_CLAMP(target_backlight, min_backlight_limit, 1.0f);
}

/**
 * @brief 核心算法 2：根据当前平滑后的背光因子，对图像 Canvas 进行像素级色彩补偿 (Pixel Gain Adjustment)
 */
SGLDEF void sgl_cabc_apply_pixel_gain(SGL_Canvas canvas, float backlight_factor)
{
    if (!canvas.pixels || backlight_factor >= 0.999f) return;

    // 增益 Gain = 1.0 / backlight_factor
    float gain = 1.0f / backlight_factor;

    // 构建预计算 LUT 表 (提高运行效率)
    uint8_t lut[256];
    for (int i = 0; i < 256; ++i) {
        int val = (int)(i * gain + 0.5f);
        lut[i] = (uint8_t)SGL_CLAMP(val, 0, 255);
    }

    // 遍历 Canvas 像素进行补偿
    for (size_t y = 0; y < canvas.height; ++y) {
        for (size_t x = 0; x < canvas.width; ++x) {
            uint32_t pixel = SGL_PIXEL_AT(canvas, x, y);

            uint8_t r = SGL_RED(pixel);
            uint8_t g = SGL_GREEN(pixel);
            uint8_t b = SGL_BLUE(pixel);
            uint8_t a = SGL_ALPHA(pixel);

            // 查表放大像素 RGB 通道
            uint8_t out_r = lut[r];
            uint8_t out_g = lut[g];
            uint8_t out_b = lut[b];

            SGL_PIXEL_AT(canvas, x, y) = SGL_RGBA(out_r, out_g, out_b, a);
        }
    }
}

/**
 * @brief 逐帧 CABC 处理入口 (Frame Processing Engine)
 * @param ctx CABC 上下文指针
 * @param canvas 输入输出图像 Canvas
 * @return float 返回当前帧应该下发给硬件背光 IC/驱动的背光亮度百分比 [0.3 ~ 1.0]
 */
SGLDEF float sgl_cabc_process_frame(SGL_CABC_Context *ctx, SGL_Canvas canvas)
{
    if (!ctx || ctx->mode == SGL_CABC_MODE_OFF) return 1.0f;

    // Step 1: 实时统计直方图，算出目标背光
    ctx->target_backlight = sgl_cabc_analyze_target_backlight(canvas, ctx->mode);

    // Step 2: IIR 低通滤波，消除画面突变引起的背光闪烁 (Breathing Effect)
    // current = current + alpha * (target - current)
    ctx->current_backlight += ctx->smoothing_alpha * (ctx->target_backlight - ctx->current_backlight);

    // Step 3: 根据平滑后的背光因子，对当前 Canvas 实施像素增益补偿
    sgl_cabc_apply_pixel_gain(canvas, ctx->current_backlight);

    // 返回当前帧对应的系统背光缩放系数
    return ctx->current_backlight;
}

typedef struct {
	float r;
	float g;
	float b;
	float a;
} SGL_ColorHDR;

SGLDEF SGL_ColorHDR sgl_color_hdr(float r,float g,float b,float a)
{
	return (SGL_ColorHDR){.r = r, .g = g, .b = b, .a = a};
}

/**
 * @brief ACES Film Tone Mapping 近似拟合算法 (Krzysztof Narkowicz)
 * @param x 输入的线形 HDR 通道值 (0.0 ~ +inf)
 * @return 映射后的 SDR 范围值 [0.0, 1.0]
 */
SGLDEF float sgl_aces_tonemap_channel(float x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return SGL_CLAMP((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

/**
 * @brief ACES 色彩映射 + Gamma 2.2 矫正
 * @param color HDR 颜色结构体
 * @param exposure 曝光系数 (默认 1.0f，值越大画面越亮)
 * @return 转换后的 32-bit RGBA 像素
 */
SGLDEF uint32_t sgl_color_hdr_to_rgba(SGL_ColorHDR color, float exposure)
{
	float r = color.r * exposure;
	float g = color.g * exposure;
	float b = color.b * exposure;

	r = sgl_aces_tonemap_channel(r);
	g = sgl_aces_tonemap_channel(g);
	b = sgl_aces_tonemap_channel(b);

	const float inv_gamma = 1.0f / 2.2f;
	r = powf(r,inv_gamma);
	g = powf(g,inv_gamma);
	b = powf(b,inv_gamma);

    uint8_t u_r = (uint8_t)SGL_CLAMP(r * 255.0f + 0.5f, 0.0f, 255.0f);
    uint8_t u_g = (uint8_t)SGL_CLAMP(g * 255.0f + 0.5f, 0.0f, 255.0f);
    uint8_t u_b = (uint8_t)SGL_CLAMP(b * 255.0f + 0.5f, 0.0f, 255.0f);
    uint8_t u_a = (uint8_t)SGL_CLAMP(color.a * 255.0f + 0.5f, 0.0f, 255.0f);

	return SGL_RGBA(u_r, u_g, u_b, u_a);
}

/**
 * @brief 将 HDR 浮点图像 Buffer (RGBA Float) 批量应用 ACES Tone Mapping 并绘制到 SDR Canvas 上
 * @param canvas 目标 8-bit Canvas[cite: 26]
 * @param hdr_pixels 尺寸必须与 canvas 一致的 SGL_ColorHDR 数组
 * @param exposure 曝光系数 (推荐 0.5f ~ 2.0f)
 */
SGLDEF void sgl_draw_hdr_buffer(SGL_Canvas canvas, const SGL_ColorHDR *hdr_pixels, float exposure)
{
    if (!canvas.pixels || !hdr_pixels) return;

    size_t total_pixels = canvas.width * canvas.height;
    for (size_t i = 0; i < total_pixels; ++i) {
        canvas.pixels[i] = sgl_color_hdr_to_rgba(hdr_pixels[i], exposure);
    }
}

/**
 * @brief 在 Canvas 上绘制带 ACES HDR 映射的圆形高光/发光体
 */
SGLDEF void sgl_hdr_circle(SGL_Canvas canvas, int cx, int cy, int r, SGL_ColorHDR hdr_color, float exposure)
{
    uint32_t color_sdr = sgl_color_hdr_to_rgba(hdr_color, exposure);
    sgl_circle_optimized(canvas, cx, cy, r, color_sdr);
}

#endif // SGL_DRAW_H

#if defined(SGL_DRAW_IMPLEMENTATION) && !defined(SGL_DRAW_IMPLEMENTATION_DOWN)
#define SGL_DRAW_IMPLEMENTATION_DOWN
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

