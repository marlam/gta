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
    gta_header_t *header;
    gta_io_state_t *io_state;
    FILE *f;
    off_t data_offset;
    void *data;
    int r;

    check(argc == 3);
    check(argv[1]);
    check(argv[2]);

    r = gta_create_header(&header);
    check(r == GTA_OK);
    r = gta_create_io_state(&io_state);
    check(r == GTA_OK);

    /* First check if we can read the valid GTA */

    f = fopen(argv[1], "r");
    check(f);
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    data_offset = ftello(f);
    check(data_offset > 0);
    /* Read the data in one block */
    check(gta_get_data_size(header) <= (uintmax_t)max_data_size);
    data = malloc(gta_get_data_size(header));
    check(data);
    r = gta_read_data_from_stream(header, data, f);
    check(r == GTA_OK);
    free(data);
    /* Read the data element-wise */
    if (gta_get_element_size(header) > 0 && gta_get_element_size(header) <= (uintmax_t)max_data_size) {
        r = fseeko(f, data_offset, SEEK_SET);
        check(r == 0);
        data = malloc(gta_get_element_size(header));
        check(data);
        for (uintmax_t i = 0; i < gta_get_elements(header); i++) {
            r = gta_read_elements_from_stream(header, io_state, 1, data, f);
            check(r == GTA_OK);
        }
        free(data);
    }
    fclose(f);

    gta_destroy_header(header);
    r = gta_create_header(&header);
    check(r == GTA_OK);
    gta_destroy_io_state(io_state);
    r = gta_create_io_state(&io_state);
    check(r == GTA_OK);

    /* Now try the corrupt GTA */

    f = fopen(argv[2], "r");
    check(f);
    r = gta_read_header_from_stream(header, f);
    if (r == GTA_OK && gta_get_data_size(header) <= (uintmax_t)max_data_size) {
        data_offset = ftello(f);
        check(data_offset > 0);
        /* Read the data in one block */
        data = malloc(gta_get_data_size(header));
        check(data);
        r = gta_read_data_from_stream(header, data, f);
        free(data);
        /* Read the data element-wise */
        if (gta_get_element_size(header) > 0 && gta_get_element_size(header) <= (uintmax_t)max_data_size) {
            r = fseeko(f, data_offset, SEEK_SET);
            check(r == 0);
            check(gta_get_element_size(header) <= gta_get_data_size(header));
            data = malloc(gta_get_element_size(header));
            check(data);
            for (uintmax_t i = 0; i < gta_get_elements(header); i++) {
                r = gta_read_elements_from_stream(header, io_state, 1, data, f);
                if (r != GTA_OK)
                    break;
            }
            free(data);
        }
    }
    fclose(f);

    return 0;
}
