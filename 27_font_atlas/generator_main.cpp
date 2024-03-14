/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 5 Feb 2014                                                     |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries and assets for respective legal notices             |
|******************************************************************************|
| Font Atlas Generator example                                                 |
| Uses Sean Barrett's STB_IMAGE_WRITE library for writing to PNG               |
\******************************************************************************/
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <freetype2/ft2build.h>  // FreeType header
#include FT_FREETYPE_H // unusual macro
#include <freetype/ftglyph.h>   // needed for bounding box bit
#include <stdio.h>
#include <stdlib.h> // some memory management is done

/* using the FreeMono font from the GNU fonts collection. this is free and has a
"copy-left" licence. see package for details */
#define FONT_FILE_NAME "FreeMono.ttf"
#define PNG_OUTPUT_IMAGE "atlas.png"
#define ATLAS_META_FILE "atlas.meta"

int main() {
  // Now we can initialise FreeType
  FT_Library ft;
  if ( FT_Init_FreeType( &ft ) ) {
    fprintf( stderr, "Could not init FreeType library\n" );
    return 1;
  }
  // load a font face from a file
  FT_Face face;
  if ( FT_New_Face( ft, FONT_FILE_NAME, 0, &face ) ) {
    fprintf( stderr, "Could not open font\n" );
    return 1;
  }
  int atlas_dimension_px = 1024;            // atlas size in pixels
  int atlas_columns      = 16;              // number of glyphs across atlas
  int padding_px         = 6;               // total space in glyph size for outlines
  int slot_glyph_size    = 64;              // glyph maximum size in pixels
  int atlas_glyph_px     = 64 - padding_px; // leave some padding for outlines

  // Next we can open a file stream to write our atlas image to
  unsigned char* atlas_buffer     = (unsigned char*)malloc( atlas_dimension_px * atlas_dimension_px * 4 * sizeof( unsigned char ) );
  unsigned int atlas_buffer_index = 0;

  // I'll tell FreeType the maximum size of each glyph in pixels
  int grows[256];                              // glyph height in pixels
  int gwidth[256];                             // glyph width in pixels
  int gpitch[256];                             // bytes per row of pixels
  int gymin[256];                              // offset for letters that dip below baseline like g and y
  unsigned char* glyph_buffer[256] = { NULL }; // stored glyph images
  // set height in pixels width 0 height 48 (48x48)
  FT_Set_Pixel_Sizes( face, 0, atlas_glyph_px );
  for ( int i = 33; i < 256; i++ ) {
    if ( FT_Load_Char( face, i, FT_LOAD_RENDER ) ) {
      fprintf( stderr, "Could not load character %i\n", i );
      return 1;
    }
    // draw glyph image anti-aliased
    FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
    // get dimensions of bitmap
    grows[i]  = face->glyph->bitmap.rows;
    gwidth[i] = face->glyph->bitmap.width;
    gpitch[i] = face->glyph->bitmap.pitch;
    // copy glyph data into memory because it seems to be overwritten/lost later
    glyph_buffer[i] = (unsigned char*)malloc( grows[i] * gpitch[i] );
    memcpy( glyph_buffer[i], face->glyph->bitmap.buffer, face->glyph->bitmap.rows * face->glyph->bitmap.pitch );

    // get y-offset to place glyphs on baseline. this is in the bounding box
    FT_Glyph glyph; // a handle to the glyph image
    if ( FT_Get_Glyph( face->glyph, &glyph ) ) {
      fprintf( stderr, "Could not get glyph handle %i\n", i );
      return 1;
    }
    // get bbox. "truncated" mode means get dimensions in pixels
    FT_BBox bbox;
    FT_Glyph_Get_CBox( glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox );
    gymin[i] = bbox.yMin;
  }

  for ( int y = 0; y < atlas_dimension_px; y++ ) {
    for ( int x = 0; x < atlas_dimension_px; x++ ) {
      // work out which grid slot[col][row] we are in e.g out of 16x16
      int col         = x / slot_glyph_size;
      int row         = y / slot_glyph_size;
      int order       = row * atlas_columns + col;
      int glyph_index = order + 32;

      // an actual glyph bitmap exists for these indices
      if ( glyph_index > 32 && glyph_index < 256 ) {
        // pixel indices within padded glyph slot area
        int x_loc = x % slot_glyph_size - padding_px / 2;
        int y_loc = y % slot_glyph_size - padding_px / 2;
        // outside of glyph dimensions use a transparent, black pixel (0,0,0,0)
        if ( x_loc < 0 || y_loc < 0 || x_loc >= gwidth[glyph_index] || y_loc >= grows[glyph_index] ) {
          atlas_buffer[atlas_buffer_index++] = 0;
          atlas_buffer[atlas_buffer_index++] = 0;
          atlas_buffer[atlas_buffer_index++] = 0;
          atlas_buffer[atlas_buffer_index++] = 0;
        } else {
          // this is 1, but it's safer to put it in anyway
          // int bytes_per_pixel = gwidth[glyph_index] / gpitch[glyph_index];
          // int bytes_in_glyph = grows[glyph_index] * gpitch[glyph_index];
          int byte_order_in_glyph = y_loc * gwidth[glyph_index] + x_loc;
          unsigned char colour[4];
          colour[0] = colour[1] = colour[2] = colour[3] = glyph_buffer[glyph_index][byte_order_in_glyph];
          // print byte from glyph
          atlas_buffer[atlas_buffer_index++] = glyph_buffer[glyph_index][byte_order_in_glyph];
          atlas_buffer[atlas_buffer_index++] = glyph_buffer[glyph_index][byte_order_in_glyph];
          atlas_buffer[atlas_buffer_index++] = glyph_buffer[glyph_index][byte_order_in_glyph];
          atlas_buffer[atlas_buffer_index++] = glyph_buffer[glyph_index][byte_order_in_glyph];
        }
        // write black in non-graphical ASCII boxes
      } else {
        atlas_buffer[atlas_buffer_index++] = 0;
        atlas_buffer[atlas_buffer_index++] = 0;
        atlas_buffer[atlas_buffer_index++] = 0;
        atlas_buffer[atlas_buffer_index++] = 0;
      } // endif
    }   // endfor
  }     // endfor

  // write meta-data file to go with atlas image
  FILE* fp = fopen( ATLAS_META_FILE, "w" );
  // comment, reminding me what each column is
  fprintf( fp,
    "// ascii_code prop_xMin prop_width prop_yMin prop_height "
    "prop_y_offset\n" );
  // write an unique line for the 'space' character
  fprintf( fp, "32 0 %f 0 %f 0\n", 0.5f, 1.0f );
  // write a line for each regular character
  for ( int i = 33; i < 256; i++ ) {
    int order   = i - 32;
    int col     = order % atlas_columns;
    int row     = order / atlas_columns;
    float x_min = (float)( col * slot_glyph_size ) / (float)atlas_dimension_px;
    float y_min = (float)( row * slot_glyph_size ) / (float)atlas_dimension_px;
    fprintf( fp, "%i %f %f %f %f %f\n", i, x_min, (float)( gwidth[i] + padding_px ) / (float)slot_glyph_size, y_min, ( grows[i] + padding_px ) / (float)slot_glyph_size,
      -( (float)padding_px - (float)gymin[i] ) / (float)slot_glyph_size );
  }
  fclose( fp );

  // free that buffer of glyph info
  for ( int i = 0; i < 256; i++ ) {
    if ( NULL != glyph_buffer[i] ) { free( glyph_buffer[i] ); }
  }

  // use stb_image_write to write directly to png
  if ( !stbi_write_png( PNG_OUTPUT_IMAGE, atlas_dimension_px, atlas_dimension_px, 4, atlas_buffer, 0 ) ) { fprintf( stderr, "ERROR: could not write file %s\n", PNG_OUTPUT_IMAGE ); }
  free( atlas_buffer );
  return 0;
}
