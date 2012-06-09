#include <math.h>
#include <string.h>
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_mouse.h>

#include <getopt.h>

#include "png_textures.h"
#include "process_status.h"

float sleeping[] = {
    153 / 510.0, 153 / 510.0, 255 / 510.0, 1.000000,
    153 / 255.0, 153 / 255.0, 255 / 255.0, 1.000000,
    153 / 255.0, 153 / 255.0, 255 / 255.0, 1.000000,
    9
};

float transparent[] = {
    0 / 510.0, 255 / 510.0, 255 / 510.0, 0.3,
    0 / 255.0, 255 / 255.0, 255 / 255.0, 0.3,
    0 / 255.0, 255 / 255.0, 255 / 255.0, 0.3,
    9
};

float nice[] = {
    255 / 510.0, 153 / 510.0, 102 / 510.0, 1.000000,
    255 / 255.0, 153 / 255.0, 102 / 255.0, 1.000000,
    255 / 255.0, 153 / 255.0, 102 / 255.0, 1.000000,
    9
};

float running[] = {
    255 / 510.0, 153 / 510.0, 0 / 510.0, 1.000000,
    255 / 255.0, 153 / 255.0, 0 / 255.0, 1.000000,
    255 / 255.0, 153 / 255.0, 0 / 255.0, 1.000000,
    9
};

float disk[] = {
    0.5, 0, 0, 1,
    1, 0, 0, 1,
    1, 0, 0, 1,
    9
};

float zombie[] = {
    204 / 510.0, 102 / 510.0, 102 / 510.0, 1.000000,
    204 / 255.0, 102 / 255.0, 102 / 255.0, 1.000000,
    204 / 255.0, 102 / 255.0, 102 / 255.0, 1.000000,
    9
};

float suspended[] = {
    204 / 510.0, 102 / 510.0, 102 / 510.0, 1.000000,
    204 / 255.0, 102 / 255.0, 102 / 255.0, 1.000000,
    204 / 255.0, 102 / 255.0, 102 / 255.0, 1.000000,
    9
};

process_status *ps, *oldps;

static void set_material(process_status *p) {
  float *m = transparent;

  if (p->state == 'S') {
    m = sleeping;
  } else if (p->state == 'R') {
    if (p->nice > 0) {
      m = nice;
    } else {
      m = running;
    }
  } else if (p->state == 'D') {
    m = disk;
  } else if (p->state == 'Z') {
    m = zombie;
  } else if (p->state == 'T') {
    m = suspended;
  } else {
    printf("state is %c\n", p->state);
    m = transparent;
  }

  glMaterialfv(GL_FRONT, GL_AMBIENT,  m);
  glMaterialfv(GL_FRONT, GL_DIFFUSE,  m + 4);
  glMaterialfv(GL_FRONT, GL_SPECULAR, m + 8);
  glMaterialf(GL_FRONT, GL_SHININESS, m[12]);
}

static void setupLights(void) {
  GLfloat light = 0.2;
  GLfloat pos0[4] = {40, 40, 0, 1};
  GLfloat pos1[4] = {-40, 40, 0, 1};
  GLfloat pos2[4] = {0, 0, 0, 1};
  GLfloat amb[4] = {light, light, light, 1.0};
  GLfloat dif[4] = {light, light, light, 1.0};
  GLfloat spc[4] = {light, light, light, 1.0};

  glEnable(GL_LIGHTING);

  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, pos0);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, spc);

  glEnable(GL_LIGHT1);
  glLightfv(GL_LIGHT1, GL_POSITION, pos1);
  glLightfv(GL_LIGHT1, GL_AMBIENT,  amb);
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  dif);
  glLightfv(GL_LIGHT1, GL_SPECULAR, spc);

  glEnable(GL_LIGHT2);
  glLightfv(GL_LIGHT2, GL_POSITION, pos2);
  glLightfv(GL_LIGHT2, GL_AMBIENT,  amb);
  glLightfv(GL_LIGHT2, GL_DIFFUSE,  dif);
  glLightfv(GL_LIGHT2, GL_SPECULAR, spc);
}

static int gridx = 5;
static int gridz = 5;

struct camera {
  double y, z;
  double rotx, roty, rotz;
};

static struct camera camera_sw =   {  2,  20,  0,  -30 };
static struct camera camera_nw =   {  8,  20, 30, -150 };
static struct camera camera_tilt = {  2,  20,  0,    0, -90 };
static struct camera camera_top =  { 16,   0, 90,  180 };
static struct camera camera_s =    {  2,  20 };
static struct camera camera_mid =  {  2 };

static struct camera camera = { 2, 20, 10, -30 };

static struct camera srccam, dstcam;
static double animation = 0;

static int jiffycount = 1;
static int fps = 50;
static int delay = 11;
int ups = 25;

static double rss_scale = 512.0;
static double spin_speed = 0;

/* smooth start and stop of animation */
static double blend(double x) {
  return 3 * x * x - 2 * x * x * x;
}

static double interpolate(double src, double dst, double t) {
  return blend(t) * src + blend(1 - t) * dst;
}

static void resizeWindow(GLsizei width, GLsizei height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, 1.0 * width / height, 0.1, 100);
  glMatrixMode(GL_MODELVIEW);
}

static void move_camera(struct camera dst) {
  srccam = camera;
  srccam.roty = fmod(srccam.roty, 360);
  dstcam = dst;
  animation = 1;
  spin_speed = 0;
}

int main(int argc, char *argv[]) {
  GLuint *font;
  unsigned framecount = 0;
  Uint32 before;

  static struct option long_options[] = {
    {"help",        no_argument,       0, 'h'},
    {"frame-rate",  required_argument, 0, 'f'},
    {"update-rate", required_argument, 0, 'u'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;

  while ((c = getopt_long(argc, argv, "hf:u:", long_options, &option_index))
	 != -1) {
    switch (c) {
    case 'h':
      printf("Available options:\n\n"
	     "-f\t--frame-rate\tFrame rate in frames per second.\n"
	     "\t\t\tRecommended values are 100, 50 (default), 33, 25, 20...\n\n"
	     "-u\t--update-rate\tData update frequency.\n"
	     "\t\t\tDefault is 25.\n");

    return 0;
    case 'f':
      sscanf(optarg, "%d", &fps);
      break;
    case 'u':
      sscanf(optarg, "%d", &ups);
      break;
    default:
      printf("option %c\n", c);
      break;
    }
  }

  printf("Bindings:\n"
	 "mouse 3 drag\t\t\tadjust camera position\n"
	 "mouse 1+3 drag vertically\tzoom\n"
	 "mouse wheel\t\t\tzoom\n"
	 "mouse 1 drag vertically\t\tscale height\n"
	 "mouse 1 drag horizontally\tadjust spin speed\n"
	 "F1--F6\t\t\t\tpredefined cameras\n"
	 "Esc, Q\t\t\t\tquit\n");

  if (SDL_Init(SDL_INIT_VIDEO) < 0
       || !SDL_SetVideoMode(640, 480, 0, SDL_OPENGL | SDL_RESIZABLE)) {
    return 1;
  }

  {
    char *slash = strrchr(argv[0], '/');
    SDL_WM_SetCaption(slash ? slash + 1 : argv[0], NULL);
  }

  before = SDL_GetTicks();

  glClearColor(0, 0, 0, 0);
  glClearDepth(1.0);		/* Enables Clearing Of The Depth Buffer */
  glDepthFunc(GL_LESS);		/* The Type Of Depth Test To Do */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);	/* Enables Depth Testing */
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_SMOOTH);	/* Enables Smooth Color Shading */
  resizeWindow(640, 480);
  setupLights();

  font = read_font();

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  try {
  oldps = read_processes_status();
  diff(oldps, 0);		/* initializes textures */

  delay = 1000 / fps - 9;

  while (1) {
    process_status *p;
    int i = gridx;
    SDL_Event event;

    if (--jiffycount <= 0) {
      jiffycount = fps / ups;
      ps = read_processes_status();
      diff(ps, oldps);
      free_processes_status(oldps);
      oldps = ps;
    }

    if (gridx * gridz < process_count
	|| (gridx - 1) * gridz > process_count) {
      gridx = ceil(sqrt(process_count));
      gridz = floor(sqrt(process_count));

      if (gridx * gridz < process_count) {
	gridz++;
      }
    }

    if (animation > 0) {
      camera.y = interpolate(srccam.y, dstcam.y, animation);
      camera.z = interpolate(srccam.z, dstcam.z, animation);
      camera.rotx = interpolate(srccam.rotx, dstcam.rotx, animation);
      camera.roty = interpolate(srccam.roty, dstcam.roty, animation);
      camera.rotz = interpolate(srccam.rotz, dstcam.rotz, animation);
      animation -= 1.0 / fps;
    }

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glRotatef(camera.rotz, 0, 0, 1);
    glTranslatef(0,0/* -camera.y */,-camera.z);
    glRotatef(camera.rotx, 1, 0, 0);
    glRotatef(camera.roty += spin_speed, 0, 1, 0);

    glPushMatrix();

    glTranslatef(-1 - gridx, 0, 1 - gridz);

    i = gridx;

    for (p = ps; p; p = p->next_process) {
      if (i--) {
	glTranslatef(2, 0, 0);
      } else {
	i = gridx - 1;
	glTranslatef(2 * (1 - gridx), 0, 2);
      }

      set_material(p);

      {
	int namelen = strlen(p->comm);
	char pid[6];
	int pidlen;
	GLfloat thickness = 0.5 + 1.0 * p->cpu / 100;
	GLfloat xl = -thickness;
	GLfloat xr = thickness;
	GLfloat zf = thickness;
	GLfloat zb = -thickness;
	GLfloat yt = p->rss / rss_scale;
	int vertical = yt > 2 * thickness;
	GLfloat minh = 4 * thickness / namelen;
	GLfloat yb = yt < minh ? yt - minh : 0;

	sprintf(pid, "%d", p->pid);
	pidlen = strlen(pid);

	{
	  int i;

	  for (i = 0; i < namelen; i++) {
	    glBindTexture(GL_TEXTURE_2D, font[p->comm[i]]);
	    glBegin(GL_QUADS);
	    glNormal3f(0, 0, 1);

	    if (vertical) {
	      glTexCoord2f(1, 0);
	      glVertex3f(xl, yb + yt / namelen * (i + 1), zf);
	      glTexCoord2f(0, 0);
	      glVertex3f(xl, yb + yt / namelen * i, zf);
	      glTexCoord2f(0, 1);
	      glVertex3f(xr, yb + yt / namelen * i, zf);
	      glTexCoord2f(1, 1);
	      glVertex3f(xr, yb + yt / namelen * (i + 1), zf);
	    } else {
	      glTexCoord2f(1, 0);
	      glVertex3f(xl + (xr - xl) / namelen * (i + 1), yt, zf);
	      glTexCoord2f(0, 0);
	      glVertex3f(xl + (xr - xl) / namelen * i, yt, zf);
	      glTexCoord2f(0, (yt - yb) / minh);
	      glVertex3f(xl + (xr - xl) / namelen * i, yb, zf);
	      glTexCoord2f(1, (yt - yb) / minh);
	      glVertex3f(xl + (xr - xl) / namelen * (i + 1), yb, zf);
	    }

	    glNormal3f(0, 0, -1);
	    if (vertical) {
	      glTexCoord2f(1, 0);
	      glVertex3f(xr, yb + yt / namelen * (i + 1), zb);
	      glTexCoord2f(0, 0);
	      glVertex3f(xr, yb + yt / namelen * i, zb);
	      glTexCoord2f(0, 1);
	      glVertex3f(xl, yb + yt / namelen * i, zb);
	      glTexCoord2f(1, 1);
	      glVertex3f(xl, yb + yt / namelen * (i + 1), zb);
	    } else {
	      glTexCoord2f(1, 0);
	      glVertex3f(xr + (xl - xr) / namelen * (i + 1), yt, zb);
	      glTexCoord2f(0, 0);
	      glVertex3f(xr + (xl - xr) / namelen * i, yt, zb);
	      glTexCoord2f(0, (yt - yb) / minh);
	      glVertex3f(xr + (xl - xr) / namelen * i, yb, zb);
	      glTexCoord2f(1, (yt - yb) / minh);
	      glVertex3f(xr + (xl - xr) / namelen * (i + 1), yb, zb);
	    }

	    glEnd();
	  }

	  for (i = 0; i < pidlen; i++) {
	    glBindTexture(GL_TEXTURE_2D, font[pid[i]]);
	    glBegin(GL_QUADS);
	    glNormal3f(1, 0, 0);

	    if (vertical) {
	      glTexCoord2f(1, 0);
	      glVertex3f(xr, yb + yt * (i + 1) / pidlen, zf);
	      glTexCoord2f(0, 0);
	      glVertex3f(xr, yb + yt * i / pidlen, zf);
	      glTexCoord2f(0, 1);
	      glVertex3f(xr, yb + yt * i / pidlen, zb);
	      glTexCoord2f(1, 1);
	      glVertex3f(xr, yb + yt * (i + 1) / pidlen, zb);
	    } else {
	      glTexCoord2f(1, 0);
	      glVertex3f(xr, yt, zf + (zb - zf) / pidlen * (i + 1));
	      glTexCoord2f(0, 0);
	      glVertex3f(xr, yt, zf + (zb - zf) / pidlen * i);
	      glTexCoord2f(0, 1);
	      glVertex3f(xr, yb, zf + (zb - zf) / pidlen * i);
	      glTexCoord2f(1, 1);
	      glVertex3f(xr, yb, zf + (zb - zf) / pidlen * (i + 1));
	    }

	    glNormal3f(-1, 0, 0);

	    if (vertical) {
	      glTexCoord2f(1, 0);
	      glVertex3f(xl, yb + yt * (i + 1) / pidlen, zb);
	      glTexCoord2f(0, 0);
	      glVertex3f(xl, yb + yt * i / pidlen, zb);
	      glTexCoord2f(0, 1);
	      glVertex3f(xl, yb + yt * i / pidlen, zf);
	      glTexCoord2f(1, 1);
	      glVertex3f(xl, yb + yt * (i + 1) / pidlen, zf);
	    } else {
	      glTexCoord2f(1, 0);
	      glVertex3f(xl, yt, zb + (zf - zb) / pidlen * (i + 1));
	      glTexCoord2f(0, 0);
	      glVertex3f(xl, yt, zb + (zf - zb) / pidlen * i);
	      glTexCoord2f(0, 1);
	      glVertex3f(xl, yb, zb + (zf - zb) / pidlen * i);
	      glTexCoord2f(1, 1);
	      glVertex3f(xl, yb, zb + (zf - zb) / pidlen * (i + 1));
	    }

	    glNormal3f(0, 1, 0);
	    glTexCoord2f(0, 0);
	    glVertex3f(xr - i * 2 * thickness / pidlen, yt, (zf + zb) / 2);
	    glTexCoord2f(0, 1);
	    glVertex3f(xr - i * 2 * thickness / pidlen, yt, zb);
	    glTexCoord2f(1, 1);
	    glVertex3f(xr - (i + 1) * 2 * thickness / pidlen, yt, zb);
	    glTexCoord2f(1, 0);
	    glVertex3f(xr - (i + 1) * 2 * thickness / pidlen, yt, (zf + zb) / 2);

	    glTexCoord2f(0, 0);
	    glVertex3f(xl + i * 2 * thickness / pidlen, yt, (zf + zb) / 2);
	    glTexCoord2f(0, 1);
	    glVertex3f(xl + i * 2 * thickness / pidlen, yt, zf);
	    glTexCoord2f(1, 1);
	    glVertex3f(xl + (i + 1) * 2 * thickness / pidlen, yt, zf);
	    glTexCoord2f(1, 0);
	    glVertex3f(xl + (i + 1) * 2 * thickness / pidlen, yt, (zf + zb) / 2);

	    glEnd();
	  }
	}
      }
    }

    glPopMatrix();

    {
      float *m = transparent;
      glMaterialfv(GL_FRONT, GL_AMBIENT,  m);
      glMaterialfv(GL_FRONT, GL_DIFFUSE,  m + 4);
      glMaterialfv(GL_FRONT, GL_SPECULAR, m + 8);
      glMaterialf(GL_FRONT, GL_SHININESS, m[12]);
    }

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-9, -0.01,  9);
    glVertex3f( 9, -0.01,  9);
    glVertex3f( 9, -0.01, -9);
    glVertex3f(-9, -0.01, -9);
    glNormal3f(0, -1, 0);
    glVertex3f(-9, -0.01,  9);
    glVertex3f(-9, -0.01, -9);
    glVertex3f( 9, -0.01, -9);
    glVertex3f( 9, -0.01,  9);
    glEnd();

    SDL_GL_SwapBuffers();
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_VIDEORESIZE:
	if (!SDL_SetVideoMode(event.resize.w, event.resize.h,
			      0, SDL_OPENGL | SDL_RESIZABLE)) {
	  return 1;
	}

	resizeWindow(event.resize.w, event.resize.h);
	break;
      case SDL_KEYDOWN:
	if (event.key.keysym.sym == SDLK_ESCAPE) {
	  SDL_Quit();
	  return 0;
	} else if (event.key.keysym.sym == SDLK_q) {
	  SDL_Quit();
	  return 0;
	} else if (event.key.keysym.sym == SDLK_F1) {
	  move_camera(camera_sw);
	} else if (event.key.keysym.sym == SDLK_F2) {
	  move_camera(camera_nw);
	} else if (event.key.keysym.sym == SDLK_F3) {
	  move_camera(camera_tilt);
	} else if (event.key.keysym.sym == SDLK_F4) {
	  move_camera(camera_top);
	} else if (event.key.keysym.sym == SDLK_F5) {
	  move_camera(camera_s);
	} else if (event.key.keysym.sym == SDLK_F6) {
	  move_camera(camera_mid);
	}

	break;
      case SDL_MOUSEBUTTONDOWN:
	spin_speed = 0;

	if (event.button.button == SDL_BUTTON_WHEELDOWN) {
	  camera.y *= 1.1;
	  camera.z *= 1.1;
	} else if (event.button.button == SDL_BUTTON_WHEELUP) {
	  camera.y /= 1.1;
	  camera.z /= 1.1;
	}

	break;
      case SDL_MOUSEMOTION:
	if (event.motion.state == (SDL_BUTTON(1) | SDL_BUTTON(3))) {
	  camera.z += event.motion.yrel / 16.0;
	} else if (event.motion.state & SDL_BUTTON(1)) {
	  spin_speed += event.motion.xrel / 256.0;
	  rss_scale *= (1 + event.motion.yrel / 256.0);
	} else if (event.motion.state & SDL_BUTTON(3)) {
	  camera.roty += event.motion.xrel / 4.0;
	  camera.rotx += event.motion.yrel / 4.0;
	}

	break;
      }
    }

    if (++framecount % fps == 0) {
      Uint32 now = SDL_GetTicks();
      printf("%f fps\n", 1000.0 * framecount / (now - before));
      framecount = 0;
      before = now;
    }

    if (delay > 0) {
      SDL_Delay(delay);
    }
  }
  } catch (const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
  }
}
