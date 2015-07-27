
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1416f
#endif

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glu.h>

#include "shader.h"

/* Added Structure Define */
typedef struct Vec3f {
  GLfloat x, y, z;
} Vec3f;

typedef struct Vec2f {
   float u, v;
} Vec2f;

typedef struct Geometry {
  Vec3f*  vertex;
  GLuint* indice;
  Vec3f*  normal;
  int rows;
  int cols;
  int sizeV;
  int sizeI;
  int sizeN;
} Geometry;

#define GRID_LENGTH       2.0
#define SPHERE_RADIUS     1.0
#define TORUS_EX_RADIUS   1.0
#define TORUS_IN_RADIUS   0.5
#define GEOMETRY_BASE     4
#define GEOMETRY_BASE_MAX 4096
#define SHININESS_MAX     100.0
#define BUMPS_BASE        16
#define BUMPS_BASE_MAX    128
#define MAX_BUMP_SIZE     50
#define WELL_FORM_TESS    2048
#define NOT_SO_WELL_TESS  1024

#define NUM_BUFFERS       3
#define VERTICES          0
#define INDICES           1
#define NORMALS           2
#define NUM_SHADERS       6
#define TRUE              1
#define FALSE             0

#define min(a, b) ((a)<(b)?(a):(b))
#define max(a, b) ((a)>(b)?(a):(b))

enum Options {
  OPT_LIGHTENING,
  OPT_SHADING_SMOOTH_FLAT,
  OPT_SHININESS_CHANGING,
  OPT_LOCAL_VIEWER,
  OPT_DRAW_NORMAL,
  OPT_OSP_SHORT_LONG,
  OPT_STDOUT_TEXT,
  OPT_SHADING_TOGG,
  OPT_SHADER_GEN_SHAPE,
  OPT_SHADING_VERTEX_PIXEL,
  OPT_SHADING_BLINN_PHONG,
  OPT_SHADER_BUMPMAP,
  OPT_SHADER_DISPMAP,
  OPT_SHADER_ANIMATION,
  OPTSIZE //NOTE: this must be the last enumerator.
};

enum shaper {
  SHAPE_IMM_GRID,
  SHAPE_IMM_SPHERE,
  SHAPE_IMM_TORUS,
  SHAPE_GRID,
  SHAPE_SPHERE,
  SHAPE_TORUS,
  SHAPE_INNER_GRID,
  SHAPE_INNER_SHPERE,
  SHAPE_INNER_TORUS,
  NUM_OF_OPT
};

enum gpu_draw_geometry {
  GPU_GRID,
  GPU_SPHERE,
  GPU_TORUS
};

static bool options[OPTSIZE];
static Geometry geometry;

static int currentShape;
static int uTessellation;

float shininess = 50.0;

long lastTime;
int  win;

int lastCalTime = 0;
int frameCount  = 0;

/* shading variables */
GLuint torusShader;
GLuint currentShader = 0;
GLint  sdBlinnOrPhong ,sdVertexOrPixel;
GLuint buffers[NUM_BUFFERS];

/* uniform variables */
bool  uGpuGenShape     = true;
bool  uLocalViewer     = false;
int   uNumOfBumps      = BUMPS_BASE;
int   uCurrBumpSize    = 1;
int   uInnerShapeType;

/* frame rate variable */
char frameRateString [256];
char infoString      [256];
char drawMethodString[256];
char localViewString [50];
char tessString      [50];
char bumpSizeString  [50];

/* camera variables */
float camRotX          = 0.0f;
float camRotY          = 0.0f;
float zoom             = 2.0;
float mouseSensitivity = 0.3;
int   lastMouseX       = 0;
int   lastMouseY       = 0;
int   buttonZoom       = 0;
int   buttonRotate     = 0;

/* vbo variables */
int useBufferObjects = FALSE;
int updateBuffer     = TRUE;

/* shader program handles */
GLuint shaderProgramHandles[ NUM_SHADERS ];

/* key down variables */
GLuint tempShaderHandle;
bool   tempGpuGenShape;
bool   tempShaderStatus;

/* constants for shaders */
enum Shaders { SHADER_VERTEX_BLINN, SHADER_PIXEL_BLINN, SHADER_VERTEX_PHONG,
               SHADER_PIXEL_PHONG, SHADER_BUMP_MAP, SHADER_DISP_MAP };

/* file names for shaders */
static const char *shaderProgramFileNames[NUM_SHADERS][2] =
{
   {"BlinnPhongPerVertex.vert", "BlinnPhongPerVertex.frag"},
   {"BlinnPhongPerPixel.vert",  "BlinnPhongPerPixel.frag"},
   {"PhongPerVertex.vert",      "PhongPerVertex.frag"},
   {"PhongPerPixel.vert",       "PhongPerPixel.frag"},
   {"BumpMap.vert",             "BumpMap.frag"},
   {"DispMap.vert",             "DispMap.frag"}
};

/* settings output string */
static const char *currentDrawingMethod[NUM_OF_OPT][1] =
{
   {"Draw method: Immdiate mode "},
   {"Draw method: Immdiate mode "},
   {"Draw method: Immdiate mode "},
   {"Draw method: CPU Generated Shape using Vertex array & VBO "},
   {"Draw method: CPU Generated Shape using Vertex array & VBO "},
   {"Draw method: CPU Generated Shape using Vertex array & VBO "},
   {"Draw method: GPU Generated Shape using Vertex array & VBO "},
   {"Draw method: GPU Generated Shape using Vertex array & VBO "},
   {"Draw method: GPU Generated Shape using Vertex array & VBO "}
};

/* Added Functions */
#define checkGLErrors() _checkGLErrors(__LINE__)
void _checkGLErrors(int line)
{
  GLuint err;
  if ((err = glGetError()) != GL_NO_ERROR)
    fprintf(stderr, "OpenGL error caught on line %i: %s\n", line,
            gluErrorString(err));
}

void initShaderState()
{
   int shaderIndex;

   for (shaderIndex = 0;  shaderIndex < NUM_SHADERS; ++shaderIndex )
   {
      GLuint shaderProgramHandle =
      loadShader( shaderProgramFileNames[ shaderIndex ][ 0 ],
      shaderProgramFileNames[ shaderIndex ][ 1 ] );

      shaderProgramHandles[ shaderIndex ] = shaderProgramHandle;
   }
   currentShader = shaderProgramHandles[SHADER_VERTEX_BLINN];

}
/* get uniform variable handle */
GLint getUniLoc(GLuint program, const GLchar *name)
{
    GLint loc;

    loc = glGetUniformLocation(program, name);

    if (loc == -1)
        printf("No such uniform named \"%s\"\n", name);

    return loc;
}

void updateShader() {

  glUseProgram(currentShader);

  glUniform1i(getUniLoc(currentShader,"uGpuGenShape")   ,uGpuGenShape);
  glUniform1i(getUniLoc(currentShader,"uInnerShapeType"),uInnerShapeType);
  glUniform1i(getUniLoc(currentShader,"uLocalViewer")   ,uLocalViewer);

  if(options[OPT_SHADER_BUMPMAP] == TRUE || options[OPT_SHADER_DISPMAP] == TRUE)
  {
    glUniform1i(getUniLoc(currentShader,"uNumOfBumps")  ,uNumOfBumps);
    glUniform1i(getUniLoc(currentShader,"uCurrBumpSize"),uCurrBumpSize);
  }
//  glUniform1i(getUniLoc(currentShader,"uTessellation"),uTessellation);

}

void enableVertexArrays()
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
}

void disableVertexArray()
{
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

void generateBuffers()
{
    glGenBuffers(NUM_BUFFERS, buffers);
}

void bufferData()
{
  /* load vertex data into buffer */
  glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTICES]);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(Vec3f) * (geometry.rows + 1) *
              (geometry.cols + 1)), geometry.vertex, GL_STATIC_DRAW);

  /* load vertex data into buffer */
  glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMALS]);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(Vec3f) * (geometry.rows + 1) *
              (geometry.cols + 1)), geometry.normal, GL_STATIC_DRAW);

  /* load indice data into buffer */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
              (sizeof(GLuint) * (geometry.rows) * (geometry.cols) * 4),
               geometry.indice, GL_STATIC_DRAW);
}

void unBindBuffers()
{
  int buffer;

  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer);
  if (buffer != 0)
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER, &buffer);
  if (buffer !=0)
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void drawAxes()
{
  glUseProgram(0);

  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);

  glLineWidth(1.0);
  glBegin(GL_LINES);
  glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(2, 0, 0);
  glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 2, 0);
  glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 2);
  glEnd();
  glPopAttrib();

  if (options[OPT_LIGHTENING])
      glEnable(GL_LIGHTING);

  if(options[OPT_SHADING_TOGG] == TRUE)
    glUseProgram(currentShader);
}

void freeGeometry(Geometry* geo)
{
  // Free allocated data.
  if (geo->vertex)
  {
    free(geo->vertex);
  }
  if (geo->indice)
  {
    free(geo->indice);
  }
  if (geo->normal)
  {
    free(geo->normal);
  }

  geo->vertex = NULL;
  geo->indice = NULL;
  geo->normal = NULL;
  geo->rows   = 0;
  geo->cols   = 0;
  geo->sizeV  = 0;
  geo->sizeI  = 0;
  geo->sizeN  = 0;
}

void allocateArray(Geometry* geo)
{
  if ((geo->vertex = (Vec3f*)malloc(sizeof(Vec3f) *
                  (geo->rows + 1) * (geo->cols + 1))) == NULL)
  {
      fprintf(stderr, "Error! Could not allocate memory.\n");
        exit(1);
  }
  if ((geo->indice = (GLuint*)malloc(sizeof(GLuint) *
                  (geo->rows) * (geo->cols) * 4)) == NULL )
  {
        fprintf(stderr, "Error! Could not allocate memory.\n");
        exit(1);
  }

  if ((geo->normal = (Vec3f*)malloc(sizeof(Vec3f) *
                (geo->rows + 1) * (geo->cols + 1))) == NULL )
  {
      fprintf(stderr, "Error! Could not allocate memory.\n");
      exit(1);
  }

  geo->sizeV = sizeof(Vec3f) *
                (geo->rows + 1) * (geo->cols + 1);

  geo->sizeI = sizeof(GLuint) *
                (geo->rows) * (geo->cols) * 4;

  geo->sizeN = sizeof(Vec3f) *
                (geo->rows + 1) * (geo->cols + 1);
}

void createInnerGeometry(Geometry* geo, int cols, int rows)
{
  float u = 0.0;
  float v = 0.0;

  int i , j, index;
  index = 0;

  for(j = 0; j <= cols; j++)
  {
      u =  1.0 / rows * j ;

    for(i = 0; i <= rows; i++)
    {
      v = 1.0 / rows * i ;
      geo->vertex[index].x = u;
      geo->vertex[index].y = v;
      geo->vertex[index].z = 0;
      index++;
    }
  }
}

/* create the vertices array: vertex */
void createGeometry(int type, Geometry* geo, int cols, int rows)
{
#ifdef DEBUG_GEOMETRY
  fprintf(stderr, "new geometry %i %i\n", cols, rows);
#endif

  geo->rows = rows;
  geo->cols = cols;
  allocateArray(geo);

  float u = 0.0;
  float v = 0.0;

  int i , j, index;

    float gridStep = GRID_LENGTH/cols;
    float start = -1.0;
    index = 0;

    switch (type) {
        case SHAPE_IMM_GRID:
        case SHAPE_GRID:
          for(j = 0; j <= cols; j++)
          {
            for(i = 0; i <= rows ; i++)
            {
              geo->vertex[index].x = start + gridStep * i;
              geo->vertex[index].y = start + gridStep * j;
              geo->vertex[index].z = 0;
              index++;
            }
          }
      break;
      case SHAPE_IMM_SPHERE:
      case SHAPE_SPHERE:
          for(j = 0; j <= cols; j++)
          {
              v =  M_PI * j / rows;
            for(i = 0; i <= rows; i++)
            {
              u = 2.0*M_PI * i / rows;
              geo->vertex[index].x = 1.0 * cos(u) * sin(v);
              geo->vertex[index].y = 1.0 * sin(u) * sin(v);
              geo->vertex[index].z = 1.0 * cos(v);
              index++;
            }
          }
      break;
      case SHAPE_IMM_TORUS:
      case SHAPE_TORUS:
          for(j = 0; j <= cols; j++)
          {
              u =  2.0 * M_PI / rows * j ;
            for(i = 0; i <= rows; i++)
            {
              v = 2.0*M_PI / rows * i ;

              geo->vertex[index].x = (TORUS_EX_RADIUS + TORUS_IN_RADIUS *
                                      cos(u)) * cos(v);
              geo->vertex[index].y = (TORUS_EX_RADIUS + TORUS_IN_RADIUS *
                                      cos(u)) * sin(v);
              geo->vertex[index].z = TORUS_IN_RADIUS * sin(u);
              index++;
            }
          }
      break;
      case SHAPE_INNER_GRID:
          uInnerShapeType = GPU_GRID;
          createInnerGeometry(geo,cols,rows);
      break;
      case SHAPE_INNER_SHPERE:
          uInnerShapeType = GPU_SPHERE;
          createInnerGeometry(geo,cols,rows);
      break;
      case SHAPE_INNER_TORUS:
          uInnerShapeType = GPU_TORUS;
          createInnerGeometry(geo,cols,rows);
      break;

    }
}

//create the vertices array: indice
void createIndices(Geometry* geo, int cols, int rows)
{
  int i , j, index;
  index = 0;

  for(j = 0; j < cols; j++)
  {
    for(i = 0; i < rows ; i++)
    {
      geo->indice[index++] = j * (cols + 1)  + i;
      geo->indice[index++] = (j + 1) * (cols + 1) + i;
      geo->indice[index++] = (j + 1) * (cols + 1) + (i + 1);
      geo->indice[index++] = j * (cols + 1)  + (i + 1);
    }
  }
}


void calculateGridNormal(Geometry* geo)
{
  int index;
  Vec3f a = geo->vertex[0];
  Vec3f b = geo->vertex[1];
  Vec3f c = geo->vertex[1 + geo->rows];
  Vec3f va;
  Vec3f vb;
  Vec3f normal;

  float len;
  index = 0;

  va.x = b.x - a.x;
  va.y = b.y - a.y;
  va.z = b.z - a.z;

  vb.x = c.x - a.x;
  vb.y = c.y - a.y;
  vb.z = c.z - a.z;

  normal.x = ((va.y * vb.z) - (va.z * vb.y));
  normal.y = -((va.z * vb.x) - (va.x * vb.z));
  normal.z = ((va.x * vb.y) - (va.y * vb.x));

  len = (float)(sqrt((normal.x * normal.x) + (normal.y * normal.y) +
                     (normal.z * normal.z)));
  normal.x /= len;
  normal.y /= len;
  normal.z /= len;

  for (int i = 0; i <= geo->rows; ++i)
  {
    for (int j = 0; j <= geo->cols; ++j)
    {
      geo->normal[index].x = normal.x;
      geo->normal[index].y = normal.y;
      geo->normal[index].z = normal.z;
      index++;
    }
  }
}

/* create the vertices array: normal */
void createNormals(int type, Geometry* geo, int cols, int rows)
{
  geo->rows = rows;
  geo->cols = cols;

  float u = 0.0;
  float v = 0.0;
  int i , j, index;

    index = 0;

    switch (type) {
        case SHAPE_GRID:
        case SHAPE_IMM_GRID:
            calculateGridNormal(geo);
        break;
        case SHAPE_SPHERE:
        case SHAPE_IMM_SPHERE:
            for(j = 0; j <= cols; j++)
            {
                v =  M_PI * j / rows;
              for(i = 0; i <= rows; i++)
              {
                u = 2.0*M_PI * i / rows;
                geo->normal[index].x = 1.0 * cos(u) * sin(v);
                geo->normal[index].y = 1.0 * sin(u) * sin(v);
                geo->normal[index].z = 1.0 * cos(v);
                index++;
              }
            }
        break;
        case SHAPE_TORUS:
        case SHAPE_IMM_TORUS:
            for(j = 0; j <= cols; j++)
            {
                u =  2.0 * M_PI / rows * j ;
              for(i = 0; i <= rows; i++)
              {
                v = 2.0*M_PI / rows * i ;
                geo->normal[index].x = cos(u) * cos(v);
                geo->normal[index].y = cos(u) * sin(v);
                geo->normal[index].z = sin(u);
                index++;
              }
            }
        break;
    }
}

void cleanup()
{
    freeGeometry(&geometry);
}

void createShape(int type)
{
    cleanup();
    createGeometry(type, &geometry, uTessellation, uTessellation);
    createIndices(&geometry, uTessellation, uTessellation);
    createNormals(type, &geometry, uTessellation, uTessellation);
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    memset(options, 0, sizeof(bool) * OPTSIZE);
    options[OPT_STDOUT_TEXT]    = TRUE;
    options[OPT_OSP_SHORT_LONG] = TRUE;
    uTessellation = WELL_FORM_TESS;

    /* set starting default setting */
    currentShape = SHAPE_GRID;
    sprintf(tessString,    "Tessellation: %d x %d",uTessellation,uTessellation);
    sprintf(bumpSizeString,"Number Of Bumps: %d x %d",uNumOfBumps,uNumOfBumps);
    strcpy(localViewString,"LocalView: Off ");
    strcpy(infoString,     "Shader: Fixed pipeline");
    strcpy(drawMethodString,currentDrawingMethod[currentShape][0]);

    createShape(currentShape);
    generateBuffers();
    enableVertexArrays();
    generateBuffers();
    bufferData();
}

void drawImmediateShape(Geometry* geo)
{
  int i, j;
  int index = 0;
  int step  = 1 + geo->cols;

  for ( i = 0; i < geo->rows; i++ )
  {
    glBegin(GL_QUAD_STRIP);
    for (j = 0; j <= geo-> cols; j++ )
    {
      glNormal3f(geo->normal[index].x, geo->normal[index].y,
                 geo->normal[index].z);
      glVertex3f(geo->vertex[index].x, geo->vertex[index].y,
                 geo->vertex[index].z);

      glNormal3f(geo->normal[index+step].x, geo->normal[index+step].y,
                 geo->normal[index+step].z);
      glVertex3f(geo->vertex[index+step].x, geo->vertex[index+step].y,
                 geo->vertex[index+step].z);
      index++;
    }
    glEnd();
  }
}

void drawScene()
{
  if (options[OPT_LIGHTENING])
      glDisable(GL_LIGHTING);

  drawAxes();

  if (options[OPT_LIGHTENING])
      glEnable(GL_LIGHTING);

  /* update buffer */
  if (updateBuffer == TRUE) {

    bufferData();
    glBindBuffer(GL_ARRAY_BUFFER,buffers[NORMALS]);
    glNormalPointer(GL_FLOAT, 0 , 0);

    glBindBuffer(GL_ARRAY_BUFFER,buffers[VERTICES]);
    glVertexPointer(3, GL_FLOAT, 0 , 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[INDICES]);
    updateBuffer = FALSE;
  }

  /* select drawing method */
  switch (currentShape)
  {
    case SHAPE_IMM_GRID:
    case SHAPE_IMM_SPHERE:
    case SHAPE_IMM_TORUS:
         drawImmediateShape(&geometry);
    break;
    case SHAPE_GRID:
    case SHAPE_SPHERE:
    case SHAPE_TORUS:
    case SHAPE_INNER_GRID:
    case SHAPE_INNER_SHPERE:
    case SHAPE_INNER_TORUS:
         glDrawElements(GL_QUADS, geometry.rows * geometry.cols * 4,
         GL_UNSIGNED_INT, 0);
    break;
  }
}

void drawNormal()
{
    if (options[OPT_DRAW_NORMAL])
    {
        glBegin(GL_LINES);

        int index = 0;
        for (int i = 0; i <= geometry.rows; ++i)
        {
          for (int j = 0; j <= geometry.cols; ++j)
          {
            glVertex3f(geometry.vertex[index].x,
            geometry.vertex[index].y, geometry.vertex[index].z);

            glVertex3f(geometry.vertex[index].x +
                      geometry.normal[index].x * 0.1f,

            geometry.vertex[index].y + geometry.normal[index].y * 0.1f,
            geometry.vertex[index].z + geometry.normal[index].z * 0.1f);
            index++;
          }
        }
        glEnd();
    }
}

void drawString(char *string, float x, float y)
{
  char *c;
  glRasterPos2f(x, y);
  for (c=string; *c != '\0'; c++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10 , *c);
}

void drawHud()
{
  glUseProgram(0);
  glColor3f(1.0, 1.0, 1.0);

  if (options[OPT_STDOUT_TEXT]) {
      if (options[OPT_OSP_SHORT_LONG]) {
          drawString(bumpSizeString  ,0.02,0.16);
          drawString(tessString      ,0.02,0.13);
          drawString(drawMethodString,0.02,0.10);
          drawString(localViewString ,0.02,0.07);
          drawString(infoString      ,0.02,0.04);
      }
      drawString(frameRateString, 0.02, 0.01);
  }
  else {
      if (options[OPT_OSP_SHORT_LONG]) {
          printf("%s\n",bumpSizeString);
          printf("%s\n",tessString);
          printf("%s\n",drawMethodString);
          printf("%s\n",localViewString);
          printf("%s\n",infoString);
      }
      printf("%s\n",frameRateString);
  }
  glColor3f(1.0, 0.5, 0.0);
  glUseProgram(currentShader);
}

void display()
{
  static const float ambient [] = {0.5, 0.5, 0.5, 1.0};
  static const float diffuse [] = {1.0, 0.0, 0.0, 1.0};
  static const float specular[] = {1.0, 1.0, 1.0, 1.0};
  static const float lightPos[] = {2.0, 2.0, 2.0, 0.0};
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  /* camera transform */
  glTranslatef(0.0, 0.0, -zoom);     /* zoom transform */
  glRotatef(camRotX, 1.0, 0.0, 0.0); /* rotation around x axis */
  glRotatef(camRotY, 0.0, 1.0, 0.0); /* rotation around y axis */

  /* set the light position */
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT, GL_SHININESS, shininess);

  /* update uniform variable */
  if (options[OPT_SHADING_TOGG])
      updateShader();

  /* draw the scene */
  drawScene();

  /* draw normals */
  drawNormal();

  /* Render the hud */
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,1,0,1,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (options[OPT_LIGHTENING])
      glDisable(GL_LIGHTING);
  /* display current setting */
  glTranslatef(0.0, 0.0, -0.5);
  drawHud();

  if (options[OPT_LIGHTENING])
      glEnable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);

  glutSwapBuffers();

  // Check for gl errors.
  checkGLErrors();
}

void idle()
{
  long thisTime;
  float deltaTime;
  /* bump var */
  static int increaseBump = TRUE;
  float bump_min = 0;
  float bump_max = 50;

  /* calculate frame time */
  thisTime  = glutGet(GLUT_ELAPSED_TIME);
  deltaTime = (thisTime - lastTime) * 0.001;
  lastTime  = thisTime;

  frameCount++;

  if ((thisTime - lastCalTime) > 1000)
  {
    float frameRate = frameCount/ ((thisTime - lastCalTime) / 1000.0f);

      lastCalTime = thisTime;
      frameCount  = 0;
      sprintf(frameRateString, "Frame rate (fps):     %.2f", frameRate);
  }

  /* bump animation */
  if ((options[OPT_SHADER_DISPMAP]   == TRUE   ||
       options[OPT_SHADER_BUMPMAP]   == TRUE)  &&
       options[OPT_SHADER_ANIMATION] == TRUE)  {

    if (uCurrBumpSize >= bump_min && increaseBump == TRUE) {
        if (uCurrBumpSize == bump_max)
        {
          increaseBump = FALSE;
        }
        else {
            uCurrBumpSize++;
        }
    }
    else if (uCurrBumpSize <= bump_max && increaseBump == FALSE) {
             if (uCurrBumpSize == bump_min) {
                 increaseBump = TRUE;
            }
            else {
                uCurrBumpSize--;
            }
    }
  }

  /* notify glut that the scene needs to be redrawn */
  glutPostRedisplay();
}

void reshape(int x, int y)
{
  float aspect;
  if (y == 0) y = 1;
  aspect = x/(float)y;

  /* set area on screen to draw to */
  glViewport(0, 0, x, y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  /* set perspective projection */
  gluPerspective(75.0, aspect, 0.1, 100);
  /* set back to modelview */
  glMatrixMode(GL_MODELVIEW);
}

void updateRandering()
{
  if ((options[OPT_SHADER_GEN_SHAPE] == TRUE  &&
       options[OPT_SHADING_TOGG]     == TRUE) ||
      (options[OPT_SHADER_BUMPMAP]   == TRUE) ||
      (options[OPT_SHADER_DISPMAP]   == TRUE))
  {
    uGpuGenShape = true;
    createShape(currentShape);
  }
  else
  {
      createShape(currentShape);
      uGpuGenShape = false;
  }
    updateBuffer = TRUE;
}

void keyDown(unsigned char key, int x, int y)
{
  switch(key)
  {
      // Exit if esc or q is pressed
      case 27:
      case 'q':
            glutDestroyWindow(win);
            cleanup();
            exit(0);
      break;

      /* change shape */
      case 'o':
            if (options[OPT_SHADER_GEN_SHAPE] == FALSE) {
                currentShape = (currentShape + 1) % 6;
           }
            else {
                currentShape = ((currentShape + 1) % 3) + 6;
            }
            strcpy(drawMethodString, currentDrawingMethod[currentShape][0]);
            updateBuffer = TRUE;
            updateRandering();

      break;

      /* increase tessellation */
      case 't':
      case 'T':
          if (key == 'T') {
              uTessellation = min(GEOMETRY_BASE_MAX, uTessellation * 2);
              updateRandering();
              updateBuffer = TRUE;
          }
          else {
              uTessellation = max(GEOMETRY_BASE, uTessellation / 2);
              updateRandering();
              updateBuffer = TRUE;
          }
          sprintf(tessString,"Tessellation: %d x %d",uTessellation,uTessellation);
      break;

      /* toggle light */
      case 'l':
            options[OPT_LIGHTENING] = !options[OPT_LIGHTENING];
          if  (options[OPT_LIGHTENING])
              glEnable(GL_LIGHTING);
          else
              glDisable(GL_LIGHTING);
      break;

      /* display normal */
      case 'n':
            if (options[OPT_SHADER_GEN_SHAPE]   == FALSE &&
                options[OPT_SHADER_BUMPMAP] == FALSE &&
                options[OPT_SHADER_DISPMAP] == FALSE ) {

                options[OPT_DRAW_NORMAL] = !options[OPT_DRAW_NORMAL];
            }
      break;

      /* toggle flat and smooth shading */
      case 'f':
            options[OPT_SHADING_SMOOTH_FLAT] =
                               !options[OPT_SHADING_SMOOTH_FLAT];
            if (options[OPT_SHADING_SMOOTH_FLAT])
                glShadeModel(GL_FLAT);
            else
                glShadeModel(GL_SMOOTH);
      break;

      /* increase and decrease specular */
      case 'h':
      case 'H':
          if (key == 'H')
              shininess += 1.0;
          else
              shininess -= 1.0;
              shininess = fmin(fmax(shininess, 0.0), SHININESS_MAX);
      break;

      /* local view toggle */
      case 'v':
            options[OPT_LOCAL_VIEWER] = !options[OPT_LOCAL_VIEWER];

            if (options[OPT_LOCAL_VIEWER] == TRUE) {
                uLocalViewer = true;
                glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
                strcpy(localViewString,"LocalView:  ON");
            }
            else {
                uLocalViewer = false;
                glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
                strcpy(localViewString,"LocalView: OFF");
            }
      break;

      /* display all settings or only frame rate */
      case 'x':
      case 'X':
           options[OPT_OSP_SHORT_LONG] = !options[OPT_OSP_SHORT_LONG];
      break;

      case 'z':
      case 'Z':
            options[OPT_STDOUT_TEXT] = !options[OPT_STDOUT_TEXT];
      break;

      /* increase and decrease bumps */
      case 'r':
      case 'R':
           if (key == 'R')
               uNumOfBumps = min(BUMPS_BASE_MAX, uNumOfBumps * 2);
           else
               uNumOfBumps = max(BUMPS_BASE, uNumOfBumps / 2);
           sprintf(bumpSizeString,"Number Of Bumps: %d x %d",uNumOfBumps,uNumOfBumps);
      break;

      /* switch between per-vertex and per-pixel lighting */
      case 'p':
          if (options[OPT_SHADING_TOGG]   == TRUE  &&
              options[OPT_SHADER_BUMPMAP] == FALSE &&
              options[OPT_SHADER_DISPMAP] == FALSE) {

              options[OPT_SHADING_VERTEX_PIXEL] =
                                 !options[OPT_SHADING_VERTEX_PIXEL];

              /* per vextex blinn phong and phong */
              if (options[OPT_SHADING_VERTEX_PIXEL] == TRUE) {
                 if (options[OPT_SHADING_BLINN_PHONG] == TRUE) {
                     strcpy(infoString, "Shader: per-vertex blinn phong ");
                     currentShader = shaderProgramHandles[SHADER_VERTEX_BLINN];
                 }
                 else {
                     strcpy(infoString, "Shader: per-vertex phong ");
                     currentShader = shaderProgramHandles[SHADER_VERTEX_PHONG];
                 }
              }

              /* per pixel blinn phong and phong */
              else if (options[OPT_SHADING_VERTEX_PIXEL] == FALSE) {

                  if (options[OPT_SHADING_BLINN_PHONG] == TRUE) {
                      strcpy(infoString, "Shader: per-pixel blinn phong ");
                      currentShader = shaderProgramHandles[SHADER_PIXEL_BLINN];
                  }
                  else {
                      strcpy(infoString, "Shader: per-pixel phong ");
                      currentShader = shaderProgramHandles[SHADER_PIXEL_PHONG];
                  }
              }
          }
      break;
      /* switch between blinn-phong and phong lighting model*/
      case 'm':
          if (options[OPT_SHADING_TOGG]   == TRUE  &&
              options[OPT_SHADER_BUMPMAP] == FALSE &&
              options[OPT_SHADER_DISPMAP] == FALSE) {

              options[OPT_SHADING_BLINN_PHONG] =
                                 !options[OPT_SHADING_BLINN_PHONG];

              /* blinn phong lighting model */
              if( options[OPT_SHADING_BLINN_PHONG] == TRUE) {
                if (options[OPT_SHADING_VERTEX_PIXEL] == TRUE) {
                    strcpy(infoString, "Shader: per-vertex blinn phong ");
                    currentShader = shaderProgramHandles[SHADER_VERTEX_BLINN];
                 }
                else {
                    strcpy(infoString, "Shader: per-pixel blinn phong ");
                    currentShader = shaderProgramHandles[SHADER_PIXEL_BLINN];
                }
              }
              /* phong lighting model */
              else if(options[OPT_SHADING_BLINN_PHONG] == FALSE) {
                if (options[OPT_SHADING_VERTEX_PIXEL] == TRUE) {
                    strcpy(infoString, "Shader: per-vertex phong ");
                    currentShader = shaderProgramHandles[SHADER_VERTEX_PHONG];
                  }
                else {
                    strcpy(infoString, "Shader: per-pixel phong ");
                    currentShader = shaderProgramHandles[SHADER_PIXEL_PHONG];
                }
              }
          }
      break;

      /* switch between gpu and cpu shape generation */
      case 'g':
          if (options[OPT_SHADING_TOGG]   == TRUE  &&
              options[OPT_SHADER_BUMPMAP] == FALSE &&
              options[OPT_SHADER_DISPMAP] == FALSE) {

              options[OPT_SHADER_GEN_SHAPE] = !options[OPT_SHADER_GEN_SHAPE];

              if (options[OPT_SHADER_GEN_SHAPE] == TRUE ) {

                  options[OPT_DRAW_NORMAL] = FALSE;
                  currentShape = SHAPE_INNER_GRID;
                  strcpy(drawMethodString,currentDrawingMethod[currentShape][0]);
                  uGpuGenShape  = true;
                  updateRandering();
              }
              else {

                  uGpuGenShape  = false;
                  currentShape = SHAPE_GRID;
                  strcpy(drawMethodString, currentDrawingMethod[currentShape][0]);
                  updateRandering();
              }
          }
      break;

      /* toggle bump-map */
      case 'b':
            options[OPT_SHADER_BUMPMAP] = !options[OPT_SHADER_BUMPMAP];

          if (options[OPT_SHADER_BUMPMAP] == TRUE) {
              /* set default setting */
              currentShape = SHAPE_INNER_GRID;

              options[OPT_DRAW_NORMAL]      = FALSE;
              options[OPT_SHADER_DISPMAP]   = FALSE;
              options[OPT_SHADER_GEN_SHAPE] = TRUE;
              options[OPT_SHADING_TOGG]     = TRUE;
              currentShader = shaderProgramHandles[SHADER_BUMP_MAP];

              /* update display settings */
            strcpy(infoString,"Shader: bump-map using per-vertex blinn phong ");
            strcpy(drawMethodString, currentDrawingMethod[currentShape][0]);

            updateRandering();
          }
          else {
              options[OPT_SHADER_BUMPMAP]   = FALSE;
              options[OPT_SHADER_GEN_SHAPE] = FALSE;

              /* set default setting */
              currentShape = SHAPE_GRID;
              currentShader = shaderProgramHandles[SHADER_VERTEX_BLINN];

              /* update display settings */
              strcpy(infoString,"Shader: per-vertex blinn phong ");
              strcpy(drawMethodString, currentDrawingMethod[currentShape][0]);

              glUseProgram(currentShader);
              updateRandering();
          }
          uCurrBumpSize = MAX_BUMP_SIZE;
      break;

      /* toggle bump-map */
      case 'd':
            options[OPT_SHADER_DISPMAP] = !options[OPT_SHADER_DISPMAP];

          if (options[OPT_SHADER_DISPMAP] == TRUE) {

              options[OPT_DRAW_NORMAL]      = FALSE;
              options[OPT_SHADER_BUMPMAP]   = FALSE;
              options[OPT_SHADER_GEN_SHAPE] = TRUE;
              options[OPT_SHADING_TOGG]     = TRUE;

              currentShape = SHAPE_INNER_GRID;
              currentShader = shaderProgramHandles[SHADER_DISP_MAP];

              /* update display settings */
              strcpy(infoString,"Shader: displace-map using per-vertex phong ");
              strcpy(drawMethodString, currentDrawingMethod[currentShape][0]);

              updateRandering();
          }
          else {
              /* restore previous setting */
              options[OPT_SHADER_DISPMAP]   = FALSE;
              options[OPT_SHADER_GEN_SHAPE] = FALSE;
              currentShader = shaderProgramHandles[SHADER_VERTEX_BLINN];

              currentShape = SHAPE_GRID;

              glUseProgram(currentShader);
              strcpy(infoString,"Shader: using per-vertex blinn phong ");
              strcpy(drawMethodString, currentDrawingMethod[currentShape][0]);

              updateRandering();
          }
          uCurrBumpSize = MAX_BUMP_SIZE;
      break;

      /* start stop animation  */
      case 'a':
          if (options[OPT_SHADER_DISPMAP] == TRUE ||
              options[OPT_SHADER_BUMPMAP] == TRUE ) {

              options[OPT_SHADER_ANIMATION] =! options[OPT_SHADER_ANIMATION];
              if (options[OPT_SHADER_ANIMATION]== FALSE )
              {
                 uCurrBumpSize = MAX_BUMP_SIZE;
              }
          }
      break;

      /* toggle fixed pipeline and user define shader */
      case 's':

          if (options[OPT_SHADER_BUMPMAP] == FALSE &&
              options[OPT_SHADER_DISPMAP] == FALSE) {

              options[OPT_SHADING_TOGG] = !options[OPT_SHADING_TOGG];

            if (!options[OPT_SHADING_TOGG]) {
                //currentShader = 0;
                strcpy(infoString, "Shader: Fixed pipeline");
                options[OPT_SHADER_GEN_SHAPE] = FALSE;
                uGpuGenShape  = false;
                currentShape = SHAPE_GRID;
                glUseProgram(0);
                updateRandering();
            }
            else {
                strcpy(infoString, "Shader: per-vertex blinn phong ");

                glUseProgram(currentShader);
                updateRandering();
            }
          }
      break;
      default:
      break;
  }
}

void mouseDown(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON)
    buttonRotate = state == GLUT_DOWN;
  if (button == GLUT_RIGHT_BUTTON)
    buttonZoom = state == GLUT_DOWN;
}

void mouseMove(int x, int y)
{
  /* get change in mouse position */
  int dx = x - lastMouseX;
  int dy = y - lastMouseY;
  if (buttonRotate)
  {
    /* rotate camera based on mouse movement */
    camRotX += dy * mouseSensitivity;
    camRotY += dx * mouseSensitivity;
    if (camRotY > 180.0) camRotY -= 360.0;
    if (camRotY <= -180.0) camRotY += 360.0;
    if (camRotX > 90.0) camRotX = 90.0;
    if (camRotX < -90.0) camRotX = -90.0;

  }
  if (buttonZoom)
  {
    /* zoom in and out based on y mouse movement */
    zoom += dy * zoom * mouseSensitivity * 0.1f;
    if (zoom < 0.1) zoom = 0.1;
    if (zoom > 100.0) zoom = 100.0;
  }

  /* update last mouse position */
  lastMouseX = x;
  lastMouseY = y;
}

int main(int argc, char** argv)
{
  /* initialize glut and create a window */
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(500, 500);
  win = glutCreateWindow("3D Real-Time Rendering Assignment 1");

  /* set up the glut callbacks */
  glutIdleFunc(idle);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyDown);
  glutMouseFunc(mouseDown);
  glutMotionFunc(mouseMove);
  glutPassiveMotionFunc(mouseMove);
  glDepthFunc(GL_LESS);

  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glDepthMask(GL_TRUE);
  // Initialize variables, create scene etc.

  /* create our shader */
  glewInit();
  init();
  initShaderState();
  useBufferObjects = TRUE;

  /* initialize lastTime and enter main loop */
  lastTime = glutGet(GLUT_ELAPSED_TIME);
  glutMainLoop();

  return 0;
}
