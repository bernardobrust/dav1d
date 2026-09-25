/*
 * Copyright © 2026, VideoLAN and dav1d authors
 * All rights reserved.
 */

#ifndef DAV1D_SRC_MOTION_VECTORS_H
#define DAV1D_SRC_MOTION_VECTORS_H

#include <stddef.h>

typedef struct Dav1dFrameContext Dav1dFrameContext;
typedef struct Dav1dTaskContext Dav1dTaskContext;
typedef struct Av1Block Av1Block;
typedef struct Dav1dPicture Dav1dPicture;
typedef struct Dav1dRef Dav1dRef;
typedef struct Dav1dMotionVector Dav1dMotionVector;

int dav1d_motion_vectors_alloc(Dav1dFrameContext *f);
void dav1d_motion_vectors_capture(const Dav1dTaskContext *t,
                                  const Av1Block *b, int bw4, int bh4,
                                  int w4, int h4);
int dav1d_motion_vectors_finalize(Dav1dFrameContext *f);
void dav1d_motion_vectors_unref(Dav1dFrameContext *f);
void dav1d_picture_set_motion_vectors(Dav1dPicture *p,
                                      Dav1dMotionVector *motion_vectors,
                                      Dav1dRef *motion_vectors_ref,
                                      size_t n_motion_vectors);

#endif /* DAV1D_SRC_MOTION_VECTORS_H */
