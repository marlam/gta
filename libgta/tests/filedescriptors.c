/*
 * filedescriptors.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
    int fd;

    r = gta_init_header(&header);
    check(r == GTA_OK);

    /* Open the file descriptor */
    fd = open("test-filedescriptors.tmp", O_CREAT | O_RDWR, S_IRWXU);
    check(fd != -1);

    /* Define an array */
    gta_type_t types[] = {
        GTA_UINT32,
        GTA_UINT32
    };
    r = gta_set_components(header, 2, types, NULL);
    check(r == GTA_OK);
    uintmax_t dims[] = { 17, 13, 47 };
    r = gta_set_dimensions(header, 3, dims);
    check(r == GTA_OK);

    /* Check data size */
    uintmax_t element_size = gta_get_element_size(header);
    check(element_size == 4 + 4);
    uintmax_t elements = gta_get_elements(header);
    check(elements == 17 * 13 * 47);
    uintmax_t data_size = gta_get_data_size(header);
    check(data_size == element_size * elements);

    /* Create the array data */
    void *data = malloc(data_size);
    check(data);
    for (uintmax_t x = 0; x < 17; x++)
    {
        for (uintmax_t y = 0; y < 13; y++)
        {
            for (uintmax_t z = 0; z < 47; z++)
            {
                uintmax_t indices[3] = { x, y, z };
                void *element = gta_get_element(header, data, indices);
                uint32_t *c0 = gta_get_component(header, element, 0);
                uint32_t *c1 = gta_get_component(header, element, 1);
                *c0 = x * y;
                *c1 = y * z;
            }
        }
    }

    /* Write the array to the file descriptor */
    r = gta_write_header_to_fd(header, fd);
    check(r == GTA_OK);
    r = gta_write_data_to_fd(header, data, fd);
    check(r == GTA_OK);

    close(fd);

    /* Reread the same file */
    fd = open("test-filedescriptors.tmp", O_RDONLY);
    check(fd != -1);
    r = gta_read_header_from_fd(header, fd);
    check(r == GTA_OK);
    check(gta_get_components(header) == 2);
    check(gta_get_component_type(header, 0) == GTA_UINT32);
    check(gta_get_component_size(header, 0) == 4);
    check(gta_get_component_type(header, 1) == GTA_UINT32);
    check(gta_get_component_size(header, 1) == 4);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 3);
    check(gta_get_dimension_size(header, 0) == 17);
    check(gta_get_dimension_size(header, 1) == 13);
    check(gta_get_dimension_size(header, 2) == 47);
    check(gta_get_elements(header) == elements);
    check(gta_get_data_size(header) == data_size);

    uint8_t *data2 = malloc(data_size);
    check(data2);
    r = gta_read_data_from_fd(header, data2, fd);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);

    char c;
    ssize_t rr = read(fd, &c, 1);
    check(rr == 0);

    close(fd);
    remove("test-filedescriptors.tmp");
    free(data);
    free(data2);

    gta_deinit_header(header);

    return 0;
}
