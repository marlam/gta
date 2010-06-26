/* This file is in the public domain. */

#include <stdio.h>
#include <stdlib.h>

#include <gta/gta.h>

/* This example transforms an input GTA into an output GTA.
 * It reads, manipulates and writes the array elements one at a time; the array
 * data does not have to fit into memory. */

int main(void)
{
    FILE *infile, *outfile;
    gta_header_t *inheader, *outheader;
    gta_io_state_t *instate, *outstate;
    gta_result_t r;
    void *element;
    uintmax_t i;

    /* Initialize input */

    infile = fopen("input.gta", "r");
    if (!infile) {
        return 1;
    }
    r = gta_init_header(&inheader);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_init_io_state(&instate);
    if (r != GTA_OK) {
        return 1;
    }

    /* Initialize output */

    outfile = fopen("output.gta", "w");
    if (!outfile) {
        return 1;
    }
    r = gta_init_header(&outheader);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_init_io_state(&outstate);
    if (r != GTA_OK) {
        return 1;
    }

    /* Copy the GTA header */

    r = gta_read_header_from_stream(inheader, infile);
    if (r != GTA_OK) {
        return 1;
    }
    r = gta_clone_header(outheader, inheader);
    if (r != GTA_OK) {
        return 1;
    }
    /* In this example, the output GTA is always uncompressed */
    gta_set_compression(outheader, GTA_NONE);
    r = gta_write_header_to_stream(outheader, outfile);
    if (r != GTA_OK) {
        return 1;
    }

    /* Copy the array data */

    if (gta_get_element_size(inheader) > SIZE_MAX) {
        return 1;
    }
    element = malloc(gta_get_element_size(inheader));
    if (!element) {
        return 1;
    }
    for (i = 0; i < gta_get_elements(inheader); i++) {
        r = gta_read_elements_from_stream(inheader, instate, 1, element, infile);
        if (r != GTA_OK) {
            return 1;
        }
        /* ... manipulate the element ... */
        r = gta_write_elements_to_stream(outheader, outstate, 1, element, outfile);
        if (r != GTA_OK) {
            return 1;
        }
    }

    /* Cleanup */

    free(element);
    gta_deinit_io_state(instate);
    gta_deinit_header(inheader);
    fclose(infile);
    gta_deinit_io_state(outstate);
    gta_deinit_header(outheader);
    fclose(outfile);
    return 0;
}
