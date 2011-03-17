/*
 * taglists.c
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
    gta_type_t types[] = {
        GTA_INT32,
        GTA_INT32,
    };
    r = gta_set_components(header, 2, types, NULL);
    check(r == GTA_OK);
    uintmax_t dims[] = { 10, 10, 10, 10 };
    r = gta_set_dimensions(header, 4, dims);
    check(r == GTA_OK);

    /* Check data size */
    uintmax_t element_size = gta_get_element_size(header);
    check(element_size == 4 + 4);
    uintmax_t elements = gta_get_elements(header);
    check(elements == 10 * 10 * 10 * 10);
    uintmax_t data_size = gta_get_data_size(header);
    check(data_size == element_size * elements);

    /* Create the array data */
    int32_t *data = malloc(data_size);
    check(data);
    for (uintmax_t i = 0; i < 10 * 10 * 10 * 10; i++)
    {
        data[2 * i + 0] = 2 * i + 0;
        data[2 * i + 1] = 2 * i + 1;
    }

    /* Add tag lists */
    // global tag list
    gta_taglist_t *gtl = gta_get_global_taglist(header);
    check(gtl);
    check(gta_get_tags(gtl) == 0);
    check(gta_get_tag(gtl, "tag0") == NULL);
    r = gta_set_tag(gtl, "tag0", "val0");
    check(r == GTA_OK);
    check(gta_get_tags(gtl) == 1);
    check(strcmp(gta_get_tag_name(gtl, 0), "tag0") == 0);
    check(strcmp(gta_get_tag_value(gtl, 0), "val0") == 0);
    check(gta_get_tag(gtl, "tag0"));
    check(strcmp(gta_get_tag(gtl, "tag0"), "val0") == 0);
    check(gta_get_tag(gtl, "tag1") == NULL);
    r = gta_unset_tag(gtl, "tag1");
    check(r == GTA_OK);
    check(gta_get_tags(gtl) == 1);
    r = gta_unset_tag(gtl, "tag0");
    check(r == GTA_OK);
    check(gta_get_tags(gtl) == 0);
    r = gta_set_tag(gtl, "tag0", "val0");
    check(r == GTA_OK);
    r = gta_set_tag(gtl, "tag1", "val1");
    check(r == GTA_OK);
    check(gta_get_tag(gtl, "tag1"));
    check(strcmp(gta_get_tag(gtl, "tag1"), "val1") == 0);
    r = gta_set_tag(gtl, "tag1", "val-x");
    check(r == GTA_OK);
    check(gta_get_tag(gtl, "tag1"));
    check(strcmp(gta_get_tag(gtl, "tag1"), "val-x") == 0);
    r = gta_set_tag(gtl, "tag1", "val1");
    check(r == GTA_OK);
    check(gta_get_tag(gtl, "tag1"));
    check(strcmp(gta_get_tag(gtl, "tag1"), "val1") == 0);
    gta_unset_all_tags(gtl);
    check(gta_get_tags(gtl) == 0);
    char namebuf[128], valbuf[128];
    for (int i = 0; i < 1234; i++)
    {
        sprintf(namebuf, "global-tag-name-%d", i);
        sprintf(valbuf, "global-tag-value-%d", i);
        r = gta_set_tag(gtl, namebuf, valbuf);
        check(r == GTA_OK);
    }
    // component tag list
    gta_taglist_t *ctl0 = gta_get_component_taglist(header, 0);
    check(ctl0);
    gta_taglist_t *ctl1 = gta_get_component_taglist(header, 1);
    check(ctl1);
    r = gta_set_tag(ctl0, "ct0", "v0");
    check(r == GTA_OK);
    // dimensions tag lists
    gta_taglist_t *dtl0 = gta_get_dimension_taglist(header, 0);
    check(dtl0);
    gta_taglist_t *dtl1 = gta_get_dimension_taglist(header, 1);
    check(dtl1);
    gta_taglist_t *dtl2 = gta_get_dimension_taglist(header, 2);
    check(dtl2);
    gta_taglist_t *dtl3 = gta_get_dimension_taglist(header, 3);
    check(dtl3);
    r = gta_set_tag(dtl1, "dtl1t0", "v0");
    check(r == GTA_OK);
    r = gta_set_tag(dtl2, "dtl2t0", "v0");
    check(r == GTA_OK);

    /* Write the array to a file */
    f = fopen("test-taglists.tmp", "w");
    check(f);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    fclose(f);

    /* Reread the same file */
    f = fopen("test-taglists.tmp", "r");
    check(f);
    uint8_t *data2 = malloc(data_size);
    check(data2);
    r = gta_read_header_from_stream(header, f);
    check(r == GTA_OK);
    check(gta_get_components(header) == 2);
    check(gta_get_element_size(header) == element_size);
    check(gta_get_dimensions(header) == 4);
    check(gta_get_data_size(header) == data_size);
    check(gta_get_compression(header) == GTA_NONE);
    r = gta_read_data_from_stream(header, data2, f);
    check(r == GTA_OK);
    check(memcmp(data, data2, data_size) == 0);
    int c = fgetc(f);
    check(c == EOF && feof(f));
    fclose(f);
    remove("test-taglists.tmp");
    free(data);
    free(data2);

    /* Check the tags */
    gtl = gta_get_global_taglist(header);
    check(gtl);
    check(gta_get_tags(gtl) == 1234);
    for (int i = 0; i < 1234; i++)
    {
        sprintf(namebuf, "global-tag-name-%d", i);
        sprintf(valbuf, "global-tag-value-%d", i);
        check(gta_get_tag(gtl, namebuf));
        check(strcmp(gta_get_tag(gtl, namebuf), valbuf) == 0);
    }
    ctl0 = gta_get_component_taglist(header, 0);
    check(ctl0);
    check(gta_get_tags(ctl0) == 1);
    check(gta_get_tag(ctl0, "ct0"));
    check(strcmp(gta_get_tag(ctl0, "ct0"), "v0") == 0);
    ctl1 = gta_get_component_taglist(header, 1);
    check(ctl1);
    check(gta_get_tags(ctl1) == 0);
    dtl0 = gta_get_dimension_taglist(header, 0);
    check(dtl0);
    check(gta_get_tags(dtl0) == 0);
    dtl1 = gta_get_dimension_taglist(header, 1);
    check(dtl1);
    check(gta_get_tags(dtl1) == 1);
    check(gta_get_tag(dtl1, "dtl1t0"));
    check(strcmp(gta_get_tag(dtl1, "dtl1t0"), "v0") == 0);
    dtl2 = gta_get_dimension_taglist(header, 2);
    check(dtl2);
    check(gta_get_tags(dtl2) == 1);
    check(gta_get_tag(dtl2, "dtl2t0"));
    check(strcmp(gta_get_tag(dtl2, "dtl2t0"), "v0") == 0);
    dtl3 = gta_get_dimension_taglist(header, 3);
    check(dtl3);
    check(gta_get_tags(dtl3) == 0);

    /* Check that invalid tags are rejected */
    check(gta_set_tag(gtl, "", "value") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name=bla", "value") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "") == GTA_OK);
    check(gta_set_tag(gtl, "name", "üäö€") == GTA_OK);
    check(gta_set_tag(gtl, "üäö€", "value") == GTA_OK);
    check(gta_set_tag(gtl, "name", "val\x07ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\x7fue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\x80ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xc0\x80ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xc2\xc3ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xf5ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xf8ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xfcue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xfeue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xf0\xbf\xbf") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xf0\x80\x80\x80ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xf0\x80\x80\x87ue") == GTA_INVALID_DATA);
    check(gta_set_tag(gtl, "name", "val\xf0\xbf\xbf\xbfue") == GTA_OK);

    gta_destroy_header(header);

    return 0;
}
