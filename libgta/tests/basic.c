/*
 * basic.c
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

    r = gta_create_header(&header);
    check(r == GTA_OK);

    check(gta_get_element_size(header) == 0);
    check(gta_get_elements(header) == 0);
    check(gta_get_dimensions(header) == 0);
    check(gta_get_data_size(header) == 0);

    /* Write an empty GTA */
    f = fopen("test-basic.tmp", "w");
    check(f);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, NULL, f);
    check(r == GTA_OK);

    /* Define an array */
    gta_type_t types[] = {
        GTA_BLOB,
        GTA_INT8,
        GTA_UINT8,
        GTA_INT16,
        GTA_UINT16,
        GTA_INT32,
        GTA_UINT32,
        GTA_INT64,
        GTA_UINT64,
        GTA_INT128,
        GTA_UINT128,
        GTA_FLOAT32,
        GTA_FLOAT64,
        GTA_FLOAT128,
        GTA_CFLOAT32,
        GTA_CFLOAT64,
        GTA_CFLOAT128
    };
    uintmax_t sizes[] = { 23 };
    r = gta_set_components(header, 17, types, sizes);
    check(r == GTA_OK);
    uintmax_t dims[] = { 10, 20, 30 };
    r = gta_set_dimensions(header, 3, dims);
    check(r == GTA_OK);

    /* Check data size */
    uintmax_t element_size = gta_get_element_size(header);
    check(element_size == 23 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16 + 4 + 8 + 16 + 8 + 16 + 32);
    uintmax_t elements = gta_get_elements(header);
    check(elements == 10 * 20 * 30);
    uintmax_t data_size = gta_get_data_size(header);
    check(data_size == element_size * elements);

    /* Create the array data */
    uint8_t *data = malloc(data_size);
    check(data);
    for (uintmax_t x = 0; x < 10; x++)
    {
        for (uintmax_t y = 0; y < 20; y++)
        {
            for (uintmax_t z = 0; z < 30; z++)
            {
                uintmax_t indices[3] = { x, y, z };
                void *element = gta_get_element(header, data, indices);
                uintmax_t i = z * (20 * 10) + y * 10 + x;
                void *element2 = gta_get_element_linear(header, data, i);
                check(element == element2);
                memset(gta_get_component(header, element, 0), 23, 23);
                memset(gta_get_component(header, element, 1), 1, 1);
                memset(gta_get_component(header, element, 2), 2, 1);
                memset(gta_get_component(header, element, 3), 3, 2);
                memset(gta_get_component(header, element, 4), 4, 2);
                memset(gta_get_component(header, element, 5), 5, 4);
                memset(gta_get_component(header, element, 6), 6, 4);
                memset(gta_get_component(header, element, 7), 7, 8);
                memset(gta_get_component(header, element, 8), 8, 8);
                memset(gta_get_component(header, element, 9), 9, 16);
                memset(gta_get_component(header, element, 10), 10, 16);
                memset(gta_get_component(header, element, 11), 11, 4);
                memset(gta_get_component(header, element, 12), 12, 8);
                memset(gta_get_component(header, element, 13), 13, 16);
                memset(gta_get_component(header, element, 14), 14, 8);
                memset(gta_get_component(header, element, 15), 15, 16);
                memset(gta_get_component(header, element, 16), 16, 32);
            }
        }
    }

    /* Write the array to the same file */
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);

    fclose(f);

    /* Reread the same file */
    f = fopen("test-basic.tmp", "r");
    check(f);
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 0);
    check(gta_get_element_size(header) == 0);
    check(gta_get_dimensions(header) == 0);
    check(gta_get_data_size(header) == 0);
    r = gta_read_data_from_stream(header, data, f);
    check(r == GTA_OK);
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 17);
    check(gta_get_component_type(header, 0) == GTA_BLOB);
    check(gta_get_component_size(header, 0) == 23);
    check(gta_get_component_type(header, 1) == GTA_INT8);
    check(gta_get_component_size(header, 1) == 1);
    check(gta_get_component_type(header, 2) == GTA_UINT8);
    check(gta_get_component_size(header, 2) == 1);
    check(gta_get_component_type(header, 3) == GTA_INT16);
    check(gta_get_component_size(header, 3) == 2);
    check(gta_get_component_type(header, 4) == GTA_UINT16);
    check(gta_get_component_size(header, 4) == 2);
    check(gta_get_component_type(header, 5) == GTA_INT32);
    check(gta_get_component_size(header, 5) == 4);
    check(gta_get_component_type(header, 6) == GTA_UINT32);
    check(gta_get_component_size(header, 6) == 4);
    check(gta_get_component_type(header, 7) == GTA_INT64);
    check(gta_get_component_size(header, 7) == 8);
    check(gta_get_component_type(header, 8) == GTA_UINT64);
    check(gta_get_component_size(header, 8) == 8);
    check(gta_get_component_type(header, 9) == GTA_INT128);
    check(gta_get_component_size(header, 9) == 16);
    check(gta_get_component_type(header, 10) == GTA_UINT128);
    check(gta_get_component_size(header, 10) == 16);
    check(gta_get_component_type(header, 11) == GTA_FLOAT32);
    check(gta_get_component_size(header, 11) == 4);
    check(gta_get_component_type(header, 12) == GTA_FLOAT64);
    check(gta_get_component_size(header, 12) == 8);
    check(gta_get_component_type(header, 13) == GTA_FLOAT128);
    check(gta_get_component_size(header, 13) == 16);
    check(gta_get_component_type(header, 14) == GTA_CFLOAT32);
    check(gta_get_component_size(header, 14) == 8);
    check(gta_get_component_type(header, 15) == GTA_CFLOAT64);
    check(gta_get_component_size(header, 15) == 16);
    check(gta_get_component_type(header, 16) == GTA_CFLOAT128);
    check(gta_get_component_size(header, 16) == 32);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 3);
    check(gta_get_dimension_size(header, 0) == 10);
    check(gta_get_dimension_size(header, 1) == 20);
    check(gta_get_dimension_size(header, 2) == 30);
    check(gta_get_data_size(header) == data_size);

    uint8_t *data2 = malloc(data_size);
    check(data2);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);

    int c = fgetc(f);
    check(c == EOF && feof(f));

    fclose(f);
    remove("test-basic.tmp");
    free(data);
    free(data2);

    gta_destroy_header(header);

    return 0;
}
