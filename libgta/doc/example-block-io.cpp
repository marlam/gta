/* This file is in the public domain. */

#include <iostream>
#include <fstream>
#include <vector>

#include <gta/gta.hpp>

/* This example opens a three-dimensional GTA and then reads a small block of
 * array data.
 * This is useful for arrays that do not fit into memory. However, the input
 * file must be seekable (i.e. it must be a real file, no pipe or network
 * stream), and the GTA must not be compressed. */

int main(void)
{
    try {
        uintmax_t block_indices_low[] = { 20, 30, 40 };
        uintmax_t block_indices_high[] = { 50, 60, 70 };

        /* Read the header */
        std::ifstream stream("input.gta", std::ios::in | std::ios::binary);
        gta::header header;
        header.read_from(stream);
        /* We assume that the input GTA has three dimensions and one element
         * component of type gta::uint16, and that it is large enough to contain
         * the block defined above. */
        std::streampos data_offset = stream.tellg();
        if (data_offset == -1) {
            throw std::exception();
        }

        /* Read the data block */
        std::vector<uint16_t> block(31 * 31 * 31);
        header.read_block(stream, data_offset,
                block_indices_low, block_indices_high, &(block[0]));
        /* do something with the data */
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
