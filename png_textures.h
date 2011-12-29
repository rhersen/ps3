#include <GL/gl.h>

struct pngtx {
  GLuint txw, txh;
  GLuint imw, imh;
  GLfloat rotate;
  int row, col;
  int bytes_per_pixel;
  void *data;
};

void setTexture(struct pngtx *tx);
struct pngtx *read_png(const char *pngfile);
GLuint *read_font(void);

