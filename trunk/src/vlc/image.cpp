/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "image.h"

namespace vlc {

image::image()
    : width(0), height(0)
{
}

image::image(unsigned width, unsigned height)
    : width(width), height(height)
{
    pixels.resize((width * height) + (align / sizeof(uint32_t)));
}

image::image(image &&from)
    : width(from.width), height(from.height),
      pixels(std::move(from.pixels))
{
    from.width = 0;
    from.height = 0;
}

image::~image()
{
}

image & image::operator=(image &&from)
{
    width = from.width;
    height = from.height;
    pixels = std::move(from.pixels);
    from.width = 0;
    from.height = 0;

    return *this;
}

const uint32_t * image::scan_line(unsigned y) const
{
    if (y < height)
    {
        auto p = (const uint32_t *)((uintptr_t(&pixels[0]) + (align - 1)) & ~uintptr_t(align - 1));
        return &p[y * width];
    }

    return nullptr;
}

uint32_t * image::scan_line(unsigned y)
{
    if (y < height)
    {
        auto p = (uint32_t *)((uintptr_t(&pixels[0]) + (align - 1)) & ~uintptr_t(align - 1));
        return &p[y * width];
    }

    return nullptr;
}

static inline uint32_t swap_rb_pixel(uint32_t px)
{
    return (px & 0xFF00FF00) | ((px & 0x00FF0000) >> 16) | ((px & 0x000000FF) << 16);
}

void image::swap_rb()
{
    for (unsigned y = 0; y < height; y++)
    {
        auto line = scan_line(y);
        for (unsigned x = 0; x < width; x++)
            line[x] = swap_rb_pixel(line[x]);
    }
}

} // End of namespace


#include <jpge/jpge.h>

bool vlc::image::save_jpeg(std::ostream &out) const
{
    return jpge::compress_image_to_jpeg_file_in_memory(
                out,
                width, height,
                4,
                reinterpret_cast<const jpge::uint8 *>(scan_line(0)));
}


#if defined(__unix__)
# include <endian.h>
#else
#define __bswap_16(x) \
     ((((x) >> 8) & 0x00FFu) | (((x) & 0x00FFu) << 8))

# define htobe16(x) __bswap_16 (x)
# define htole16(x) (x)
# define be16toh(x) __bswap_16 (x)
# define le16toh(x) (x)

#define __bswap_32(x) \
     ((((x) & 0xFF000000u) >> 24) | (((x) & 0x00FF0000u) >>  8) | \
      (((x) & 0x0000FF00u) <<  8) | (((x) & 0x000000FFu) << 24))

# define htobe32(x) __bswap_32 (x)
# define htole32(x) (x)
# define be32toh(x) __bswap_32 (x)
# define le32toh(x) (x)
#endif

extern "C" int mz_compress(unsigned char *pDest, unsigned long *pDest_len, const unsigned char *pSource, unsigned long source_len);

static uint32_t crc_table[256];
static struct _Crc
{
    _Crc()
    {
        for (int n = 0; n < 256; n++)
        {
            uint32_t c = uint32_t(n);
            for (int k = 0; k < 8; k++)
                if (c & 1) c = 0xEDB88320 ^ (c >> 1); else c = c >> 1;

            crc_table[n] = c;
        }
    }
} crc_init;

static uint32_t calc_crc(const void *buf, size_t len, uint32_t crc = 0xFFFFFFFF)
{
    for (size_t n = 0; n < len; n++)
        crc = crc_table[(crc ^ reinterpret_cast<const uint8_t *>(buf)[n]) & 0xff] ^ (crc >> 8);

    return crc;
}

bool vlc::image::save_png(std::ostream &out) const
{
    {
        static const uint8_t png_header[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
        out.write(reinterpret_cast<const char *>(png_header), sizeof(png_header));
    }
    {
        struct  __attribute__ ((__packed__)) { uint32_t width, height; uint8_t bit_depth, color_type, compress, filter, interlace; } ihdr;
        ihdr.width = htobe32(width);
        ihdr.height = htobe32(height);
        ihdr.bit_depth = 8;
        ihdr.color_type = 6;
        ihdr.compress = 0;
        ihdr.filter = 0;
        ihdr.interlace = 0;

        const uint32_t length = htobe32(sizeof(ihdr));
        out.write(reinterpret_cast<const char *>(&length), sizeof(length));
        static const char type[4] = { 'I', 'H', 'D', 'R' };
        out.write(type, sizeof(type));
        out.write(reinterpret_cast<const char *>(&ihdr), sizeof(ihdr));
        const uint32_t crc = htobe32(calc_crc(&ihdr, sizeof(ihdr), calc_crc(type, sizeof(type))) ^ 0xFFFFFFFF);
        out.write(reinterpret_cast<const char *>(&crc), sizeof(crc));
    }
    {
        const uint32_t total_size = height * ((width * sizeof(uint32_t)) + 1);
        const uint32_t length = htobe32(total_size);
        out.write(reinterpret_cast<const char *>(&length), sizeof(length));
        static const char type[4] = { 'I', 'D', 'A', 'T' };
        out.write(type, sizeof(type));

        std::vector<uint8_t> data;
        data.resize(total_size);
        for (uint32_t y = 0; y < height; y++)
        {
            const auto * const src_line = scan_line(y);
            uint8_t * const dst_line = &(data[y * ((width * sizeof(uint32_t)) + 1)]);

            dst_line[0] = 0; // no filter
            for (uint32_t x = 0; x < width; x++)
                reinterpret_cast<uint32_t *>(dst_line + 1)[x] = src_line[x];
        }

        std::vector<uint8_t> compressed;
        compressed.resize(total_size + (total_size / 2));

        unsigned long dst_len = compressed.size();
        if (mz_compress(&(compressed[0]), &dst_len, &(data[0]), data.size()) != 0)
            return false;

        out.write(reinterpret_cast<const char *>(&(compressed[0])), dst_len);
        const uint32_t crc = htobe32(calc_crc(&(compressed[0]), dst_len, calc_crc(type, sizeof(type))) ^ 0xFFFFFFFF);
        out.write(reinterpret_cast<const char *>(&crc), sizeof(crc));
    }
    {
        const uint32_t length = htobe32(0);
        out.write(reinterpret_cast<const char *>(&length), sizeof(length));
        static const char type[4] = { 'I', 'E', 'N', 'D' };
        out.write(type, sizeof(type));
        const uint32_t crc = htobe32(calc_crc(type, sizeof(type)) ^ 0xFFFFFFFF);
        out.write(reinterpret_cast<const char *>(&crc), sizeof(crc));
    }

    return true;
}
