/* This file is in the public domain. */

#include <iostream>

#include <gta/gta.hpp>


int main(void)
{
    try {
        gta::header header;
        const char *name, *value;

        /* Create an example GTA */

        header.set_components(gta::uint16, gta::float32, gta::cfloat64);
        header.set_dimensions(170, 190);

        /* The global taglist contains tags that affect the whole array */

        /* Set a tag */
        header.global_taglist().set("PRODUCER", "FOO");

        /* Get a tag */
        value = header.global_taglist().get("X-BAR");
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
        header.global_taglist().unset("X-FOO");

        /* Unset all tags (clear the taglist) */
        header.global_taglist().unset_all();

        /* Access all tags in the list */
        for (uintmax_t t = 0; t < header.global_taglist().tags(); t++) {
            name = header.global_taglist().name(t);
            value = header.global_taglist().value(t);
        }

        /* The dimension taglists contain tags that affect the array dimensions */

        header.dimension_taglist(0).set("INTERPRETATION", "X");
        /* ... */
        header.dimension_taglist(1).set("INTERPRETATION", "Y");
        /* ... */

        /* The component taglists contain tags that affect the array element components */

        header.component_taglist(0).set("INTERPRETATION", "X-FOO");
        /* ... */
        header.component_taglist(1).set("UNIT", "m");
        /* ... */
        header.component_taglist(2).set("X-FOO", "BAR");
        /* ... */
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
