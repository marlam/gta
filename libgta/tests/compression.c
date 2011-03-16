/*
 * compression.c
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

int main(void)
{
    gta_header_t *header;
    gta_result_t r;
    FILE *f;

    r = gta_init_header(&header);
    check(r == GTA_OK);

    /* Define an array */
    gta_type_t types[] = {
        GTA_FLOAT32,
        GTA_FLOAT32,
        GTA_FLOAT32,
    };
    r = gta_set_components(header, 3, types, NULL);
    check(r == GTA_OK);
    uintmax_t dims[] = { 100, 100 };
    r = gta_set_dimensions(header, 2, dims);
    check(r == GTA_OK);

    /* Check data size */
    uintmax_t element_size = gta_get_element_size(header);
    check(element_size == 4 + 4 + 4);
    uintmax_t elements = gta_get_elements(header);
    check(elements = 100 * 100);
    uintmax_t data_size = gta_get_data_size(header);
    check(data_size == element_size * elements);

    /* Create the array data */
    uint8_t *data = malloc(data_size);
    check(data);
    for (uintmax_t x = 0; x < 100; x++)
    {
        for (uintmax_t y = 0; y < 100; y++)
        {
            uintmax_t indices[2] = { x, y };
            float *element = gta_get_element(header, data, indices);
            element[0] = (float)(y * 100 + x) + 0.0f;
            element[1] = (float)(y * 100 + x) + 0.3f;
            element[2] = (float)(y * 100 + x) + 0.6f;
        }
    }

    /* Write the array to a file */
    f = fopen("test-compression.tmp", "w");
    check(f);
    // Compression GTA_NONE
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB
    gta_set_compression(header, GTA_ZLIB);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB1
    gta_set_compression(header, GTA_ZLIB1);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB2
    gta_set_compression(header, GTA_ZLIB2);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB3
    gta_set_compression(header, GTA_ZLIB3);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB4
    gta_set_compression(header, GTA_ZLIB4);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB5
    gta_set_compression(header, GTA_ZLIB5);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB6
    gta_set_compression(header, GTA_ZLIB6);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB7
    gta_set_compression(header, GTA_ZLIB7);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB8
    gta_set_compression(header, GTA_ZLIB8);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_ZLIB9
    gta_set_compression(header, GTA_ZLIB9);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_BZIP2
    gta_set_compression(header, GTA_BZIP2);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    // Compression GTA_XZ
    gta_set_compression(header, GTA_XZ);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    fclose(f);

    /* Reread the same file */
    f = fopen("test-compression.tmp", "r");
    check(f);
    uint8_t *data2 = malloc(data_size);
    check(data2);
    // Compression GTA_NONE
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_NONE);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB1
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB1);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB2
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB2);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB3
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB3);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB4
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB4);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB5
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB5);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB6
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB6);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB7
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB7);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB8
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB8);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_ZLIB9
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_ZLIB9);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_BZIP2
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_BZIP2);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    // Compression GTA_XZ
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 3);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 2);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_XZ);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);

    int c = fgetc(f);
    check(c == EOF && feof(f));

    fclose(f);
    remove("test-compression.tmp");
    free(data);
    free(data2);

    gta_deinit_header(header);

    return 0;
}
