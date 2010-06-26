/* This file is in the public domain. */

#include <iostream>
#include <fstream>
#include <exception>
#include <limits>
#include <vector>

#include <gta/gta.hpp>

/* This example transforms an input GTA into an output GTA.
 * It reads, manipulates and writes the array elements one at a time; the array
 * data does not have to fit into memory. */

int main(void)
{
    try {
        /* Initialize input */
        std::ifstream instream("input.gta", std::ios::in | std::ios::binary);
        gta::header inheader;
        gta::io_state instate;

        /* Initialize output */
        std::ofstream outstream("output.gta", std::ios::out | std::ios::binary);
        gta::header outheader;
        gta::io_state outstate;

        /* Copy the GTA header */
        inheader.read_from(instream);
        outheader = inheader;
        /* In this example, the output GTA is always uncompressed */
        outheader.set_compression(gta::none);
        outheader.write_to(outstream);

        /* Copy the array data */
        if (inheader.element_size() > std::numeric_limits<size_t>::max()) {
            throw std::exception();
        }
        std::vector<char> element(inheader.element_size());
        for (uintmax_t i = 0; i < inheader.elements(); i++) {
            inheader.read_elements(instate, instream, 1, &(element[0]));
            /* ... manipulate the element ... */
            outheader.write_elements(outstate, outstream, 1, &(element[0]));
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
