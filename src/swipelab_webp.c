// swipelab_webp.c - FFI wrapper for libwebp encoding
#include <stdlib.h>
#include <string.h>
#include "webp/encode.h"

#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

// Result structure for encoded WebP data
typedef struct {
    uint8_t* data;
    size_t size;
    int error;
} WebPEncodeResult;

// Encode RGBA image to WebP (lossy)
// Returns allocated data that must be freed with swipelab_webp_free
EXPORT WebPEncodeResult swipelab_webp_encode_rgba(
    const uint8_t* rgba,
    int width,
    int height,
    int stride,
    float quality
) {
    WebPEncodeResult result = {0};

    if (rgba == NULL || width <= 0 || height <= 0) {
        result.error = 1;
        return result;
    }

    uint8_t* output = NULL;
    size_t size = WebPEncodeRGBA(rgba, width, height, stride, quality, &output);

    if (size == 0 || output == NULL) {
        result.error = 2;
        return result;
    }

    result.data = output;
    result.size = size;
    result.error = 0;

    return result;
}

// Encode RGBA image to WebP (lossless)
EXPORT WebPEncodeResult swipelab_webp_encode_rgba_lossless(
    const uint8_t* rgba,
    int width,
    int height,
    int stride
) {
    WebPEncodeResult result = {0};

    if (rgba == NULL || width <= 0 || height <= 0) {
        result.error = 1;
        return result;
    }

    uint8_t* output = NULL;
    size_t size = WebPEncodeLosslessRGBA(rgba, width, height, stride, &output);

    if (size == 0 || output == NULL) {
        result.error = 2;
        return result;
    }

    result.data = output;
    result.size = size;
    result.error = 0;

    return result;
}

// Encode RGBA image to WebP (lossy) applying a pixel-orientation transform.
// Orientation values:
//   0 = normal, 1 = flipH, 2 = rot180, 3 = flipV,
//   4 = transpose, 5 = rot90CW, 6 = transverse, 7 = rot270CW.
EXPORT WebPEncodeResult swipelab_webp_encode_rgba_oriented(
    const uint8_t* rgba,
    int width,
    int height,
    int stride,
    float quality,
    int orientation
) {
    WebPEncodeResult result = {0};

    if (rgba == NULL || width <= 0 || height <= 0) {
        result.error = 1;
        return result;
    }

    if (orientation < 0 || orientation > 7) orientation = 0;

    if (orientation == 0) {
        return swipelab_webp_encode_rgba(rgba, width, height, stride, quality);
    }

    // Output dimensions are transposed for the diagonal/rotate-by-90 cases.
    const int swap = (orientation >= 4);
    const int outW = swap ? height : width;
    const int outH = swap ? width : height;
    const int outStride = outW * 4;

    uint8_t* out = (uint8_t*)malloc((size_t)outStride * (size_t)outH);
    if (out == NULL) {
        result.error = 3;
        return result;
    }

    // For each source pixel (x, y) compute destination (dx, dy) in the
    // upright frame and copy 4 bytes.
    for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow = rgba + (size_t)y * (size_t)stride;
        for (int x = 0; x < width; ++x) {
            int dx, dy;
            switch (orientation) {
                case 1: dx = width - 1 - x;  dy = y;               break;
                case 2: dx = width - 1 - x;  dy = height - 1 - y;  break;
                case 3: dx = x;              dy = height - 1 - y;  break;
                case 4: dx = y;              dy = x;               break;
                case 5: dx = height - 1 - y; dy = x;               break;
                case 6: dx = height - 1 - y; dy = width - 1 - x;   break;
                case 7: dx = y;              dy = width - 1 - x;   break;
                default: dx = x; dy = y; break;
            }
            uint8_t* dstPx = out + (size_t)dy * (size_t)outStride + (size_t)dx * 4;
            const uint8_t* srcPx = srcRow + (size_t)x * 4;
            dstPx[0] = srcPx[0];
            dstPx[1] = srcPx[1];
            dstPx[2] = srcPx[2];
            dstPx[3] = srcPx[3];
        }
    }

    uint8_t* output = NULL;
    size_t size = WebPEncodeRGBA(out, outW, outH, outStride, quality, &output);
    free(out);

    if (size == 0 || output == NULL) {
        result.error = 2;
        return result;
    }

    result.data = output;
    result.size = size;
    result.error = 0;
    return result;
}

// Free WebP encoded data
EXPORT void swipelab_webp_free(uint8_t* data) {
    if (data != NULL) {
        WebPFree(data);
    }
}

// Get encoder version
EXPORT int swipelab_webp_version(void) {
    return WebPGetEncoderVersion();
}
