/*
 * Modified Library Code
 */

#ifndef DAV1D_MOTION_VECTORS_H
#define DAV1D_MOTION_VECTORS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum Dav1dMotionVectorFlags {
    DAV1D_MV_FLAG_COMPOUND = 1 << 0,
    DAV1D_MV_FLAG_GLOBAL   = 1 << 1,
    DAV1D_MV_FLAG_WARP     = 1 << 2,
    DAV1D_MV_FLAG_INTRABC  = 1 << 3,
};

/**
 * Motion data for one inter-coded AV1 leaf block. Coordinates and dimensions
 * are in coded-picture pixels. Motion vectors are in AV1 1/8-pixel units.
 * ref[] indexes the seven logical AV1 references; ref_slot[] is the matching
 * reference-frame slot; ref_order_hint[] is that reference's order hint.
 * The reference fields are undefined when DAV1D_MV_FLAG_INTRABC is set.
 */
typedef struct Dav1dMotionVector {
    int32_t dst_x, dst_y;
    uint16_t w, h;
    int32_t motion_x[2], motion_y[2];
    int8_t ref[2];
    uint8_t ref_slot[2];
    uint8_t ref_order_hint[2];
    uint8_t n_refs;
    uint8_t flags;
} Dav1dMotionVector;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DAV1D_MOTION_VECTORS_H */
