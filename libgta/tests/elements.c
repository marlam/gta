/*
 * elements.c
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
    FILE *f;
    gta_header_t *h;
    gta_io_state_t *s;
    gta_result_t r;
    uintmax_t dims[] = { 7, 11, 13, 17 };
    uintmax_t index;

    r = gta_create_header(&h);
    check(r == GTA_OK);

    /* Define an array */
    gta_type_t types[] = { GTA_UINT16 };
    r = gta_set_components(h, 1, types, NULL);
    check(r == GTA_OK);
    r = gta_set_dimensions(h, 4, dims);
    check(r == GTA_OK);

    /* Write the array */
    f = fopen("test-elements.tmp", "w");
    check(f);
    r = gta_write_header_to_stream(h, f);
    check(r == GTA_OK);
    r = gta_create_io_state(&s);
    check(r == GTA_OK);
    index = 0;
    for (uintmax_t w = 0; w < gta_get_dimension_size(h, 3); w++)
    {
        for (uintmax_t z = 0; z < gta_get_dimension_size(h, 2); z++)
        {
            for (uintmax_t y = 0; y < gta_get_dimension_size(h, 1); y++)
            {
                for (uintmax_t x = 0; x < gta_get_dimension_size(h, 0); x++)
                {
                    uintmax_t indices[4] = { x, y, z, w };
                    uintmax_t test_index = gta_indices_to_linear_index(h, indices);
                    check(test_index == index);
                    uintmax_t test_indices[4] = { -1, -1, -1, -1 };
                    gta_linear_index_to_indices(h, index, test_indices);
                    check(test_indices[0] == indices[0]);
                    check(test_indices[1] == indices[1]);
                    check(test_indices[2] == indices[2]);
                    check(test_indices[3] == indices[3]);
                    uint16_t i = index;
                    r = gta_write_elements_to_stream(h, s, 1, &i, f);
                    check(r == GTA_OK);
                    index++;
                }
            }
        }
    }
    gta_destroy_io_state(s);
    fclose(f);

    /* Open the files, read and check the arrays */
    f = fopen("test-elements.tmp", "r");
    check(f);
    r = gta_read_header_from_stream(h, f);
    check(r == GTA_OK);
    r = gta_create_io_state(&s);
    check(r == GTA_OK);
    index = 0;
    for (uintmax_t w = 0; w < gta_get_dimension_size(h, 3); w++)
    {
        for (uintmax_t z = 0; z < gta_get_dimension_size(h, 2); z++)
        {
            for (uintmax_t y = 0; y < gta_get_dimension_size(h, 1); y++)
            {
                for (uintmax_t x = 0; x < gta_get_dimension_size(h, 0); x++)
                {
                    uint16_t i;
                    r = gta_read_elements_from_stream(h, s, 1, &i, f);
                    check(r == GTA_OK);
                    check(i == index);
                    index++;
                }
            }
        }
    }
    gta_destroy_io_state(s);
    fclose(f);

    gta_destroy_header(h);
    remove("test-elements.tmp");
    return 0;
}
