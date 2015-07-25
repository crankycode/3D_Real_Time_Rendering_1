
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

GLint loc1,loc2,loc3,loc4;


/* Added Structure Define */
typedef struct Vec3f {
	GLfloat x, y, z;
} Vec3f;

typedef struct Vec2f {
   float u, v;
} Vec2f;

typedef struct Geometry {
	Vec3f* vertex;
	GLuint* indice;
	Vec3f* normal;
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
#define GEOMETRY_BASE_MAX 10000
#define SHININESS_MAX     100.0

#define NUM_BUFFERS       3
#define VERTICES          0
#define INDICES           1
#define NORMALS           2
#define NUM_SHADERS       6
#define TRUE  1
#define FALSE 0
//#define COLORS 1

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
	OPTSIZE //NOTE: this must be the last enumerator.
};

enum shaper {
	SHAPE_GRID,
	SHAPE_SPHERE,
  SHAPE_TORUS,
  SHAPE_INNER_GRID,
  SHAPE_INNER_SHPERE,
  SHAPE_INNER_TORUS,
  SHAPE_IMM_GRID,
  SHAPE_IMM_SPHERE,
  SHAPE_IMM_TORUS
};	

static bool options[OPTSIZE];
static Geometry geometry;

static int currentShape;
static int uTessellation;

float shininess = 50.0;

int win;
long lastTime;

int lastCalTime = 0;
int frameCount  = 0;

/* shading variables */
GLuint torusShader;
GLuint currentShader = 0;
GLint  sdBlinnOrPhong ,sdVertexOrPixel;
GLuint buffers[NUM_BUFFERS];

/* uniform variables */
bool uGpuGenShape    = true;
int  uInnerShapeType;

int bPerPixel      = 0;
int bFixedPipeline = 1;
int bBlinnPhong    = 1;
int numOfBump      = 8;

/* frame rate variable */
char frameRateString[256];
char infoString     [256];

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

/* constants for shaders */
enum Shaders { SHADER_VERTEX_BLINN, SHADER_PIXEL_BLINN, SHADER_VERTEX_PHONG,
               SHADER_PIXEL_PHONG, SHADER_BUMP_MAP, SHADER_DISP_MAP };

/* file names for shaders */
static const char *shaderProgramFileNames[NUM_SHADERS][2] = 
{  
   {"BlinnPhongPerVertex.vert", "BlinnPhongPerVertex.frag"},
   {"BlinnPhongPerPixel.vert", "BlinnPhongPerPixel.frag"}, 
   {"PhongPerVertex.vert", "PhongPerVertex.frag"}, 
   {"PhongPerPixel.vert", "PhongPerPixel.frag"}, 
   {"BumpMap.vert", "BumpMap.frag"},
   {"DispMap.vert", "DispMap.frag"}
};

/* Added Functions */
#define checkGLErrors() _checkGLErrors(__LINE__)
void _checkGLErrors(int line)
{
	GLuint err;
	if ((err = glGetError()) != GL_NO_ERROR)
		fprintf(stderr, "OpenGL error caught on line %i: %s\n", line, gluErrorString(err));
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
  
  glUniform1i(getUniLoc(currentShader,"uGpuGenShape"),uGpuGenShape);
  glUniform1i(getUniLoc(currentShader,"uTessellation"),uTessellation);
  glUniform1i(getUniLoc(currentShader,"uInnerShapeType"),uInnerShapeType);
    
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
  if(buffer != 0)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER, &buffer);
  if(buffer !=0)
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void drawAxes()
{
	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
	glLineWidth(1.0);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(2, 0, 0);
	glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 2, 0);
	glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 2);
	glEnd();
	glPopAttrib();
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
	geo->rows = 0;
	geo->cols = 0;
	geo->sizeV = 0;
	geo->sizeI = 0;
	geo->sizeN = 0;
}

void allocateArray(Geometry* geo)
{
	if ( (geo->vertex = (Vec3f*)malloc(sizeof(Vec3f) * 
	                (geo->rows + 1) * (geo->cols + 1))) == NULL)
	{
	    fprintf(stderr, "Error! Could not allocate memory.\n");
        exit(1); 
	} 
	if ( (geo->indice = (GLuint*)malloc(sizeof(GLuint) * 
	                (geo->rows) * (geo->cols) * 4)) == NULL )
	{
        fprintf(stderr, "Error! Could not allocate memory.\n");
        exit(1); 
  }
    
  if ( (geo->normal = (Vec3f*)malloc(sizeof(Vec3f) * 
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

//create the vertices array: vertex
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
        case SHAPE_GRID:        printf("outer gen\n");
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
	    case SHAPE_TORUS:
          for(j = 0; j <= cols; j++)
          {
              u =  2.0 * M_PI / rows * j ;	
            for(i = 0; i <= rows; i++)
            {
              v = 2.0*M_PI / rows * i ;
                
              geo->vertex[index].x = (TORUS_EX_RADIUS + TORUS_IN_RADIUS * cos(u)) * cos(v);
              geo->vertex[index].y = (TORUS_EX_RADIUS + TORUS_IN_RADIUS * cos(u)) * sin(v);
              geo->vertex[index].z = TORUS_IN_RADIUS * sin(u);
              index++;
            }
          }
	    break;
      case SHAPE_INNER_GRID:
printf("SHAPE_INNER_GRID gen\n");
          uInnerShapeType = SHAPE_GRID;
          for(j = 0; j <= cols; j++)
	        {
		        for(i = 0; i <= rows ; i++)
		        {
              geo->vertex[index].x = i;  geo->vertex[index].y = j;
              geo->vertex[index].z = 0;
              index++;
		        }
	        }
      break;
      case SHAPE_INNER_SHPERE:
          uInnerShapeType = SHAPE_SPHERE;
          createInnerGeometry(geo,cols,rows);
      break;
      case SHAPE_INNER_TORUS:
          uInnerShapeType = SHAPE_TORUS;
          createInnerGeometry(geo,cols,rows);
      break;
      case SHAPE_IMM_GRID:
      break;
      case SHAPE_IMM_SPHERE:
      break;
      case SHAPE_IMM_TORUS:
      break;
      
    }    
/*    
    //Print vetex
    for (i = 0; i < (rows+1) *(cols+1); i++)
	{
	    printf("%d  x: %f ,y: %f, z: %f\n", i, geo->vertex[i].x, 
            geo->vertex[i].y,
            geo->vertex[i].z );
	}
*/
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
/*	
	//Print indices
	for (i = 0; i < cols * rows; i++)
	{
	    printf("%d  in: %d\n", i,
            geo->indice[i] );
	} 
*/
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

  len = (float)(sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z))) ;
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

//create the vertices array: normal
void createNormals(int type, Geometry* geo, int cols, int rows)
{
  geo->rows = rows;
	geo->cols = cols;	
	
	float u = 0.0;
	float v = 0.0;
	int i , j, index;
			
    index = 0;
    Vec3f normal;

    switch (type) {
        case SHAPE_GRID:            
            calculateGridNormal(geo);
 	        break;	
	    case SHAPE_SPHERE:
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
    
    /*for (i = 0; i < (rows+1) *(cols+1); i++)
	{
	    printf("%d  x: %f ,y: %f, z: %f\n", i, geo->normal[i].x, 
            geo->normal[i].y,
            geo->normal[i].z );
	}*/
}

void createShape(int type)
{
    freeGeometry(&geometry);
    createGeometry(type, &geometry, uTessellation, uTessellation);
    createIndices(&geometry, uTessellation, uTessellation);
    createNormals(type, &geometry, uTessellation, uTessellation);
}


void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    memset(options, 0, sizeof(bool) * OPTSIZE);

    options[OPT_STDOUT_TEXT] = 1;
    
    currentShape = SHAPE_GRID;
//    tessellation = GEOMETRY_BASE;
      uTessellation = 64;
  
    createShape(currentShape);
    generateBuffers();
    enableVertexArrays();
    generateBuffers();
    bufferData();
}

void drawScene()
{

	// f - (smooth,flat) shading
	if (options[OPT_SHADING_SMOOTH_FLAT])	
	    glShadeModel(GL_FLAT);   
	else 
	    glShadeModel(GL_SMOOTH); 
  
	//  v - local viewer 
  if (options[OPT_LOCAL_VIEWER])
	    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	else
	    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);   


	drawAxes();

  if(updateBuffer == TRUE) {
    bufferData();
    glBindBuffer(GL_ARRAY_BUFFER,buffers[NORMALS]);
    glNormalPointer(GL_FLOAT, 0 , 0);
    
    glBindBuffer(GL_ARRAY_BUFFER,buffers[VERTICES]);
    glVertexPointer(3, GL_FLOAT, 0 , 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[INDICES]);
    updateBuffer = FALSE;
  }
 // if(bumpMap == FALSE) {
    glDrawElements(GL_QUADS, geometry.rows * geometry.cols * 4, GL_UNSIGNED_INT, 0);

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
            glVertex3f(geometry.vertex[index].x + geometry.normal[index].x * 0.1f, 
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
	glColor3f(1.0, 0.5, 0.0);
	if (options[OPT_STDOUT_TEXT])
	{
	    drawString(infoString, 0.02, 0.04);
	    drawString(frameRateString, 0.02, 0.01);
	}
}

void display()
{
	static const float lightPos[] = {2.0, 2.0, 2.0, 0.0};
	static const float ambient[] = {0.5, 0.5, 0.5, 1.0};
	static const float diffuse[] = {1.0, 0.0, 0.0, 1.0};
	static const float specular[] = {1.0, 1.0, 1.0, 1.0};
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	/* camera transform */
	glTranslatef(0.0, 0.0, -zoom); /* zoom transform */
	glRotatef(camRotX, 1.0, 0.0, 0.0); /* rotation around x axis */
	glRotatef(camRotY, 0.0, 1.0, 0.0); /* rotation around y axis */
	
	/* set the light position */
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
	if(options[OPT_SHADING_TOGG])
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
	
	//glUseProgram(0);
	
//	if (options[OPT_LIGHTENING])
//	    glDisable(GL_LIGHTING);
	glTranslatef(0.0, 0.0, -0.5);
	drawHud();
//	if (options[OPT_LIGHTENING])
//	    glEnable(GL_LIGHTING);
	
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
	GLint loc;
	long thisTime;
	float deltaTime;
	
	/* calculate frame time */
	thisTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = (thisTime - lastTime) * 0.001;
	lastTime = thisTime;
	
    frameCount++;
    
    if ((thisTime - lastCalTime) > 1000)
    {
        float frameRate = frameCount/ ((thisTime - lastCalTime) / 1000.0f);
        
        lastCalTime = thisTime;
        frameCount = 0;
        sprintf(frameRateString, "frame rate (fps):     %.2f", frameRate);
    }
    
	/* pass in the current time to the shader */
	//if (shader != 0 && (loc = glGetUniformLocation(shader, "time")) != -1)
		//glUniform1f(loc, thisTime * 0.001f);
	
	/* notify glut that the scene needs to be redrawn */
	glutPostRedisplay();
	
   /*int now_time;
   static int last_time = 0;
   float elapsed_time;

   frameCount++;
   now_time = glutGet(GLUT_ELAPSED_TIME);

   //converts to seconds 
   elapsed_time = (now_time - last_time) * 0.001;
   last_time = now_time;
   elapsed_time *= startTimeRate;   
   saturnRot += 360*elapsed_time;
 
   if ((now_time - lastCalTime) > 1000)
   {
      
      float frameRate = frameCount/ ((now_time - lastCalTime) / 1000.0f);
      float frameTime = (float)(now_time - lastCalTime)/frameCount;
      int particles = rings.rows*(rings.cols-1);

      lastCalTime = now_time;
      frameCount = 0;
      sprintf(buffer[0], "frame rate (fps):     %.2f", frameRate);
      sprintf(buffer[1], "frame time (ms):      %.2f", frameTime);
   }*/
   
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
	glMatrixMode(GL_MODELVIEW); /* don't forget to go back to modelview */
}


void cleanup()
{
    freeGeometry(&geometry);
}

void updateRandering()
{
  if(options[OPT_SHADER_GEN_SHAPE] == TRUE && options[OPT_SHADING_TOGG] == TRUE) 
  { 
    uGpuGenShape = true;
    printf("shape no:%d\n",currentShape+3);
    createShape(currentShape+3);

  }
  else 
  {
    printf("shape no:%d\n",currentShape);
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
	          currentShape = (currentShape + 1) % 3;
            if(options[OPT_SHADER_GEN_SHAPE] == TRUE)
             createShape(currentShape+3);
            else
              createShape(currentShape);
            updateBuffer = TRUE;
      break;
		  
      /* increase tessellation */
      case 't':
	    case 'T':
		      if (key == 'T') {
			      uTessellation = min(GEOMETRY_BASE_MAX, uTessellation * 2);
            createShape(currentShape);
            updateBuffer = TRUE;
          }
		      else {
		        uTessellation = max(GEOMETRY_BASE, uTessellation / 2);
            createShape(currentShape);
            updateBuffer = TRUE;
          }
      break;
	    
      /* toggle light */
      case 'l':
            options[OPT_LIGHTENING] = !options[OPT_LIGHTENING]; 
          if(options[OPT_LIGHTENING])
	          glEnable(GL_LIGHTING);
	        else
            glDisable(GL_LIGHTING);           
      break;		    
      
      /* display normal */
      case 'n':
            options[OPT_DRAW_NORMAL] = !options[OPT_DRAW_NORMAL];            
		  break;
	    
      /* toggle flat and smooth shading */
      case 'f':
	          options[OPT_SHADING_SMOOTH_FLAT] = !options[OPT_SHADING_SMOOTH_FLAT];
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
      break;		
	    
      case 'x':
	    case 'X':
      break;
      
      case 'z':
	    case 'Z':
	          options[OPT_STDOUT_TEXT] = !options[OPT_STDOUT_TEXT];
		  break;
      
      case 'p':
            options[OPT_SHADING_VERTEX_PIXEL] = !options[OPT_SHADING_VERTEX_PIXEL];
          if(options[OPT_SHADING_VERTEX_PIXEL] == TRUE) {
             if(options[OPT_SHADING_BLINN_PHONG] == TRUE) {
                                                                                 printf("vertex blinn phong \n");
               currentShader = shaderProgramHandles[SHADER_VERTEX_BLINN];
             }
             else {                                                              printf("vertex  phong \n");
               currentShader = shaderProgramHandles[SHADER_VERTEX_PHONG];
             }
          }
          else if(options[OPT_SHADING_VERTEX_PIXEL] == FALSE) {
             if(options[OPT_SHADING_BLINN_PHONG] == TRUE) {                      printf("pixel blinn phong \n");
               currentShader = shaderProgramHandles[SHADER_PIXEL_BLINN];
             }
             else {                                                              printf("pixel phong \n");
               currentShader = shaderProgramHandles[SHADER_PIXEL_PHONG];
             }
          }
      break;  
      
      case 'm': 
          options[OPT_SHADING_BLINN_PHONG] = !options[OPT_SHADING_BLINN_PHONG];
          if( options[OPT_SHADING_BLINN_PHONG] == TRUE) {
             if(options[OPT_SHADING_VERTEX_PIXEL] == TRUE) {                     printf("vertex blinn phong \n");
               currentShader = shaderProgramHandles[SHADER_VERTEX_BLINN];
             }
             else {                                                              printf("pixel blinn phong \n");
               currentShader = shaderProgramHandles[SHADER_PIXEL_BLINN];
             }
          }
          else if(options[OPT_SHADING_BLINN_PHONG] == FALSE) {
             if(options[OPT_SHADING_VERTEX_PIXEL] == TRUE) {                     printf("vertex phong \n");
               currentShader = shaderProgramHandles[SHADER_VERTEX_PHONG];
             }
             else {                                                              printf("pixel phong \n");
               currentShader = shaderProgramHandles[SHADER_PIXEL_PHONG];
             }
          }
      break;  
      
      case 'g':
          if (options[OPT_SHADING_TOGG] == TRUE) { 
              options[OPT_SHADER_GEN_SHAPE] = !options[OPT_SHADER_GEN_SHAPE];
              if(options[OPT_SHADER_GEN_SHAPE] == TRUE ) {           
                printf("OPT_SHADER_GEN_SHAPE inner generation true\n");
                uGpuGenShape = true;
              }
              else {
                 printf("OPT_SHADER_GEN_SHAPE inner generation false\n");
                 uGpuGenShape = false;
              }
          }
      break;
      
      case 'b':
            options[OPT_SHADER_BUMPMAP] = !options[OPT_SHADER_BUMPMAP];
          if(options[OPT_SHADER_BUMPMAP])
            currentShader = shaderProgramHandles[SHADER_BUMP_MAP];
          else
            currentShader = shaderProgramHandles[SHADER_BUMP_MAP];
      break;
      
      case 's':
           options[OPT_SHADING_TOGG] = !options[OPT_SHADING_TOGG];
          if (!options[OPT_SHADING_TOGG]) {
            //currentShader = 0; 
            strcpy(infoString, "Fixed pipeline");
            glUseProgram(0);
          }
          else {
            strcpy(infoString, "shader");
            glUseProgram(currentShader);
          }
      break;
	    default:
		  break;
	}
  updateRandering();
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
	win = glutCreateWindow("assignment 1");

	/* set up the glut callbacks */
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyDown);
	glutMouseFunc(mouseDown);
	glutMotionFunc(mouseMove);
	glutPassiveMotionFunc(mouseMove);
	glDepthFunc(GL_LESS);

	//glEnable(GL_LIGHTING);
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
