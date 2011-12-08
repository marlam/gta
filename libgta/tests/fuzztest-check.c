/*
 * fuzztest-check.c
 *
 * This file is part of libgta, a library that implements the Generic Tagged
 * Array (GTA) file format.
 *
 * Copyright (C) 2010, 2011
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

int main(int argc, char *argv[])
{
    const int max_data_size = 64 * 1024 * 1024;
    gta_header_t *iheader;
    gta_header_t *oheader;
    gta_io_state_t *istate;
    gta_io_state_t *ostate;
    FILE *f;
    FILE *nullf;
    off_t data_offset;
    void *data;
    int r;

    check(argc == 3);
    check(argv[1]);
    check(argv[2]);

    nullf = fopen("/dev/null", "w");
    check(nullf);

    /* First check if we can read and write the valid GTA */

    f = fopen(argv[1], "r");
    check(f);
    r = gta_create_header(&iheader);
    check(r == GTA_OK);
    r = gta_create_header(&oheader);
    check(r == GTA_OK);
    r = gta_read_header_from_stream(iheader, f);
    check(r == GTA_OK);
    data_offset = ftello(f);
    check(data_offset > 0);
    r = gta_clone_header(oheader, iheader);
    check(r == GTA_OK);
    /* Read/write/skip the data in one block */
    check(gta_get_data_size(iheader) <= (uintmax_t)max_data_size);
    data = malloc(gta_get_data_size(iheader));
    check(data);
    r = gta_read_data_from_stream(iheader, data, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(oheader, data, nullf);
    check(r == GTA_OK);
    free(data);
    r = fseeko(f, data_offset, SEEK_SET);
    check(r == 0);
    r = gta_skip_data_from_stream(iheader, f);
    check(r == GTA_OK);
    r = fseeko(f, data_offset, SEEK_SET);
    check(r == 0);
    r = gta_copy_data_stream(iheader, f, oheader, nullf);
    check(r == GTA_OK);
    if (gta_get_compression(oheader) == GTA_NONE)
        gta_set_compression(oheader, GTA_ZLIB);
    else
        gta_set_compression(oheader, GTA_NONE);
    r = fseeko(f, data_offset, SEEK_SET);
    check(r == 0);
    r = gta_copy_data_stream(iheader, f, oheader, nullf);
    check(r == GTA_OK);
    /* Read/write the data element-wise */
    if (gta_get_element_size(iheader) > 0 && gta_get_element_size(iheader) <= (uintmax_t)max_data_size) {
        r = fseeko(f, data_offset, SEEK_SET);
        check(r == 0);
        r = gta_create_io_state(&istate);
        check(r == GTA_OK);
        r = gta_create_io_state(&ostate);
        check(r == GTA_OK);
        data = malloc(gta_get_element_size(iheader));
        check(data);
        for (uintmax_t i = 0; i < gta_get_elements(iheader); i++) {
            r = gta_read_elements_from_stream(iheader, istate, 1, data, f);
            check(r == GTA_OK);
            r = gta_write_elements_to_stream(oheader, ostate, 1, data, nullf);
            check(r == GTA_OK);
        }
        gta_destroy_io_state(istate);
        gta_destroy_io_state(ostate);
        r = fseeko(f, data_offset, SEEK_SET);
        check(r == 0);
        r = gta_create_io_state(&istate);
        check(r == GTA_OK);
        r = gta_create_io_state(&ostate);
        check(r == GTA_OK);
        if (gta_get_compression(oheader) == GTA_NONE)
            gta_set_compression(oheader, GTA_ZLIB);
        else
            gta_set_compression(oheader, GTA_NONE);
        for (uintmax_t i = 0; i < gta_get_elements(iheader); i++) {
            r = gta_read_elements_from_stream(iheader, istate, 1, data, f);
            check(r == GTA_OK);
            r = gta_write_elements_to_stream(oheader, ostate, 1, data, nullf);
            check(r == GTA_OK);
        }
        free(data);
        gta_destroy_io_state(istate);
        gta_destroy_io_state(ostate);
    }
    gta_destroy_header(iheader);
    gta_destroy_header(oheader);
    fclose(f);


    /* Now try the corrupt GTA */

    f = fopen(argv[2], "r");
    check(f);
    r = gta_create_header(&iheader);
    check(r == GTA_OK);
    r = gta_read_header_from_stream(iheader, f);
    if (r == GTA_OK && gta_get_data_size(iheader) <= (uintmax_t)max_data_size) {
        data_offset = ftello(f);
        check(data_offset > 0);
        r = gta_create_header(&oheader);
        check(r == GTA_OK);
        r = gta_clone_header(oheader, iheader);
        check(r == GTA_OK);
        /* Read/write/skip the data in one block */
        data = malloc(gta_get_data_size(iheader));
        check(data);
        r = gta_read_data_from_stream(iheader, data, f);
        r = gta_write_data_to_stream(oheader, data, nullf);
        r = fseeko(f, data_offset, SEEK_SET);
        check(r == 0);
        r = gta_skip_data_from_stream(iheader, f);
        r = fseeko(f, data_offset, SEEK_SET);
        check(r == 0);
        r = gta_copy_data_stream(iheader, f, oheader, nullf);
        r = fseeko(f, data_offset, SEEK_SET);
        check(r == 0);
        if (gta_get_compression(oheader) == GTA_NONE)
            gta_set_compression(oheader, GTA_ZLIB);
        else
            gta_set_compression(oheader, GTA_NONE);
        r = gta_copy_data_stream(iheader, f, oheader, nullf);
        free(data);
        /* Read/write the data element-wise */
        if (gta_get_element_size(iheader) > 0 && gta_get_element_size(iheader) <= (uintmax_t)max_data_size) {
            r = fseeko(f, data_offset, SEEK_SET);
            check(r == 0);
            r = gta_create_io_state(&istate);
            check(r == GTA_OK);
            r = gta_create_io_state(&ostate);
            check(r == GTA_OK);
            data = malloc(gta_get_element_size(iheader));
            check(data);
            for (uintmax_t i = 0; i < gta_get_elements(iheader); i++) {
                r = gta_read_elements_from_stream(iheader, istate, 1, data, f);
                if (r != GTA_OK)
                    break;
            }
            for (uintmax_t i = 0; i < gta_get_elements(iheader); i++) {
                r = gta_write_elements_to_stream(oheader, ostate, 1, data, nullf);
                if (r != GTA_OK)
                    break;
            }
            gta_destroy_io_state(istate);
            gta_destroy_io_state(ostate);
            r = fseeko(f, data_offset, SEEK_SET);
            check(r == 0);
            r = gta_create_io_state(&istate);
            check(r == GTA_OK);
            r = gta_create_io_state(&ostate);
            check(r == GTA_OK);
            if (gta_get_compression(oheader) == GTA_NONE)
                gta_set_compression(oheader, GTA_ZLIB);
            else
                gta_set_compression(oheader, GTA_NONE);
            for (uintmax_t i = 0; i < gta_get_elements(iheader); i++) {
                r = gta_read_elements_from_stream(iheader, istate, 1, data, f);
                if (r != GTA_OK)
                    break;
            }
            for (uintmax_t i = 0; i < gta_get_elements(iheader); i++) {
                r = gta_write_elements_to_stream(oheader, ostate, 1, data, nullf);
                if (r != GTA_OK)
                    break;
            }
            free(data);
            gta_destroy_io_state(istate);
            gta_destroy_io_state(ostate);
        }
        gta_destroy_header(oheader);
    }
    gta_destroy_header(iheader);
    fclose(f);

    return 0;
}
