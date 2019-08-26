/*
 * endianness.c
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


void check_taglist_equality(const gta_taglist_t *tl_a, const gta_taglist_t *tl_b)
{
    check(gta_get_tags(tl_a) == gta_get_tags(tl_b));
    for (uintmax_t i = 0; i < gta_get_tags(tl_a); i++)
    {
        check(strcmp(gta_get_tag_name(tl_a, i), gta_get_tag_name(tl_b, i)) == 0);
        check(strcmp(gta_get_tag_value(tl_a, i), gta_get_tag_value(tl_b, i)) == 0);
    }
}

void check_header_equality(const gta_header_t *hdr_a, const gta_header_t *hdr_b)
{
    check(gta_get_components(hdr_a) == gta_get_components(hdr_b));
    check(gta_get_element_size(hdr_a) == gta_get_element_size(hdr_b));
    check(gta_get_dimensions(hdr_a) == gta_get_dimensions(hdr_b));
    check(gta_get_data_size(hdr_a) == gta_get_data_size(hdr_b));
    check_taglist_equality(gta_get_global_taglist_const(hdr_a), gta_get_global_taglist_const(hdr_b));
    for (uintmax_t i = 0; i < gta_get_components(hdr_a); i++)
    {
        check(gta_get_component_type(hdr_a, i) == gta_get_component_type(hdr_b, i));
        check(gta_get_component_size(hdr_a, i) == gta_get_component_size(hdr_b, i));
        check_taglist_equality(gta_get_component_taglist_const(hdr_a, i), gta_get_component_taglist_const(hdr_b, i));
    }
    for (uintmax_t i = 0; i < gta_get_dimensions(hdr_a); i++)
    {
        check(gta_get_dimension_size(hdr_a, i) == gta_get_dimension_size(hdr_b, i));
        check_taglist_equality(gta_get_dimension_taglist_const(hdr_a, i), gta_get_dimension_taglist_const(hdr_b, i));
    }
}

void check_data(const gta_header_t *header, void *data)
{
    for (uintmax_t x = 0; x < gta_get_dimension_size(header, 0); x++)
    {
        void *element = gta_get_element_linear(header, data, x);

        uint8_t *comp0 = gta_get_component(header, element, 0);
        check(comp0[0] == 0x00);
        check(comp0[1] == 0x50);
        check(comp0[2] == 0xA0);
        check(comp0[3] == 0xff);

        int8_t *comp1 = gta_get_component(header, element, 1);
        check(*comp1 == (int8_t)x);

        uint8_t *comp2 = gta_get_component(header, element, 2);
        check(*comp2 == (uint8_t)x);

        void *comp3 = gta_get_component(header, element, 3);
        int16_t comp3_val;
        memcpy(&comp3_val, comp3, sizeof(comp3_val));
        check(comp3_val == (int16_t)x);

        void *comp4 = gta_get_component(header, element, 4);
        uint16_t comp4_val;
        memcpy(&comp4_val, comp4, sizeof(comp4_val));
        check(comp4_val == (uint16_t)x);

        void *comp5 = gta_get_component(header, element, 5);
        int32_t comp5_val;
        memcpy(&comp5_val, comp5, sizeof(comp5_val));
        check(comp5_val == (int32_t)x);

        void *comp6 = gta_get_component(header, element, 6);
        uint32_t comp6_val;
        memcpy(&comp6_val, comp6, sizeof(comp6_val));
        check(comp6_val == (uint32_t)x);

        void *comp7 = gta_get_component(header, element, 7);
        int64_t comp7_val;
        memcpy(&comp7_val, comp7, sizeof(comp7_val));
        check(comp7_val == (int64_t)x);

        void *comp8 = gta_get_component(header, element, 8);
        uint64_t comp8_val;
        memcpy(&comp8_val, comp8, sizeof(comp8_val));
        check(comp8_val == (uint64_t)x);

        void *comp9 = gta_get_component(header, element, 9);
        int64_t comp9_val0;
        int64_t comp9_val1;
        memcpy(&comp9_val0, comp9, sizeof(comp9_val0));
        memcpy(&comp9_val1, (char *)comp9 + sizeof(int64_t), sizeof(comp9_val1));
        check(comp9_val0 == 0);
        check(comp9_val1 == 0);

        void *comp10 = gta_get_component(header, element, 10);
        uint64_t comp10_val0;
        uint64_t comp10_val1;
        memcpy(&comp10_val0, comp10, sizeof(comp10_val0));
        memcpy(&comp10_val1, (char *)comp10 + sizeof(uint64_t), sizeof(comp10_val1));
        check(comp10_val0 == 0);
        check(comp10_val1 == 0);

        void *comp11 = gta_get_component(header, element, 11);
        float comp11_val;
        memcpy(&comp11_val, comp11, sizeof(comp11_val));
        check(comp11_val <= (float)x && comp11_val >= (float)x);

        void *comp12 = gta_get_component(header, element, 12);
        double comp12_val;
        memcpy(&comp12_val, comp12, sizeof(comp12_val));
        check(comp12_val <= (double)x && comp12_val >= (double)x);

        void *comp13 = gta_get_component(header, element, 13);
        uint8_t comp13_val[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        check(memcmp(comp13, comp13_val, sizeof(long double)) == 0);

        void *comp14 = gta_get_component(header, element, 14);
        float comp14_val0;
        float comp14_val1;
        memcpy(&comp14_val0, comp14, sizeof(comp14_val0));
        memcpy(&comp14_val1, (char *)comp14 + sizeof(float), sizeof(comp14_val1));
        check(comp14_val0 <= (float)x && comp14_val0 >= (float)x);
        check(comp14_val1 <= (float)(2 * x) && comp14_val1 >= (float)(2 * x));

        void *comp15 = gta_get_component(header, element, 15);
        double comp15_val0;
        double comp15_val1;
        memcpy(&comp15_val0, comp15, sizeof(comp15_val0));
        memcpy(&comp15_val1, (char *)comp15 + sizeof(double), sizeof(comp15_val1));
        check(comp15_val0 <= (double)x && comp15_val0 >= (double)x);
        check(comp15_val1 <= (double)(2 * x) && comp15_val1 >= (double)(2 * x));

        void *comp16 = gta_get_component(header, element, 16);
        uint8_t comp16_val0[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        uint8_t comp16_val1[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        check(memcmp(comp16, comp16_val0, sizeof(long double)) == 0);
        check(memcmp((char *)comp16 + sizeof(long double), comp16_val1, sizeof(long double)) == 0);
    }
}

int main(void)
{
    gta_header_t *header, *le_header, *be_header;
    void *data, *le_data, *be_data;
    gta_result_t r;
    FILE *f;
    int c;

    /* Get environment */
    char *env_srcdir = getenv("srcdir");
    check(env_srcdir);
    char *le_test_file = malloc(strlen(env_srcdir) + strlen("/little-endian.gta") + 1);
    check(le_test_file);
    strcpy(le_test_file, env_srcdir);
    strcat(le_test_file, "/little-endian.gta");
    char *be_test_file = malloc(strlen(env_srcdir) + strlen("/big-endian.gta") + 1);
    check(be_test_file);
    strcpy(be_test_file, env_srcdir);
    strcat(be_test_file, "/big-endian.gta");

    /* Init headers */
    r = gta_create_header(&header);
    check(r == GTA_OK);
    r = gta_create_header(&le_header);
    check(r == GTA_OK);
    r = gta_create_header(&be_header);
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
    uintmax_t sizes[] = { 4 };
    r = gta_set_components(header, 17, types, sizes);
    check(r == GTA_OK);
    uintmax_t dims[] = { 100 };
    r = gta_set_dimensions(header, 1, dims);
    check(r == GTA_OK);
    r = gta_set_tag(gta_get_global_taglist(header), "abc", "123");
    check(r == GTA_OK);
    r = gta_set_tag(gta_get_global_taglist(header), "123", "abc");
    check(r == GTA_OK);
    for (uintmax_t i = 0; i < gta_get_components(header); i++)
    {
        if (i % 3 == 0)
        {
            for (uintmax_t j = 0; j < i; j++)
            {
                char nambuf[128];
                char valbuf[128];
                sprintf(nambuf, "comp%d-tag%d-name", (int)i, (int)j);
                sprintf(valbuf, "comp%d-tag%d-value", (int)i, (int)j);
                r = gta_set_tag(gta_get_component_taglist(header, i), nambuf, valbuf);
                check(r == GTA_OK);
            }
        }
    }
    for (uintmax_t j = 0; j < 3; j++)
    {
        char nambuf[128];
        char valbuf[128];
        sprintf(nambuf, "dim0-tag%d-name", (int)j);
        sprintf(valbuf, "dim0-tag%d-value", (int)j);
        r = gta_set_tag(gta_get_dimension_taglist(header, 0), nambuf, valbuf);
        check(r == GTA_OK);
    }

    /* Check data size */
    uintmax_t element_size = gta_get_element_size(header);
    check(element_size == 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16 + 4 + 8 + 16 + 8 + 16 + 32);
    uintmax_t elements = gta_get_elements(header);
    check(elements == 100);
    uintmax_t data_size = gta_get_data_size(header);
    check(data_size == element_size * elements);

    /* Create the array data */
    data = malloc(data_size);
    check(data);
    for (uintmax_t x = 0; x < gta_get_dimension_size(header, 0); x++)
    {
        uintmax_t indices[] = { x };
        void *element = gta_get_element(header, data, indices);
        check(element == (char *)data + x * gta_get_element_size(header));
        check(element = gta_get_element_linear(header, data, x));

        uint8_t *comp0 = gta_get_component(header, element, 0);
        check(comp0 == element);
        comp0[0] = 0x00;
        comp0[1] = 0x50;
        comp0[2] = 0xA0;
        comp0[3] = 0xff;

        int8_t *comp1 = gta_get_component(header, element, 1);
        check((char *)comp1 == (char *)element + 4);
        *comp1 = x;

        uint8_t *comp2 = gta_get_component(header, element, 2);
        check((char *)comp2 == (char *)element + 4 + 1);
        *comp2 = x;

        void *comp3 = gta_get_component(header, element, 3);
        check((char *)comp3 == (char *)element + 4 + 1 + 1);
        int16_t comp3_val = x;
        memcpy(comp3, &comp3_val, sizeof(comp3_val));

        void *comp4 = gta_get_component(header, element, 4);
        check((char *)comp4 == (char *)element + 4 + 1 + 1 + 2);
        uint16_t comp4_val = x;
        memcpy(comp4, &comp4_val, sizeof(comp4_val));

        void *comp5 = gta_get_component(header, element, 5);
        check((char *)comp5 == (char *)element + 4 + 1 + 1 + 2 + 2);
        int32_t comp5_val = x;
        memcpy(comp5, &comp5_val, sizeof(comp5_val));

        void *comp6 = gta_get_component(header, element, 6);
        check((char *)comp6 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4);
        uint32_t comp6_val = x;
        memcpy(comp6, &comp6_val, sizeof(comp6_val));

        void *comp7 = gta_get_component(header, element, 7);
        check((char *)comp7 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4);
        int64_t comp7_val = x;
        memcpy(comp7, &comp7_val, sizeof(comp7_val));

        void *comp8 = gta_get_component(header, element, 8);
        check((char *)comp8 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8);
        uint64_t comp8_val = x;
        memcpy(comp8, &comp8_val, sizeof(comp8_val));

        void *comp9 = gta_get_component(header, element, 9);
        check((char *)comp9 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8);
        int64_t comp9_val0 = 0;
        int64_t comp9_val1 = 0;
        memcpy(comp9, &comp9_val0, sizeof(comp9_val0));
        memcpy((char *)comp9 + sizeof(int64_t), &comp9_val1, sizeof(comp9_val1));

        void *comp10 = gta_get_component(header, element, 10);
        check((char *)comp10 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16);
        uint64_t comp10_val0 = 0;
        uint64_t comp10_val1 = 0;
        memcpy(comp10, &comp10_val0, sizeof(comp10_val0));
        memcpy((char *)comp10 + sizeof(uint64_t), &comp10_val1, sizeof(comp10_val1));

        void *comp11 = gta_get_component(header, element, 11);
        check((char *)comp11 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16);
        float comp11_val = x;
        memcpy(comp11, &comp11_val, sizeof(comp11_val));

        void *comp12 = gta_get_component(header, element, 12);
        check((char *)comp12 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16 + 4);
        double comp12_val = x;
        memcpy(comp12, &comp12_val, sizeof(comp12_val));

        void *comp13 = gta_get_component(header, element, 13);
        check((char *)comp13 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16 + 4 + 8);
        memset(comp13, 0, sizeof(long double));

        void *comp14 = gta_get_component(header, element, 14);
        check((char *)comp14 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16 + 4 + 8 + 16);
        float comp14_val0 = x;
        float comp14_val1 = 2 * x;
        memcpy(comp14, &comp14_val0, sizeof(comp14_val0));
        memcpy((char *)comp14 + sizeof(float), &comp14_val1, sizeof(comp14_val1));

        void *comp15 = gta_get_component(header, element, 15);
        check((char *)comp15 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16 + 4 + 8 + 16 + 8);
        double comp15_val0 = x;
        double comp15_val1 = 2 * x;
        memcpy(comp15, &comp15_val0, sizeof(comp15_val0));
        memcpy((char *)comp15 + sizeof(double), &comp15_val1, sizeof(comp15_val1));

        void *comp16 = gta_get_component(header, element, 16);
        check((char *)comp16 == (char *)element + 4 + 1 + 1 + 2 + 2 + 4 + 4 + 8 + 8 + 16 + 16 + 4 + 8 + 16 + 8 + 16);
        memset(comp16, 0, 2 * sizeof(long double));
    }
    check_data(header, data);

    /* Write the array to a file */
    f = fopen("test-endianness.tmp", "w");
    check(f);
    r = gta_write_header_to_stream(header, f);
    check(r == GTA_OK);
    r = gta_write_data_to_stream(header, data, f);
    check(r == GTA_OK);
    fclose(f);

    /* Read the two endianness test files.
     * They were generated with the above code on little and big endian systems
     * and their content must be identical to the content we just generated. */
    f = fopen(le_test_file, "r");
    check(f);
    r = gta_read_header_from_stream(le_header, f);
    check(r == GTA_OK);
    check_header_equality(header, le_header);
    le_data = malloc(gta_get_data_size(le_header));
    check(le_data);
    r = gta_read_data_from_stream(le_header, le_data, f);
    check(r == GTA_OK);
    c = fgetc(f);
    check(c == EOF && feof(f));
    check_data(le_header, le_data);
    check(memcmp(data, le_data, gta_get_data_size(header)) == 0);
    fclose(f);
    f = fopen(be_test_file, "r");
    check(f);
    r = gta_read_header_from_stream(be_header, f);
    check(r == GTA_OK);
    check_header_equality(header, be_header);
    be_data = malloc(gta_get_data_size(be_header));
    check(be_data);
    r = gta_read_data_from_stream(be_header, be_data, f);
    check(r == GTA_OK);
    c = fgetc(f);
    check(c == EOF && feof(f));
    check_data(be_header, be_data);
    check(memcmp(data, be_data, gta_get_data_size(header)) == 0);
    fclose(f);

    free(data);
    free(le_data);
    free(be_data);
    free(le_test_file);
    free(be_test_file);
    gta_destroy_header(header);
    gta_destroy_header(le_header);
    gta_destroy_header(be_header);

    remove("test-endianness.tmp");
    return 0;
}
