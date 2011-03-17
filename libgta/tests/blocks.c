/*
 * blocks.c
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

    /* Define an array */
    gta_type_t types[] = { GTA_UINT16 };
    r = gta_set_components(header, 1, types, NULL);
    check(r == GTA_OK);
    uintmax_t dims[] = { 10, 10, 10 };
    r = gta_set_dimensions(header, 3, dims);
    check(r == GTA_OK);

    /* Create the array data */
    void *data = malloc(gta_get_data_size(header));
    check(data);
    for (uintmax_t z = 0; z < 10; z++)
    {
        for (uintmax_t y = 0; y < 10; y++)
        {
            for (uintmax_t x = 0; x < 10; x++)
            {
                uintmax_t indices[3] = { x, y, z };
                void *element = gta_get_element(header, data, indices);
                uint16_t i = z * (10 * 10) + y * 10 + x;
                void *element2 = gta_get_element_linear(header, data, i);
                check(element == element2);
                memcpy(element, &i, sizeof(uint16_t));
            }
        }
    }

    /* Write the array */
    f = fopen("test-blocks.tmp", "w");
    check(f);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    fclose(f);

    /* Define a block */
    uintmax_t lc[] = { 2, 3, 4 };
    uintmax_t hc[] = { 5, 6, 7 };
    void *block = malloc(4 * 4 * 4 * sizeof(uint16_t));
    check(block);

    /* Open the file, read the block */
    f = fopen("test-blocks.tmp", "r+");
    check(f);
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    off_t data_offset = ftello(f);
    check(data_offset != -1);
    r = gta_read_block_from_stream(header, data_offset, lc, hc, block, f);
    check(r == GTA_OK);
    for (uintmax_t z = 0; z < hc[2] - lc[2] + 1; z++)
    {
        for (uintmax_t y = 0; y < hc[1] - lc[1] + 1; y++)
        {
            for (uintmax_t x = 0; x < hc[0] - lc[0] + 1; x++)
            {
                uintmax_t index = z * (hc[1] - lc[1] + 1) * (hc[0] - lc[0] + 1) + y * (hc[0] - lc[0] + 1) + x;
                void *element = (char *)block + index * sizeof(uint16_t);
                uint16_t i = (z + lc[2]) * (10 * 10) + (y + lc[1]) * 10 + x + lc[0];
                uint16_t v;
                memcpy(&v, element,  sizeof(uint16_t));
                check(v == i);
            }
        }
    }

    /* Modify the block, write it */
    memset(block, 0, (hc[2] - lc[2] + 1) * (hc[1] - lc[1] + 1) * (hc[0] - lc[0] + 1) * sizeof(uint16_t));
    r = gta_write_block_to_stream(header, data_offset, lc, hc, block, f);
    check(r == GTA_OK);
    fclose(f);

    /* Reread the whole file, check the data */
    f = fopen("test-blocks.tmp", "r");
    check(f);
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    r = gta_read_data_from_stream(header, data, f);
    check(r == GTA_OK);
    fclose(f);
    for (uintmax_t z = 0; z < 10; z++)
    {
        for (uintmax_t y = 0; y < 10; y++)
        {
            for (uintmax_t x = 0; x < 10; x++)
            {
                uintmax_t indices[3] = { x, y, z };
                void *element = gta_get_element(header, data, indices);
                uint16_t i = z * (10 * 10) + y * 10 + x;
                uint16_t v;
                memcpy(&v, element, sizeof(uint16_t));
                if (x >= lc[0] && x <= hc[0] && y >= lc[1] && y <= hc[1] && z >= lc[2] && z <= hc[2])
                {
                    check(v == 0);
                }
                else
                {
                    check(v == i);
                }
            }
        }
    }

    free(data);
    free(block);
    gta_destroy_header(header);
    remove("test-blocks.tmp");
    return 0;
}
