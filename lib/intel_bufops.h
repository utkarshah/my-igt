#ifndef __INTEL_BUFOPS_H__
#define __INTEL_BUFOPS_H__

#include <stdint.h>
#include "igt_aux.h"
#include "intel_batchbuffer.h"

struct buf_ops;

#define INTEL_BUF_INVALID_ADDRESS (-1ull)
#define INTEL_BUF_NAME_MAXSIZE 32
struct intel_buf {
	struct buf_ops *bops;
	uint32_t handle;
	uint32_t tiling;
	uint32_t bpp;
	uint32_t compression;
	uint32_t swizzle_mode;
	uint32_t yuv_semiplanar_bpp;
	bool format_is_yuv;
	bool format_is_yuv_semiplanar;
	struct {
		uint32_t offset;
		uint32_t stride;
		uint32_t size;
	} surface[2];
	struct {
		uint32_t offset;
		uint32_t stride;
	} ccs[2];
	struct {
		uint32_t offset;
	} cc;
	struct {
		uint64_t offset;
		uint32_t ctx;
	} addr;

	/* CPU mapping */
	uint32_t *ptr;
	bool cpu_write;

	/* For debugging purposes */
	char name[INTEL_BUF_NAME_MAXSIZE + 1];
};

static inline bool intel_buf_compressed(const struct intel_buf *buf)
{
	return buf->compression != I915_COMPRESSION_NONE;
}

static inline unsigned int intel_buf_width(const struct intel_buf *buf)
{
	return buf->surface[0].stride / (buf->bpp / 8);
}

static inline unsigned int intel_buf_height(const struct intel_buf *buf)
{
	return buf->surface[0].size / buf->surface[0].stride;
}

static inline unsigned int
intel_buf_ccs_width(int gen, const struct intel_buf *buf)
{
	/*
	 * GEN12+: The AUX CCS unit size is 64 bytes mapping 4 main surface
	 * tiles. Thus the width of the CCS unit is 4*32=128 pixels on the
	 * main surface.
	 */
	if (gen >= 12)
		return DIV_ROUND_UP(intel_buf_width(buf), 128) * 64;

	return DIV_ROUND_UP(intel_buf_width(buf), 1024) * 128;
}

static inline unsigned int
intel_buf_ccs_height(int gen, const struct intel_buf *buf)
{
	/*
	 * GEN12+: The AUX CCS unit size is 64 bytes mapping 4 main surface
	 * tiles. Thus the height of the CCS unit is 32 pixel rows on the main
	 * surface.
	 */
	if (gen >= 12)
		return DIV_ROUND_UP(intel_buf_height(buf), 32);

	return DIV_ROUND_UP(intel_buf_height(buf), 512) * 32;
}

uint32_t intel_buf_bo_size(const struct intel_buf *buf);

struct buf_ops *buf_ops_create(int fd);
struct buf_ops *buf_ops_create_with_selftest(int fd);
void buf_ops_destroy(struct buf_ops *bops);
int buf_ops_get_fd(struct buf_ops *bops);

bool buf_ops_set_software_tiling(struct buf_ops *bops,
				 uint32_t tiling,
				 bool use_software_tiling);

void intel_buf_to_linear(struct buf_ops *bops, struct intel_buf *buf,
			 uint32_t *linear);

void linear_to_intel_buf(struct buf_ops *bops, struct intel_buf *buf,
			 uint32_t *linear);

bool buf_ops_has_hw_fence(struct buf_ops *bops, uint32_t tiling);
bool buf_ops_has_tiling_support(struct buf_ops *bops, uint32_t tiling);

void intel_buf_init(struct buf_ops *bops, struct intel_buf *buf,
		    int width, int height, int bpp, int alignment,
		    uint32_t tiling, uint32_t compression);
void intel_buf_close(struct buf_ops *bops, struct intel_buf *buf);

void intel_buf_init_using_handle(struct buf_ops *bops,
				 uint32_t handle,
				 struct intel_buf *buf,
				 int width, int height, int bpp, int alignment,
				 uint32_t req_tiling, uint32_t compression);

struct intel_buf *intel_buf_create(struct buf_ops *bops,
				   int width, int height,
				   int bpp, int alignment,
				   uint32_t req_tiling, uint32_t compression);
void intel_buf_destroy(struct intel_buf *buf);

void *intel_buf_cpu_map(struct intel_buf *buf, bool write);
void *intel_buf_device_map(struct intel_buf *buf, bool write);
void intel_buf_unmap(struct intel_buf *buf);
void intel_buf_flush_and_unmap(struct intel_buf *buf);

void intel_buf_print(const struct intel_buf *buf);
void intel_buf_dump(const struct intel_buf *buf, const char *filename);
const char *intel_buf_set_name(struct intel_buf *buf, const char *name);

void intel_buf_write_to_png(struct intel_buf *buf, const char *filename);
void intel_buf_write_aux_to_png(struct intel_buf *buf, const char *filename);

#endif
