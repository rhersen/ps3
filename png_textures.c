#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "png_textures.h"

static struct pngtx *new_pngtx(void) {
  struct pngtx empty = { 0, 1, 1 };
  struct pngtx *r = (pngtx *) malloc(sizeof(struct pngtx));
  *r = empty;
  return r;
}

struct pngtx *read_png(const char *pngfile) {
  FILE *fp = fopen(pngfile, "rb");
  struct pngtx *r;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  unsigned i;

  if (!fp) {
/*     printf("couldn't open %s\t", pngfile); */
    return 0;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  info_ptr = png_create_info_struct(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    fclose(fp);
    return 0;
  }

  png_init_io(png_ptr, fp);
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, png_voidp_NULL);

  row_pointers = png_get_rows(png_ptr, info_ptr);

  r = new_pngtx();
  r->txw = 1;
  r->txh = 1;
  r->imw = png_get_image_width(png_ptr, info_ptr);
  r->imh = png_get_image_height(png_ptr, info_ptr);

  while (r->txw < r->imw) {
    r->txw <<= 1;
  }

  while (r->txh < r->imh) {
    r->txh <<= 1;
  }

  r->bytes_per_pixel =
    png_get_bit_depth(png_ptr, info_ptr)
    * png_get_channels(png_ptr, info_ptr)
    / 8;

  r->data = (char*) malloc(r->txw
		   * r->bytes_per_pixel
		   * r->txh);

  for (i = 0; i < png_get_image_height(png_ptr, info_ptr); i++) {
    memcpy(r->data + i * r->txw * r->bytes_per_pixel,
	   row_pointers[i],
	   png_get_rowbytes(png_ptr, info_ptr));
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
  fclose(fp);

  return r;
}

static void set_min_filter(void) {
#ifdef GL_GENERATE_MIPMAP_SGIS
    if (strstr((const char*) glGetString(GL_EXTENSIONS), "GL_SGIS_generate_mipmap")) {
    glTexParameteri(GL_TEXTURE_2D,
		    GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
}

GLuint *read_font(void) {
  unsigned char i;
  struct pngtx *tx;
  GLuint *r = (GLuint*) malloc('z' * sizeof(GLuint));
  glGenTextures('z', r);

  for (i = ' '; i <= 'z'; i++) {
    char filename[] = "font/*.png";
    filename[5] = i;
    tx = read_png(filename);

    if (!tx) {
      unsigned char pixel_data[] =
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\377\0\0\0\0"
	"\0\0\0\0\0\0\0\377\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

      tx = new_pngtx();
      tx->bytes_per_pixel = 4;
      tx->txw = 4;
      tx->txh = 4;
      tx->data = (char*) malloc(tx->txw * tx->bytes_per_pixel * tx->txh);
      memcpy(tx->data, pixel_data, tx->txw * tx->bytes_per_pixel * tx->txh);
    }

    glBindTexture(GL_TEXTURE_2D, r[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    set_min_filter();

    glTexImage2D(GL_TEXTURE_2D, 0,
		 tx->bytes_per_pixel == 4 ?
		 GL_RGBA : GL_RGB,
		 tx->txw, tx->txh, 0,
		 tx->bytes_per_pixel == 4 ? GL_RGBA : GL_RGB,
		 GL_UNSIGNED_BYTE, tx->data);

    free(tx->data);
    free(tx);
  }

  return r;
}
