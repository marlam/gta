/* This file is in the public domain. */

#include <iostream>
#include <fstream>
#include <exception>

#include <gta/gta.hpp>


int main(void)
{
    unsigned char *data = NULL;

    try {
        gta::header hdr;

        /* Create a GTA that contains an RGB image with 256x128 pixels */

        hdr.set_components(gta::uint8, gta::uint8, gta::uint8);
        hdr.set_dimensions(256, 128);
        data = new unsigned char[hdr.data_size()];
        for (uintmax_t y = 0; y < 128; y++) {
            for (uintmax_t x = 0; x < 256; x++) {
                unsigned char *pixel =
                    static_cast<unsigned char *>(hdr.element(data, x, y));
                pixel[0] = x;
                pixel[1] = 2 * y;
                pixel[2] = 128;
            }
        }

        /* Set some tags (this is entirely optional) */
        hdr.component_taglist(0).set("INTERPRETATION", "RED");
        hdr.component_taglist(1).set("INTERPRETATION", "GREEN");
        hdr.component_taglist(2).set("INTERPRETATION", "BLUE");

        /* Write the GTA to a file */
        std::ofstream ofs("rgb.gta", std::ios::out | std::ios::binary);
        hdr.set_compression(gta::bzip2);
        hdr.write_to(ofs);
        hdr.write_data(ofs, data);
        ofs.close();
        delete[] data;
        data = NULL;

        /* Reread the same file */
        std::ifstream ifs("rgb.gta", std::ios::in | std::ios::binary);
        hdr.read_from(ifs);
        if (hdr.components() != 3
                || hdr.component_type(0) != gta::uint8
                || hdr.component_type(1) != gta::uint8
                || hdr.component_type(2) != gta::uint8) {
            throw std::exception();
        }
        if (hdr.dimensions() != 2
                || hdr.dimension_size(0) != 256
                || hdr.dimension_size(1) != 128) {
            throw std::exception();
        }
        data = new unsigned char[hdr.data_size()];
        hdr.read_data(ifs, data);
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    delete[] data;
    return 0;
}
