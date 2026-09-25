/*
 * Copyright © 2026, VideoLAN and dav1d authors
 * All rights reserved.
 */

#include "config.h"

#include <string.h>

#include "common/intops.h"

#include "src/internal.h"
#include "src/motion_vectors.h"
#include "src/ref.h"

void dav1d_picture_set_motion_vectors(Dav1dPicture *const p,
                                      Dav1dMotionVector *const motion_vectors,
                                      Dav1dRef *const motion_vectors_ref,
                                      const size_t n_motion_vectors)
{
    dav1d_ref_dec(&p->motion_vectors_ref);
    p->motion_vectors = motion_vectors;
    p->n_motion_vectors = n_motion_vectors;
    p->motion_vectors_ref = motion_vectors_ref;
    if (motion_vectors_ref) dav1d_ref_inc(motion_vectors_ref);
}

int dav1d_motion_vectors_alloc(Dav1dFrameContext *const f) {
    if (!f->c->export_motion_vectors ||
        (f->frame_hdr->frame_type != DAV1D_FRAME_TYPE_INTER &&
         f->frame_hdr->frame_type != DAV1D_FRAME_TYPE_SWITCH))
        return 0;

    const size_t len = (size_t) f->b4_stride * f->h4;
    if (len > SIZE_MAX / sizeof(*f->motion_vectors_grid))
        return DAV1D_ERR(ENOMEM);

    f->motion_vectors_grid_ref = dav1d_ref_create(ALLOC_BLOCK,
        len * sizeof(*f->motion_vectors_grid));
    if (!f->motion_vectors_grid_ref)
        return DAV1D_ERR(ENOMEM);

    f->motion_vectors_grid = f->motion_vectors_grid_ref->data;
    f->motion_vectors_grid_len = len;
    memset(f->motion_vectors_grid, 0, len * sizeof(*f->motion_vectors_grid));
    return 0;
}

void dav1d_motion_vectors_capture(const Dav1dTaskContext *const t,
                                  const Av1Block *const b, const int bw4,
                                  const int bh4, const int w4, const int h4)
{
    const Dav1dFrameContext *const f = t->f;
    if (!f->motion_vectors_grid || b->intra)
        return;

    Dav1dMotionVector *const dst =
        &f->motion_vectors_grid[t->by * f->b4_stride + t->bx];
    const int n_refs = b->comp_type == COMP_INTER_NONE ? 1 : 2;
    *dst = (Dav1dMotionVector) {
        .dst_x = t->bx * 4,
        .dst_y = t->by * 4,
        .w = w4 * 4,
        .h = h4 * 4,
        .n_refs = n_refs,
    };

    if (n_refs == 2) dst->flags |= DAV1D_MV_FLAG_COMPOUND;
    if (b->motion_mode == MM_WARP) dst->flags |= DAV1D_MV_FLAG_WARP;
    if (b->inter_mode == GLOBALMV || b->inter_mode == GLOBALMV_GLOBALMV)
        dst->flags |= DAV1D_MV_FLAG_GLOBAL;
    if (f->frame_hdr->allow_intrabc) dst->flags |= DAV1D_MV_FLAG_INTRABC;

    for (int i = 0; i < n_refs; i++) {
        dst->motion_x[i] = b->mv[i].x;
        dst->motion_y[i] = b->mv[i].y;
        dst->ref[i] = b->ref[i];
        if (!f->frame_hdr->allow_intrabc) {
            const int slot = f->frame_hdr->refidx[b->ref[i]];
            dst->ref_slot[i] = slot;
            dst->ref_order_hint[i] = f->refp[b->ref[i]].p.frame_hdr->frame_offset;
        }
    }

    (void) bw4;
    (void) bh4;
}

int dav1d_motion_vectors_finalize(Dav1dFrameContext *const f) {
    if (!f->motion_vectors_grid)
        return 0;

    size_t n = 0;
    for (size_t i = 0; i < f->motion_vectors_grid_len; i++)
        n += f->motion_vectors_grid[i].n_refs != 0;

    Dav1dRef *ref = NULL;
    Dav1dMotionVector *motion_vectors = NULL;
    if (n) {
        ref = dav1d_ref_create(ALLOC_BLOCK, n * sizeof(*motion_vectors));
        if (!ref) return DAV1D_ERR(ENOMEM);
        motion_vectors = ref->data;
        for (size_t i = 0, j = 0; i < f->motion_vectors_grid_len; i++)
            if (f->motion_vectors_grid[i].n_refs)
                motion_vectors[j++] = f->motion_vectors_grid[i];
    }

    Dav1dContext *const c = (Dav1dContext *) f->c;
    dav1d_picture_set_motion_vectors(&f->sr_cur.p, motion_vectors, ref, n);
    dav1d_picture_set_motion_vectors(&f->cur, motion_vectors, ref, n);
    if (c->n_fc == 1) {
        dav1d_picture_set_motion_vectors(&c->out.p, motion_vectors, ref, n);
    } else {
        const unsigned idx = (unsigned) (f - c->fc);
        dav1d_picture_set_motion_vectors(&c->frame_thread.out_delayed[idx].p,
                                         motion_vectors, ref, n);
    }
    for (int i = 0; i < 8; i++)
        if (c->refs[i].p.p.frame_hdr == f->frame_hdr)
            dav1d_picture_set_motion_vectors(&c->refs[i].p.p, motion_vectors, ref, n);

    dav1d_ref_dec(&ref);
    dav1d_motion_vectors_unref(f);
    return 0;
}

void dav1d_motion_vectors_unref(Dav1dFrameContext *const f) {
    dav1d_ref_dec(&f->motion_vectors_grid_ref);
    f->motion_vectors_grid = NULL;
    f->motion_vectors_grid_len = 0;
}
