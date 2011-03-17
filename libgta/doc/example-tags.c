/* This file is in the public domain. */

#include <gta/gta.h>


int main(void)
{
    gta_type_t components[] = { GTA_UINT16, GTA_FLOAT32, GTA_CFLOAT64 };
    uintmax_t dimensions[] = { 170, 190 };
    gta_header_t *header;
    gta_taglist_t *taglist;
    gta_result_t r;
    uintmax_t tags, t;
    const char *name, *value;

    /* Create an example GTA */

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

    /* The global taglist contains tags that affect the whole array */

    taglist = gta_get_global_taglist(header);

    /* Set a tag */
    r = gta_set_tag(taglist, "PRODUCER", "FOO");
    if (r != GTA_OK) {
        return 1;
    }
    
    /* Get a tag */
    value = gta_get_tag(taglist, "X-BAR");
    if (!value) {
        /* This tag is undefined */
    }
    else if (value[0] == '\0') {
        /* This tag is defined but empty */
    }
    else {
        /* This tag is defined and not empty */
    }

    /* Unset a tag, whether it is defined or not */
    r = gta_unset_tag(taglist, "X-FOO");
    if (r != GTA_OK) {
        return 1;
    }

    /* Unset all tags (clear the taglist) */
    gta_unset_all_tags(taglist);

    /* Access all tags in the list */
    tags = gta_get_tags(taglist);
    for (t = 0; t < tags; t++) {
        name = gta_get_tag_name(taglist, t);
        value = gta_get_tag_value(taglist, t);
    }

    /* The dimension taglists contain tags that affect the array dimensions */

    taglist = gta_get_dimension_taglist(header, 0);
    r = gta_set_tag(taglist, "INTERPRETATION", "X");
    if (r != GTA_OK) {
        return 1;
    }
    /* ... */
    taglist = gta_get_dimension_taglist(header, 1);
    r = gta_set_tag(taglist, "INTERPRETATION", "Y");
    if (r != GTA_OK) {
        return 1;
    }
    /* ... */

    /* The component taglists contain tags that affect the array element components */

    taglist = gta_get_component_taglist(header, 0);
    r = gta_set_tag(taglist, "INTERPRETATION", "X-FOO");
    if (r != GTA_OK) {
        return 1;
    }
    /* ... */
    taglist = gta_get_component_taglist(header, 1);
    r = gta_set_tag(taglist, "UNIT", "m");
    if (r != GTA_OK) {
        return 1;
    }
    /* ... */
    taglist = gta_get_component_taglist(header, 2);
    r = gta_set_tag(taglist, "X-FOO", "BAR");
    if (r != GTA_OK) {
        return 1;
    }
    /* ... */

    gta_destroy_header(header);
    return 0;
}
