/* This file is in the public domain. */

#include <stdio.h>
#include <stdlib.h>

#include <gta/gta.h>


int main(void)
{
    FILE *f;
    gta_header_t *hdr;
    void *data;
    gta_result_t r;
    uintmax_t x, y;

    /* Create a GTA that contains an RGB image with 256x128 pixels */

    r = gta_init_header(&hdr);
    if (r != GTA_OK) {
        return 1;
    }
    gta_type_t components[] = { GTA_UINT8, GTA_UINT8, GTA_UINT8 };
    r = gta_set_components(hdr, 3, components, NULL);
    if (r != GTA_OK) {
        return 1;
    }
    uintmax_t size[] = { 256, 128 };
    r = gta_set_dimensions(hdr, 2, size);
    if (r != GTA_OK) {
        return 1;
    }

    data = malloc(gta_get_data_size(hdr));
    if (!data) {
        return 1;
    }
    for (y = 0; y < 128; y++) {
        for (x = 0; x < 256; x++) {
            uintmax_t indices[] = { x, y };
            unsigned char *pixel = gta_get_element(hdr, data, indices);
            pixel[0] = x;
            pixel[1] = 2 * y;
            pixel[2] = 128;
        }
    }

    /* Set some tags (this is entirely optional) */

    r = gta_set_tag(gta_get_component_taglist(hdr, 0), "INTERPRETATION", "RED");
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_set_tag(gta_get_component_taglist(hdr, 1), "INTERPRETATION", "GREEN");
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_set_tag(gta_get_component_taglist(hdr, 2), "INTERPRETATION", "BLUE");
    if (r != GTA_OK) {
        return 1;
    }

    /* Write the GTA to a file */

    f = fopen("rgb.gta", "wb");
    if (!f) {
        return 1;
    }
    gta_set_compression(hdr, GTA_BZIP2);
    r = gta_write_header_to_stream(hdr, f);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_write_data_to_stream(hdr, data, f);
    if (r != GTA_OK) {
        return 1;
    }
    if (fclose(f) != 0) {
        return 1;
    }

    free(data);
    gta_deinit_header(hdr);

    /* Reread the same file */

    f = fopen("rgb.gta", "rb");
    if (!f) {
        return 1;
    }
    r = gta_init_header(&hdr);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_read_header_from_stream(hdr, f);
    if (r != GTA_OK) {
        return 1;
    }

    if (gta_get_components(hdr) != 3
            || gta_get_component_type(hdr, 0) != GTA_UINT8
            || gta_get_component_type(hdr, 1) != GTA_UINT8
            || gta_get_component_type(hdr, 2) != GTA_UINT8) {
        return 1;
    }
    if (gta_get_dimensions(hdr) != 2
            || gta_get_dimension_size(hdr, 0) != 256
            || gta_get_dimension_size(hdr, 1) != 128) {
        return 1;
    }

    data = malloc(gta_get_data_size(hdr));
    if (!data) {
        return 1;
    }
    r = gta_read_data_from_stream(hdr, data, f);
    if (r != GTA_OK) {
        return 1;
    }
    if (fclose(f) != 0) {
        return 1;
    }

    free(data);
    gta_deinit_header(hdr);
    return 0;
}
