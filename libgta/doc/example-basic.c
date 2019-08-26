/* This file is in the public domain. */

#include <stdio.h>
#include <stdlib.h>

#include <gta/gta.h>


int main(void)
{
    gta_type_t components[] = { GTA_UINT8, GTA_UINT8, GTA_UINT8 };
    uintmax_t dimensions[] = { 256, 128 };
    gta_header_t *header;
    gta_result_t r;
    void *data;
    uintmax_t x, y;
    FILE *f;

    /* Create a GTA that contains an RGB image with 256x128 pixels */

    r = gta_create_header(&header);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_set_components(header, 3, components, NULL);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_set_dimensions(header, 2, dimensions);
    if (r != GTA_OK) {
        return 1;
    }

    data = malloc(gta_get_data_size(header));
    if (!data) {
        return 1;
    }
    for (y = 0; y < 128; y++) {
        for (x = 0; x < 256; x++) {
            uintmax_t indices[] = { x, y };
            unsigned char *pixel = gta_get_element(header, data, indices);
            pixel[0] = x;
            pixel[1] = 2 * y;
            pixel[2] = 128;
        }
    }

    /* Set some tags (this is entirely optional) */

    r = gta_set_tag(gta_get_component_taglist(header, 0), "INTERPRETATION", "RED");
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_set_tag(gta_get_component_taglist(header, 1), "INTERPRETATION", "GREEN");
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_set_tag(gta_get_component_taglist(header, 2), "INTERPRETATION", "BLUE");
    if (r != GTA_OK) {
        return 1;
    }

    /* Write the GTA to a file */

    f = fopen("rgb.gta", "wb");
    if (!f) {
        return 1;
    }
    r = gta_write_header_to_stream(header, f);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_write_data_to_stream(header, data, f);
    if (r != GTA_OK) {
        return 1;
    }
    if (fclose(f) != 0) {
        return 1;
    }

    free(data);
    gta_destroy_header(header);

    /* Reread the same file */

    f = fopen("rgb.gta", "rb");
    if (!f) {
        return 1;
    }
    r = gta_create_header(&header);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_read_header_from_stream(header, f);
    if (r != GTA_OK) {
        return 1;
    }

    if (gta_get_components(header) != 3
            || gta_get_component_type(header, 0) != GTA_UINT8
            || gta_get_component_type(header, 1) != GTA_UINT8
            || gta_get_component_type(header, 2) != GTA_UINT8) {
        return 1;
    }
    if (gta_get_dimensions(header) != 2
            || gta_get_dimension_size(header, 0) != 256
            || gta_get_dimension_size(header, 1) != 128) {
        return 1;
    }

    data = malloc(gta_get_data_size(header));
    if (!data) {
        return 1;
    }
    r = gta_read_data_from_stream(header, data, f);
    if (r != GTA_OK) {
        return 1;
    }
    if (fclose(f) != 0) {
        return 1;
    }

    free(data);
    gta_destroy_header(header);
    return 0;
}
