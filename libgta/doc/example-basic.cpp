/* This file is in the public domain. */

#include <iostream>
#include <fstream>
#include <exception>
#include <vector>

#include <gta/gta.hpp>


int main(void)
{
    try {
        gta::header header;

        /* Create a GTA that contains an RGB image with 256x128 pixels */

        header.set_components(gta::uint8, gta::uint8, gta::uint8);
        header.set_dimensions(256, 128);
        std::vector<unsigned char> data(header.data_size());
        for (uintmax_t y = 0; y < 128; y++) {
            for (uintmax_t x = 0; x < 256; x++) {
                unsigned char *pixel = static_cast<unsigned char *>(
                        header.element(&(data[0]), x, y));
                pixel[0] = x;
                pixel[1] = 2 * y;
                pixel[2] = 128;
            }
        }

        /* Set some tags (this is entirely optional) */
        header.component_taglist(0).set("INTERPRETATION", "RED");
        header.component_taglist(1).set("INTERPRETATION", "GREEN");
        header.component_taglist(2).set("INTERPRETATION", "BLUE");

        /* Write the GTA to a file */
        std::ofstream ofs("rgb.gta", std::ios::out | std::ios::binary);
        header.set_compression(gta::bzip2);
        header.write_to(ofs);
        header.write_data(ofs, &(data[0]));
        ofs.close();

        /* Reread the same file */
        std::ifstream ifs("rgb.gta", std::ios::in | std::ios::binary);
        header.read_from(ifs);
        if (header.components() != 3
                || header.component_type(0) != gta::uint8
                || header.component_type(1) != gta::uint8
                || header.component_type(2) != gta::uint8) {
            throw std::exception();
        }
        if (header.dimensions() != 2
                || header.dimension_size(0) != 256
                || header.dimension_size(1) != 128) {
            throw std::exception();
        }
        header.read_data(ifs, &(data[0]));
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
