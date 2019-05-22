#include <Magick++.h>
#include <gmpxx.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;
using namespace Magick;

// Tuning parameters.  These control the quality and size of the compression.  Increase these to
// enhance image quality at the expense of a larger output string.  These numbers should produce 140
// character strings with the full assigned Unicode set described below.
static const int steps_in_x = 23;
static const int steps_in_y = 23;
static const int steps_in_red = 4;
static const int steps_in_green = 4;
static const int steps_in_blue = 4;
static const int blocks_in_x = 11;
static const int blocks_in_y = 11;
static const int maximum_width = 1000;
static const int maximum_height = 1000;

// Other tuning parameters that don't affect the string size.  The first two are weights for color
// blending.  Higher in b makes things more "fractally" looking.  Higher in a makes things more
// blocky.  The iterations is just how many steps to go through when decoding before stopping.  The
// output image usually converges well before 15 iterations.
static const int color_weight_a = 1;
static const int color_weight_b = 2;
static const int iterations = 15;

// Images will be broken into a fixed set of blocks, with a small amount of data for each block.  We
// also need to store the image size.  Filling this in is really the core function of this program.
struct block
{
    int orientation;
    int x, y;
    int red, green, blue;
    long long int error;
} blocks[ blocks_in_y ][ blocks_in_x ];
int image_width, image_height;

// Full assigned Unicode, excluding <, >, &, control, combining, surrogate and private characters.
// The data for this run-length encoded into pairs of numbers in the table.  The first number is how
// many invalid character codes to skip over.  The second number is the how many of the next
// character codes are valid for use.  The data terminates with two empty runs.
static const int number_assigned = 100385;
static const int codes[] =
{

    32, 6, 1, 21, 1, 1, 1, 705, 112, 8, 2, 5, 5, 7, 1, 1, 1, 20, 1, 224, 7, 154, 13, 38, 2, 7, 1, 39, 1, 2, 6, 55, 8, 27, 5,
    5, 11, 4, 2, 22, 2, 2, 1, 62, 1, 174, 1, 60, 2, 101, 14, 43, 9, 7, 262, 57, 2, 18, 2, 5, 3, 27, 8, 5, 1, 3, 1, 8, 2,
    2, 2, 22, 1, 7, 1, 1, 3, 4, 2, 9, 2, 2, 2, 4, 8, 1, 4, 2, 1, 5, 2, 21, 6, 3, 1, 6, 4, 2, 2, 22, 1, 7, 1, 2, 1, 2, 1,
    2, 2, 1, 1, 5, 4, 2, 2, 3, 3, 1, 7, 4, 1, 1, 7, 16, 11, 3, 1, 9, 1, 3, 1, 22, 1, 7, 1, 2, 1, 5, 2, 10, 1, 3, 1, 3,
    2, 1, 15, 4, 2, 10, 1, 1, 15, 3, 1, 8, 2, 2, 2, 22, 1, 7, 1, 2, 1, 5, 2, 9, 2, 2, 2, 3, 8, 2, 4, 2, 1, 5, 2, 12, 16,
    2, 1, 6, 3, 3, 1, 4, 3, 2, 1, 1, 1, 2, 3, 2, 3, 3, 3, 12, 4, 5, 3, 3, 1, 4, 2, 1, 6, 1, 14, 21, 6, 3, 1, 8, 1, 3, 1,
    23, 1, 10, 1, 5, 3, 8, 1, 3, 1, 4, 7, 2, 1, 2, 6, 4, 2, 10, 8, 8, 2, 2, 1, 8, 1, 3, 1, 23, 1, 10, 1, 5, 2, 9, 1, 3,
    1, 4, 7, 2, 7, 1, 1, 4, 2, 10, 1, 2, 15, 2, 1, 8, 1, 3, 1, 23, 1, 16, 3, 8, 1, 3, 1, 4, 9, 1, 8, 4, 2, 16, 3, 7, 2,
    2, 1, 18, 3, 24, 1, 9, 1, 1, 2, 7, 3, 1, 4, 6, 1, 1, 1, 8, 18, 3, 12, 58, 4, 29, 37, 2, 1, 1, 2, 2, 1, 1, 2, 1, 6,
    4, 1, 7, 1, 3, 1, 1, 1, 1, 2, 2, 1, 13, 1, 3, 2, 5, 1, 1, 1, 6, 2, 10, 2, 2, 34, 72, 1, 36, 4, 27, 4, 8, 1, 36, 1,
    15, 1, 7, 43, 154, 4, 40, 10, 45, 3, 90, 5, 68, 5, 82, 6, 73, 1, 4, 2, 7, 1, 1, 1, 4, 2, 41, 1, 4, 2, 33, 1, 4, 2,
    7, 1, 1, 1, 4, 2, 15, 1, 57, 1, 4, 2, 67, 5, 29, 3, 26, 6, 85, 12, 630, 9, 29, 3, 81, 15, 13, 1, 7, 11, 23, 9, 20,
    12, 13, 1, 3, 1, 2, 12, 94, 2, 10, 6, 10, 6, 15, 1, 10, 6, 88, 8, 43, 85, 29, 3, 12, 4, 12, 4, 1, 3, 42, 2, 5, 11,
    42, 6, 26, 6, 10, 4, 62, 2, 2, 224, 76, 4, 27, 9, 9, 3, 43, 3, 12, 70, 56, 3, 15, 3, 51, 128, 192, 64, 278, 2, 6, 2,
    38, 2, 6, 2, 8, 1, 1, 1, 1, 1, 1, 1, 31, 2, 53, 1, 15, 1, 14, 2, 6, 1, 19, 2, 3, 1, 9, 1, 101, 5, 8, 2, 27, 1, 5,
    11, 22, 74, 80, 3, 54, 7, 600, 24, 39, 25, 11, 21, 574, 2, 29, 3, 4, 61, 4, 1, 4, 2, 28, 1, 35, 1, 1, 1, 4, 3, 1, 1,
    7, 2, 52, 3, 24, 1, 14, 1, 11, 1, 1, 3, 893, 3, 5, 171, 47, 1, 47, 1, 16, 1, 13, 2, 107, 14, 45, 10, 54, 9, 1, 16,
    23, 9, 7, 1, 7, 1, 7, 1, 7, 1, 7, 1, 7, 1, 7, 1, 7, 33, 49, 79, 26, 1, 89, 12, 214, 26, 12, 4, 64, 1, 86, 4, 101, 5,
    41, 3, 94, 1, 40, 8, 36, 12, 47, 1, 36, 12, 175, 1, 6838, 10, 20996, 60, 1165, 3, 55, 57, 300, 20, 32, 2, 13, 4, 1,
    10, 26, 104, 141, 110, 49, 20, 56, 8, 69, 9, 12, 38, 84, 11, 1, 160, 55, 9, 14, 2, 10, 2, 4, 416, 11172, 8540, 302,
    2, 59, 5, 106, 38, 7, 12, 5, 5, 26, 1, 5, 1, 1, 1, 2, 1, 2, 1, 108, 33, 365, 16, 64, 2, 54, 40, 14, 2, 26, 22, 35,
    1, 19, 1, 4, 4, 5, 1, 135, 2, 1, 1, 190, 3, 6, 2, 6, 2, 6, 2, 3, 3, 7, 1, 7, 10, 5, 2, 12, 1, 26, 1, 19, 1, 2, 1,
    15, 2, 14, 34, 123, 5, 3, 4, 45, 3, 84, 5, 12, 52, 45, 131, 29, 3, 49, 47, 31, 1, 4, 12, 27, 53, 30, 1, 37, 4, 14,
    42, 158, 2, 10, 854, 6, 2, 1, 1, 44, 1, 2, 3, 1, 2, 1, 192, 26, 5, 27, 5, 1, 192, 4, 1, 2, 5, 8, 1, 3, 1, 27, 4, 3,
    4, 9, 8, 9, 5543, 879, 145, 99, 13, 4, 43916, 246, 10, 39, 2, 60, 5, 3, 6, 8, 8, 2, 7, 30, 4, 48, 34, 66, 3, 1, 186,
    87, 9, 18, 142, 85, 1, 71, 1, 2, 2, 1, 2, 2, 2, 4, 1, 12, 1, 1, 1, 7, 1, 65, 1, 4, 2, 8, 1, 7, 1, 28, 1, 4, 1, 5, 1,
    1, 3, 7, 1, 340, 2, 292, 2, 50, 6144, 44, 4, 100, 3948, 42711, 20777, 542, 722403, 1, 30, 96, 128, 240, 0, 0

};

// Printable 7-bit ASCII, excluding <, and >, and &
// static const int number_assigned = 92;
// static const int codes[] =
// {
//     32, 6, 1, 21, 1, 1, 1, 64, 0, 0
// };

// CJK Unified
// static const int number_assigned = 20944;
// static const int codes[] =
// {
//     19968, 20944, 0, 0
// };

void compress(
    char const *filename )
{
    // Initialize the blocks' error to a ridiculously high number.
    for ( int y = 0; y < blocks_in_y; ++y )
        for ( int x = 0; x < blocks_in_x; ++x )
            blocks[ y ][ x ].error = 0x7fffffffffffffffLL;

    // Grab the source (range) image.
    Image range( filename );
    range.type( TrueColorType );
    range.matte( true );
    range.backgroundColor( Color( 0, 0, 0, QuantumRange ) );
    Geometry range_size = range.size();
    Pixels range_view( range );

    // Save the image size data for encoding later.
    image_width = range_size.width();
    image_height = range_size.height();

    // Try 16 different orientations for the downsampled image: four different 45 degree angles,
    // either flipped horizontally or not, and flipped vertically or not.  These are the domains.
    // We'll search these for matches to the blocks in the range.  (The 45 degree angle versions are
    // unorthodox, but help a *lot* for this image size and are only two more bits per block.)
    for ( int orientation = 0; orientation < 16; ++orientation )
    {
        Image domain( range );
        domain.zoom( "50%" );
        if ( orientation & 1 )
            domain.flip();
        if ( orientation & 2 )
            domain.flop();
        domain.rotate( orientation / 4 * 45 );
        Geometry domain_size = domain.size();
        Pixels domain_view( domain );

        // For each of the range blocks, we'll look for good matches in the current domain image.
        for ( int range_y = 0; range_y < blocks_in_y; ++range_y )
        {
            cout << ( orientation * blocks_in_y + range_y ) << "/" << ( 16 * blocks_in_y ) << "\r" << flush;
            int range_top = range_size.height() * range_y / blocks_in_y;
            int block_height = range_size.height() * ( range_y + 1) / blocks_in_y - range_top;
            for ( int range_x = 0; range_x < blocks_in_x; ++range_x )
            {
                int range_left = range_size.width() * range_x / blocks_in_x;
                int block_width = range_size.width() * ( range_x + 1) / blocks_in_x - range_left;
                int area = block_width * block_height;

                // Make note of the average color in this range block.
                const PixelPacket *range_pixels = range_view.getConst( range_left, range_top, block_width, block_height );
                int range_red = 0;
                int range_green = 0;
                int range_blue = 0;
                for ( int index = 0; index < area; ++index, ++range_pixels )
                {
                    range_red += range_pixels->red;
                    range_green += range_pixels->green;
                    range_blue += range_pixels->blue;
                }

                // Now we'll search for pieces of the domain that look like good matches for the
                // current range block.
                for ( int domain_y = 0; domain_y < steps_in_y; ++domain_y )
                {
                    int domain_top = ( domain_size.height() - block_height ) * domain_y / steps_in_y;
                    for ( int domain_x = 0; domain_x < steps_in_x; ++domain_x )
                    {
                        int domain_left = ( domain_size.width() - block_width ) * domain_x / steps_in_x;

                        // Compute the average color in this domain block.  If we have transparent
                        // pixels, we've hit the transparent corners of the image generated by
                        // rotations at 45 degree angles.  We'll reject these since they produce
                        // weird solid colors in the output.
                        const PixelPacket *domain_pixels = domain_view.getConst( domain_left, domain_top, block_width, block_height );
                        int domain_red = 0;
                        int domain_green = 0;
                        int domain_blue = 0;
                        bool corner = false;
                        for ( int index = 0; index < area; ++index, ++domain_pixels )
                        {
                            if ( domain_pixels->opacity > QuantumRange / 2 )
                            {
                                corner = true;
                                break;
                            }
                            domain_red += domain_pixels->red;
                            domain_green += domain_pixels->green;
                            domain_blue += domain_pixels->blue;
                        }
                        if ( corner )
                            continue;

                        // Storing a contrast and brightness adjustment for each each color
                        // component takes too much space.  Instead, we find a constant color, that
                        // when blended with the domain block comes as close as possible to the
                        // range block's color.
                        int red_shift = ( ( color_weight_a + color_weight_b ) * range_red - color_weight_b * domain_red ) / ( area * color_weight_a );
                        int green_shift = ( ( color_weight_a + color_weight_b ) * range_green - color_weight_b * domain_green ) / ( area * color_weight_a );
                        int blue_shift = ( ( color_weight_a + color_weight_b ) * range_blue - color_weight_b * domain_blue ) / ( area * color_weight_a );

                        // Then we heavily quantize that color down to a few steps.  It's an
                        // extremely limited palette but when blended with the domain blocks the
                        // color fidelity can be surprising good.
                        int red_bits = ( red_shift * ( steps_in_red - 1 ) + QuantumRange / 2 ) / QuantumRange;
                        int green_bits = ( green_shift * ( steps_in_green - 1 ) + QuantumRange / 2 ) / QuantumRange;
                        int blue_bits = ( blue_shift * ( steps_in_blue - 1 ) + QuantumRange / 2 ) / QuantumRange;
                        red_bits = min( steps_in_red - 1, max( 0, red_bits ) );
                        green_bits = min( steps_in_green - 1, max( 0, green_bits ) );
                        blue_bits = min( steps_in_blue - 1, max( 0, blue_bits ) );

                        // Now, compute the total RMS error between all of the pixels in the range
                        // block and the color-blended domain block.
                        int quantized_red = red_bits * QuantumRange / ( steps_in_red - 1 );
                        int quantized_green = green_bits * QuantumRange / ( steps_in_green - 1 );
                        int quantized_blue = blue_bits * QuantumRange / ( steps_in_blue - 1 );
                        range_pixels = range_view.getConst( range_left, range_top, block_width, block_height );
                        domain_pixels = domain_view.getConst( domain_left, domain_top, block_width, block_height );
                        long long int error = 0;
                        for ( int index = 0; index < area; ++index, ++range_pixels, ++domain_pixels )
                        {
                            long long int red_error = range_pixels->red - ( quantized_red * color_weight_a + domain_pixels->red * color_weight_b ) / ( color_weight_a + color_weight_b );
                            long long int green_error = range_pixels->green - ( quantized_green * color_weight_a + domain_pixels->green * color_weight_b ) / ( color_weight_a + color_weight_b );
                            long long int blue_error = range_pixels->blue - ( quantized_blue * color_weight_a + domain_pixels->blue * color_weight_b ) / ( color_weight_a + color_weight_b );
                            error += red_error * red_error + green_error * green_error + blue_error * blue_error;
                        }

                        // Is it out best match yet?
                        if ( error < blocks[ range_y ][ range_x ].error )
                        {
                            blocks[ range_y ][ range_x ].orientation = orientation;
                            blocks[ range_y ][ range_x ].x = domain_x;
                            blocks[ range_y ][ range_x ].y = domain_y;
                            blocks[ range_y ][ range_x ].red = red_bits;
                            blocks[ range_y ][ range_x ].green = green_bits;
                            blocks[ range_y ][ range_x ].blue = blue_bits;
                            blocks[ range_y ][ range_x ].error = error;
                        }

                    }
                }
            }
        }
    }
}

void decompress(
    char const *filename )
{
    // Start out with a black range image.
    Image range( Geometry( image_width, image_height ), Color( "black" ) );
    range.backgroundColor( Color( "black" ) );

    // Then go for a fixed number of iterations.  Saving the image after each iteration produces a
    // fun progressive reveal of the decompressed image.
    for ( int iteration = 0; iteration < iterations; ++iteration )
    {
        cout << iteration << "/" << iterations << "\r" << flush;

        // Precompute the downsampling and all of the different orientations of the range image.
        Image orientations[ 16 ];
        orientations[ 0 ] = range;
        orientations[ 0 ].zoom( "50%" );
        for ( int orientation = 1; orientation < 16; ++orientation )
        {
            orientations[ orientation ] = orientations[ 0 ];
            if ( orientation & 1 )
                orientations[ orientation ].flip();
            if ( orientation & 2 )
                orientations[ orientation ].flop();
            orientations[ orientation ].rotate( orientation / 4 * 45 );
        }

        // Then loop over all of the range blocks, and replace each with a copy of the domain block
        // that our compression data tells us to.  We also do the color blending as part of this.
        for ( int range_y = 0; range_y < blocks_in_y; ++range_y )
        {
            int range_top = image_height * range_y / blocks_in_y;
            int block_height = image_height * ( range_y + 1) / blocks_in_y - range_top;
            for ( int range_x = 0; range_x < blocks_in_x; ++range_x )
            {
                int range_left = image_width * range_x / blocks_in_x;
                int block_width = image_width * ( range_x + 1) / blocks_in_x - range_left;

                block &current = blocks[ range_y ][ range_x ];

                Image domain = orientations[ current.orientation ];
                Geometry domain_size = domain.size();

                int domain_top = ( domain_size.height() - block_height ) * current.y / steps_in_y;
                int domain_left = ( domain_size.width() - block_width ) * current.x / steps_in_x;
                domain.crop( Geometry( block_width, block_height, domain_left, domain_top ) );

                int quantized_red = current.red * QuantumRange / ( steps_in_red - 1 );
                int quantized_green = current.green * QuantumRange / ( steps_in_green - 1 );
                int quantized_blue = current.blue * QuantumRange / ( steps_in_blue - 1 );
                domain.colorize( 100 * color_weight_a / ( color_weight_a + color_weight_b ),
                                 Color( quantized_red, quantized_green, quantized_blue ) );

                range.draw( DrawableCompositeImage( range_left, range_top, domain ) );
            }
        }

    }

    range.write( filename );
}

void encode(
    char const *filename )
{
    // For encoding, we just take all the information stored in the blocks structure, along with the
    // image size, and turn it into a huge number.  This is like using a variable base.
    mpz_class number = 0;
    for ( int y = 0; y < blocks_in_y; ++y )
        for ( int x = 0; x < blocks_in_x; ++x )
        {
            block &current = blocks[ y ][ x ];
            int part = current.orientation;
            part = part * steps_in_x + current.x;
            part = part * steps_in_y + current.y;
            part = part * steps_in_red + current.red;
            part = part * steps_in_green + current.green;
            part = part * steps_in_blue + current.blue;
            number *= 16 * steps_in_x * steps_in_y * steps_in_red * steps_in_green * steps_in_blue;
            number += part;
        }
    number = number * maximum_width + image_width;
    number = number * maximum_height + image_height;

    // Next, we convert that to characters.  We essentially just convert the giant number into the
    // base of how ever many characters we have available in our encoding.
    int count = 0;
    ofstream out( filename, ios::out | ios::binary );
    while ( number != 0 )
    {
        int value = static_cast< mpz_class >( number % number_assigned ).get_si();
        number /= number_assigned;

        // Lookup the character that this value maps to.
        int character = 0;
        for ( int index = 0; ; index += 2 )
        {
            character += codes[ index ] + min( value, codes[ index + 1 ] );
            if ( value == codes[ index + 1 ] )
                character += codes[ index + 2 ];
            value -= min( value, codes[ index + 1 ] );
            if ( !value )
                break;
        }

        // And UTF-8 encode that and output it.
        if ( character < 0x80 )
            out << static_cast< char >( character );
        else if ( character < 0x800 )
            out << static_cast< char >( 0xc0 | character >>  6 & 0x1f )
                << static_cast< char >( 0x80 | character >>  0 & 0x3f );
        else if ( character < 0x10000 )
            out << static_cast< char >( 0xe0 | character >> 12 & 0x0f )
                << static_cast< char >( 0x80 | character >>  6 & 0x3f )
                << static_cast< char >( 0x80 | character >>  0 & 0x3f );
        else if ( character < 0x200000 )
            out << static_cast< char >( 0xf0 | character >> 18 & 0x07 )
                << static_cast< char >( 0x80 | character >> 12 & 0x3f )
                << static_cast< char >( 0x80 | character >>  6 & 0x3f )
                << static_cast< char >( 0x80 | character >>  0 & 0x3f );
        ++count;
    }
    cout << "Characters written: " << count << endl;
}

void decode(
    char const *filename )
{
    // Here we build back that giant number.  Again, it's deriving a number in the base of however
    // many characters there are in the character set we're using.
    ifstream in( filename, ios::out | ios::binary );
    mpz_class number = 0;
    mpz_class place = 1;
    for ( ; ; )
    {
        // Read in a UTF-8 character
        int character = in.get();
        if ( character == EOF )
            break;
        if ( ( character & 0xe0 ) == 0xc0 )
            character = ( ( character & 0x1f ) <<  6 |
                          in.get()    & 0x3f );
        else if ( ( character & 0xf0 ) == 0xe0 )
            character = ( ( character & 0x0f ) << 12 |
                          ( in.get()  & 0x3f ) <<  6 |
                          in.get()    & 0x3f );
        else if ( ( character & 0xf8 ) == 0xf0 )
            character = ( ( character & 0x07 ) << 18 |
                          ( in.get()  & 0x3f )  << 12 |
                          ( in.get()  & 0x3f )  <<  6 |
                          in.get()    & 0x3f );

        // And convert it back to a value.
        int value = 0;
        for ( int index = 0; character; index += 2 )
        {
            character -= codes[ index ];
            value += min( character, codes[ index + 1 ] );
            character -= min( character, codes[ index + 1 ] );
        }

        // We're reading back in order from least to most significant.
        number += value * place;
        place *= number_assigned;
    }

    // Now, we do the conversion back from that giant number to fill the image size and the block
    // data.  This is all just the reverse order of the encoding of this into the giant number.
    image_height = static_cast< mpz_class >( number % maximum_height ).get_si();
    number /= maximum_height;
    image_width = static_cast< mpz_class >( number % maximum_width ).get_si();
    number /= maximum_width;
    for ( int y = blocks_in_y - 1; y >= 0; --y )
        for ( int x = blocks_in_x - 1; x >= 0; --x )
        {
            block &current = blocks[ y ][ x ];
            current.blue = static_cast< mpz_class >( number % steps_in_blue ).get_si();
            number /= steps_in_blue;
            current.green = static_cast< mpz_class >( number % steps_in_green ).get_si();
            number /= steps_in_green;
            current.red = static_cast< mpz_class >( number % steps_in_red ).get_si();
            number /= steps_in_red;
            current.y = static_cast< mpz_class >( number % steps_in_y ).get_si();
            number /= steps_in_y;
            current.x = static_cast< mpz_class >( number % steps_in_x ).get_si();
            number /= steps_in_x;
            current.orientation = static_cast< mpz_class >( number % 16 ).get_si();
            number /= 16;
        }
}

int main(
    int argc,
    char **argv )
{
    if ( !strcmp( argv[ 1 ], "encode" ) )
    {
        compress( argv[ 2 ] );
        encode( argv[ 3 ] );
    }
    else
    {
        decode( argv[ 2 ] );
        decompress( argv[ 3 ] );
    }
    return 0;
}