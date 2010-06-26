/* This file is in the public domain. */

#include <stdio.h>
#include <stdlib.h>

#include <gta/gta.h>

/* This example opens a three-dimensional GTA and then reads a small block of
 * array data.
 * This is useful for arrays that do not fit into memory. However, the input
 * file must be seekable (i.e. it must be a real file, no pipe or network
 * stream), and the GTA must not be compressed. */

int main(void)
{
    FILE *file;
    gta_header_t *header;
    gta_result_t r;
    off_t data_offset;
    uintmax_t block_indices_low[] = { 20, 30, 40 };
    uintmax_t block_indices_high[] = { 50, 60, 70 };
    void *block;

    /* Read the header */

    file = fopen("input.gta", "r");
    if (!file) {
        return 1;
    }
    r = gta_init_header(&header);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_read_header_from_stream(header, file);
    if (r != GTA_OK) {
        return 1;
    }
    /* We assume that the input GTA has three dimensions and one element
     * component of type GTA_UINT16, and that it is large enough to contain
     * the block defined above. */
    data_offset = ftello(file);
    if (data_offset == -1) {
        return 1;
    }

    /* Read the data block */

    block = malloc(31 * 31 * 31 * sizeof(uint16_t));
    if (!block) {
        return 1;
    }
    r = gta_read_block_from_stream(header, data_offset,
            block_indices_low, block_indices_high, block, file);
    if (r != GTA_OK) {
        return 1;
    }
    /* do something with the data */

    /* Cleanup */

    free(block);
    gta_deinit_header(header);
    fclose(file);
    return 0;
}
