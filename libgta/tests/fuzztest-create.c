/*
 * fuzztest-create.c
 *
 * This file is part of libgta, a library that implements the Generic Tagged
 * Array (GTA) file format.
 *
 * Copyright (C) 2011
 * Martin Lambers <marlam@marlam.de>
 *
 * Libgta is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * Libgta is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Libgta. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gta/gta.h>

#define check(condition) \
    /* fprintf(stderr, "%s:%d: %s: Checking '%s'.\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition); */ \
    if (!(condition)) \
    { \
        fprintf(stderr, "%s:%d: %s: Check '%s' failed.\n", \
                __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition); \
        exit(1); \
    }

int rand_in_range(int low, int high)
{
    int r = rand();
    r %= high - low + 1;
    return r + low;
}

int main(int argc, char *argv[])
{
    const gta_type_t types[] = {
        GTA_BLOB, GTA_INT8, GTA_UINT8,
        GTA_INT16, GTA_UINT16, GTA_INT32, GTA_UINT32,
        GTA_INT64, GTA_UINT64, GTA_INT128, GTA_UINT128,
        GTA_FLOAT32, GTA_FLOAT64, GTA_FLOAT128,
        GTA_CFLOAT32, GTA_CFLOAT64, GTA_CFLOAT128
    };
    const int tag_variants = 3;
    const char *tag_names[] = { "TAG0", "X", "_?_" };
    const char *tag_values[] = { "", "42", "(*)" };
    const int max_total_size = 64 * 1024 * 1024;
    gta_header_t *header;
    gta_taglist_t *taglist;
    gta_result_t r;
    void *data;
    FILE *f;
    off_t header_size, total_size;

    check(argc == 4);
    check(argv[1]);
    check(argv[2]);
    check(argv[3]);
    srand(atoi(argv[1]));

    /* Create a header */
    r = gta_create_header(&header);
    check(r == GTA_OK);

    /* Set global tags */
    int global_tags = rand_in_range(0, 3);
    taglist = gta_get_global_taglist(header);
    check(taglist);
    for (int j = 0; j < global_tags; j++) {
        r = gta_set_tag(taglist,
                tag_names[rand_in_range(0, tag_variants - 1)],
                tag_values[rand_in_range(0, tag_variants - 1)]);
        check(r == GTA_OK);
    }

    /* Set dimensions + tags */
    int dims = rand_in_range(0, 5);
    if (dims > 0) {
        uintmax_t dimensions[5];
        for (int i = 0; i < dims; i++) {
            dimensions[i] = rand_in_range(1, 13);
        }
        r = gta_set_dimensions(header, dims, dimensions);
        check(r == GTA_OK);
        for (int i = 0; i < dims; i++) {
            int dim_tags = rand_in_range(0, 3);
            taglist = gta_get_dimension_taglist(header, i);
            check(taglist);
            for (int j = 0; j < dim_tags; j++) {
                r = gta_set_tag(taglist,
                        tag_names[rand_in_range(0, tag_variants - 1)],
                        tag_values[rand_in_range(0, tag_variants - 1)]);
                check(r == GTA_OK);
            }
        }
    }

    /* Set components + tags */
    int comps = rand_in_range(0, 5);
    if (comps > 0) {
        gta_type_t component_types[5];
        uintmax_t component_sizes[5];
        int type_blobs = 0;
        for (int i = 0; i < comps; i++) {
            component_types[i] = types[rand_in_range(0, 16)];
            if (component_types[i] == GTA_BLOB) {
                component_sizes[type_blobs++] = rand_in_range(1, 7);
            }
        }
        r = gta_set_components(header, comps, component_types, component_sizes);
        check(r == GTA_OK);
        for (int i = 0; i < comps; i++) {
            int comp_tags = rand_in_range(0, 3);
            taglist = gta_get_component_taglist(header, i);
            check(taglist);
            for (int j = 0; j < comp_tags; j++) {
                r = gta_set_tag(taglist,
                        tag_names[rand_in_range(0, tag_variants - 1)],
                        tag_values[rand_in_range(0, tag_variants - 1)]);
                check(r == GTA_OK);
            }
        }
    }

    /* Write the GTA to two files */
    check(gta_get_data_size(header) < (uintmax_t)max_total_size);
    data = calloc(gta_get_data_size(header), 1);
    check(data);
    f = fopen(argv[2], "w");
    check(f);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    header_size = ftello(f);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    total_size = ftello(f);
    check(total_size < max_total_size);
    fclose(f);
    f = fopen(argv[3], "w");
    check(f);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    fclose(f);
    free(data);

    gta_destroy_header(header);

    /* Now corrupt the second file */
    int max_corrupt_offset = header_size - 1;
    f = fopen(argv[3], "r+");
    check(f);
    int corruptions = rand_in_range(1, 4);
    for (int i = 0; i < corruptions; i++) {
        off_t offset = rand_in_range(0, max_corrupt_offset);
        r = fseek(f, offset, SEEK_SET);
        check(r == 0);
        unsigned char newbyte = rand_in_range(0, 255);
        r = fwrite(&newbyte, sizeof(unsigned char), 1, f);
        check(r == 1);
    }
    fclose(f);

    /* Print new seed value */
    printf("%d\n", rand());
    return 0;
}
