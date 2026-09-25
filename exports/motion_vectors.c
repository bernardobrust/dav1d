/*
 * Modified Library Code
 * This file was created for test purpuses
 */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "dav1d/dav1d.h"

#include "input/input.h"

static void print_motion_vectors(const Dav1dPicture *const pic,
                                 const unsigned frame) {
    printf("frame=%u vectors=%zu\n", frame, pic->n_motion_vectors);
    for (size_t i = 0; i < pic->n_motion_vectors; i++) {
        const Dav1dMotionVector *const mv = &pic->motion_vectors[i];
        printf("  dst=%d,%d size=%ux%u refs=%u flags=0x%x "
               "mv0=%d,%d ref0=%d slot0=%u hint0=%u",
               mv->dst_x, mv->dst_y, mv->w, mv->h, mv->n_refs, mv->flags,
               mv->motion_x[0], mv->motion_y[0], mv->ref[0],
                   mv->ref_slot[0], mv->ref_order_hint[0]);
        if (mv->n_refs == 2)
            printf(" mv1=%d,%d ref1=%d slot1=%u hint1=%u",
                   mv->motion_x[1], mv->motion_y[1], mv->ref[1],
                   mv->ref_slot[1], mv->ref_order_hint[1]);
        putchar('\n');
    }
}

int main(const int argc, char *const argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <AV1 IVF, Annex-B, or Section 5 input>\n", argv[0]);
        return 1;
    }

    DemuxerContext *input = NULL;
    Dav1dContext *decoder = NULL;
    Dav1dData data = { 0 };
    Dav1dPicture pic = { 0 };
    Dav1dSettings settings;
    unsigned ignored[2], num_frames, frame = 0;
    int res;

    if ((res = input_open(&input, NULL, argv[1], ignored, &num_frames, ignored)) < 0) {
        fprintf(stderr, "Unable to open input: %s\n", strerror(-res));
        return 1;
    }

    dav1d_default_settings(&settings);
    settings.export_motion_vectors = 1;
    if ((res = dav1d_open(&decoder, &settings)) < 0) {
        fprintf(stderr, "Unable to open decoder: %s\n", strerror(-res));
        input_close(input);
        return 1;
    }

    while ((res = input_read(input, &data)) >= 0) {
        while (data.sz) {
            res = dav1d_send_data(decoder, &data);
            if (res < 0 && res != DAV1D_ERR(EAGAIN)) goto error;
            while ((res = dav1d_get_picture(decoder, &pic)) == 0) {
                print_motion_vectors(&pic, frame++);
                dav1d_picture_unref(&pic);
            }
            if (res != DAV1D_ERR(EAGAIN)) goto error;
        }
    }
    while ((res = dav1d_get_picture(decoder, &pic)) == 0) {
        print_motion_vectors(&pic, frame++);
        dav1d_picture_unref(&pic);
    }
    if (res != DAV1D_ERR(EAGAIN)) goto error;

    dav1d_close(&decoder);
    input_close(input);
    return 0;

error:
    fprintf(stderr, "Decode failed: %s\n", strerror(-res));
    dav1d_data_unref(&data);
    dav1d_picture_unref(&pic);
    dav1d_close(&decoder);
    input_close(input);
    return 1;
}
