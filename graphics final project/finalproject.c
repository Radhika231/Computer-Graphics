
/*
 *  Radhika Mahadev Paryekar
 *  CSCI 4229/5229
 *  Computer Graphics Project
 *  Student ID Number : 105739854
 *
 *
 *  Key bindings:
 *  Space      To select car
 *  1/2        To select a car
 *  Enter      To enter the game
 *  arrows     To navigate within the scene in perspective mode
 *  +/-        Zoom in and zoom out in perspective mode
 *  PgDn/PgUp  overhead view, PgUp to view the scene from top and pgDn to return to ground, in perspective mode
 *  Z          To increase speed
 *  X	       To decrease speed
 *  R/r        To restart
 *  ESC        Exit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

//  OpenGL with prototypes for glext
#ifdef USEGLEW
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


double i,j;          //  For loop Variables
int s=0;
int e;
int score=0;
int k=0;
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int xh=0;
int ah=0;
int d=5;
int fov=75;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=30.0;   //  Size of world
float Ex = 0.0, Ey = 1.0,Ez=1000; // eye coordinates initially 20 units in front of origin
double lz=-1; //for the eye look ahead
float jump = 3.2; // the strafe should jump by 3.2
int move=1;
int light=1;
int collision=0;
int collisiondistance[7];
int p=0;
int dx=0;
int dy=0;
int dz=0;
int inc       =  10;  // Ball increment
int emission  =   0;  // Emission intensity (%)
int ambient   =  30;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shinyvec[1];    // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light
unsigned int texture[10];
unsigned int sky[7];
int car1,car2,dragon;
int view=0;
int viewx=0;
int opt=0;
float y=0.0;
int xpos=10;
int xvel=1;
int mode =1;
bool keySpecialStates[256];

#define Cos(th) (cos((th)*3.1415927/180))
#define Sin(th) (sin((th)*3.1415927/180))




#define LEN 8192  //  Maximum length of text string
void Print(const char* format , int font, ...)
{
      char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,font);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
     {
	 
    if(font==1)
    {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
    }
    if(font==2)
    {
    	glColor3f(0,0,0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,*ch++);
    }
  }
}

void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

void Fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}

/*
 *  Reverse n bytes
 */
static void Reverse(void* x,const int n)
{
   int k;
   char* ch = (char*)x;
   for (k=0;k<n/2;k++)
   {
      char tmp = ch[k];
      ch[k] = ch[n-1-k];
      ch[n-1-k] = tmp;
   }
}

/*
 *  Load texture from BMP file
 */
unsigned int LoadTexBMP(const char* file)
{
   unsigned int   texture;    // Texture name
   FILE*          f;          // File pointer
   unsigned short magic;      // Image magic
   unsigned int   dx,dy,size; // Image dimensions
   unsigned short nbp,bpp;    // Planes and bits per pixel
   unsigned char* image;      // Image data
   unsigned int   k;          // Counter
   int            max;        // Maximum texture dimensions

   //  Open file
   f = fopen(file,"rb");
   if (!f) Fatal("Cannot open file %s\n",file);
   //  Check image magic
   if (fread(&magic,2,1,f)!=1) Fatal("Cannot read magic from %s\n",file);
   if (magic!=0x4D42 && magic!=0x424D) Fatal("Image magic not BMP in %s\n",file);
   //  Seek to and read header
   if (fseek(f,16,SEEK_CUR) || fread(&dx ,4,1,f)!=1 || fread(&dy ,4,1,f)!=1 ||
       fread(&nbp,2,1,f)!=1 || fread(&bpp,2,1,f)!=1 || fread(&k,4,1,f)!=1)
     Fatal("Cannot read header from %s\n",file);
   //  Reverse bytes on big endian hardware (detected by backwards magic)
   if (magic==0x424D)
   {
      Reverse(&dx,4);
      Reverse(&dy,4);
      Reverse(&nbp,2);
      Reverse(&bpp,2);
      Reverse(&k,4);
   }
   //  Check image parameters
   glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);
   if (dx<1 || dx>max) Fatal("%s image width %d out of range 1-%d\n",file,dx,max);
   if (dy<1 || dy>max) Fatal("%s image height %d out of range 1-%d\n",file,dy,max);
   if (nbp!=1)  Fatal("%s bit planes is not 1: %d\n",file,nbp);
   if (bpp!=24) Fatal("%s bits per pixel is not 24: %d\n",file,bpp);
   if (k!=0)    Fatal("%s compressed files not supported\n",file);
#ifndef GL_VERSION_2_0
   //  OpenGL 2.0 lifts the restriction that texture size must be a power of two
   for (k=1;k<dx;k*=2);
   if (k!=dx) Fatal("%s image width not a power of two: %d\n",file,dx);
   for (k=1;k<dy;k*=2);
   if (k!=dy) Fatal("%s image height not a power of two: %d\n",file,dy);
#endif

   //  Allocate image memory
   size = 3*dx*dy;
   image = (unsigned char*) malloc(size);
   if (!image) Fatal("Cannot allocate %d bytes of memory for image %s\n",size,file);
   //  Seek to and read image
   if (fseek(f,20,SEEK_CUR) || fread(image,size,1,f)!=1) Fatal("Error reading data from image %s\n",file);
   fclose(f);
   //  Reverse colors (BGR -> RGB)
   for (k=0;k<size;k+=3)
   {
      unsigned char temp = image[k];
      image[k]   = image[k+2];
      image[k+2] = temp;
   }

   //  Sanity check
   ErrCheck("LoadTexBMP");
   //  Generate 2D texture
   glGenTextures(1,&texture);
   glBindTexture(GL_TEXTURE_2D,texture);
   //  Copy image
   glTexImage2D(GL_TEXTURE_2D,0,3,dx,dy,0,GL_RGB,GL_UNSIGNED_BYTE,image);
   if (glGetError()) Fatal("Error in glTexImage2D %s %dx%d\n",file,dx,dy);
   //  Scale linearly when image size doesn't match
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

   //  Free image memory
   free(image);
   //  Return texture name
   return texture;
}


//  Load an OBJ file
//  Vertex, Normal and Texture coordinates are supported
//  Materials are supported
//  Textures must be BMP files
//  Surfaces are not supported
//
//  WARNING:  This is a minimalist implementation of the OBJ file loader.  It
//  will only correctly load a small subset of possible OBJ files.  It is
//  intended to be a starting point to allow you to load models, but in order
//  for you to do this you will have to understand the OBJ format and most
//  likely will have to significantly enhance this code.
//
//  WARNING:  There are lots of really broken OBJ files on the internet.  Some
//  files may have correct surfaces, but the normals are complete junk and so
//  the lighting is totally broken.  So beware of which OBJ files you use.

//  Material structure
typedef struct
{
   char* name;                 //  Material name
   float Ka[4],Kd[4],Ks[4],Ns; //  Colors and shininess
   float d;                    //  Transparency
   int map;                    //  Texture
} mtl_t;

//  Material count and array
static int Nmtl=0;
static mtl_t* mtl=NULL;

//
//  Return true if CR or LF
//
static int CRLF(char ch)
{
   return ch == '\r' || ch == '\n';
}

//
//  Read line from file
//    Returns pointer to line or NULL on EOF
//
static int linelen=0;    //  Length of line
static char* line=NULL;  //  Internal storage for line
static char* readline(FILE* f)
{
   char ch;  //  Character read
   int k=0;  //  Character count
   while ((ch = fgetc(f)) != EOF)
   {
      //  Allocate more memory for long strings
      if (k>=linelen)
      {
         linelen += 8192;
         line = (char*)realloc(line,linelen);
         if (!line) Fatal("Out of memory in readline\n");
      }
      //  End of Line
      if (CRLF(ch))
      {
         // Eat extra CR or LF characters (if any)
         while ((ch = fgetc(f)) != EOF)
           if (!CRLF(ch)) break;
         //  Stick back the overrun
         if (ch != EOF) ungetc(ch,f);
         //  Bail
         break;
      }
      //  Pad character to line
      else
         line[k++] = ch;
   }
   //  Terminate line if anything was read
   if (k>0) line[k] = 0;
   //  Return pointer to line or NULL on EOF
   return k>0 ? line : NULL;
}

//
//  Read to next non-whitespace word
//  Note that this destroys line in the process
//
static char* getword(char** line)
{
   char* word;
   //  Skip leading whitespace
   while (**line && isspace(**line))
      (*line)++;
   if (!**line) return NULL;
   //  Start of word
   word = *line;
   //  Read until next whitespace
   while (**line && !isspace(**line))
      (*line)++;
   //  Mark end of word if not NULL
   if (**line)
   {
      **line = 0;
      (*line)++;
   }
   return word;
}

//
//  Read n floats
//
static void readfloat(char* line,int n,float x[])
{
   int i;
   for (i=0;i<n;i++)
   {
      char* str = getword(&line);
      if (!str)  Fatal("Premature EOL reading %d floats\n",n);
      if (sscanf(str,"%f",x+i)!=1) Fatal("Error reading float %d\n",i);
   }
}

//
//  Read coordinates
//    n is how many coordiantes to read
//    N is the coordinate index
//    M is the number of coordinates
//    x is the array
//    This function adds more memory as needed in 8192 work chunks
//
static void readcoord(char* line,int n,float* x[],int* N,int* M)
{
   //  Allocate memory if necessary
   if (*N+n > *M)
   {
      *M += 8192;
      *x = (float*)realloc(*x,(*M)*sizeof(float));
      if (!*x) Fatal("Cannot allocate memory\n");
   }
   //  Read n coordinates
   readfloat(line,n,(*x)+*N);
   (*N)+=n;
}

//
//  Read string conditionally
//     Line must start with skip string
//     After skip sting return first word
//     getword terminates the line
//
static char* readstr(char* line,const char* skip)
{
   //  Check for a match on the skip string
   while (*skip && *line && *skip==*line)
   {
      skip++;
      line++;
   }
   //  Skip must be NULL for a match
   if (*skip || !isspace(*line)) return NULL;
   //  Read string
   return getword(&line);
}

//
//  Load materials from file
//
static void LoadMaterial(const char* file)
{
   int k=-1;
   char* line;
   char* str;

   //  Open file or return with warning on error
   FILE* f = fopen(file,"r");
   if (!f)
   {
      fprintf(stderr,"Cannot open material file %s\n",file);
      return;
   }

   //  Read lines
   while ((line = readline(f)))
   {
      //  New material
      if ((str = readstr(line,"newmtl")))
      {
         int l = strlen(str);
         //  Allocate memory for structure
         k = Nmtl++;
         mtl = (mtl_t*)realloc(mtl,Nmtl*sizeof(mtl_t));
         //  Store name
         mtl[k].name = (char*)malloc(l+1);
         if (!mtl[k].name) Fatal("Cannot allocate %d for name\n",l+1);
         strcpy(mtl[k].name,str);
         //  Initialize materials
         mtl[k].Ka[0] = mtl[k].Ka[1] = mtl[k].Ka[2] = 0;   mtl[k].Ka[3] = 1;
         mtl[k].Kd[0] = mtl[k].Kd[1] = mtl[k].Kd[2] = 0;   mtl[k].Kd[3] = 1;
         mtl[k].Ks[0] = mtl[k].Ks[1] = mtl[k].Ks[2] = 0;   mtl[k].Ks[3] = 1;
         mtl[k].Ns  = 0;
         mtl[k].d   = 0;
         mtl[k].map = 0;
      }
      //  If no material short circuit here
      else if (k<0)
      {}
      //  Ambient color
      else if (line[0]=='K' && line[1]=='a')
         readfloat(line+2,3,mtl[k].Ka);
      //  Diffuse color
      else if (line[0]=='K' && line[1] == 'd')
         readfloat(line+2,3,mtl[k].Kd);
      //  Specular color
      else if (line[0]=='K' && line[1] == 's')
         readfloat(line+2,3,mtl[k].Ks);
      //  Material Shininess
      else if (line[0]=='N' && line[1]=='s')
         readfloat(line+2,1,&mtl[k].Ns);
      //  Textures (must be BMP - will fail if not)
      else if ((str = readstr(line,"map_Kd")))
         mtl[k].map = LoadTexBMP(str);
      //  Ignore line if we get here
   }
   fclose(f);
}

//
//  Set material
//
static void SetMaterial(const char* name)
{
   int k;
   //  Search materials for a matching name
   for (k=0;k<Nmtl;k++)
      if (!strcmp(mtl[k].name,name))
      {
         //  Set material colors
         glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT  ,mtl[k].Ka);
         glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE  ,mtl[k].Kd);
         glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR ,mtl[k].Ks);
         glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&mtl[k].Ns);
         //  Bind texture if specified
         if (mtl[k].map)
         {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,mtl[k].map);
         }
         else
            glDisable(GL_TEXTURE_2D);
         return;
      }
   //  No matches
   fprintf(stderr,"Unknown material %s\n",name);
}

//
//  Load OBJ file
//
int LoadOBJ(const char* file)
{
   int k;
   int  Nv,Nn,Nt;  //  Number of vertex, normal and textures
   int  Mv,Mn,Mt;  //  Maximum vertex, normal and textures
   float* V;       //  Array of vertexes
   float* N;       //  Array of normals
   float* T;       //  Array if textures coordinates
   char*  line;    //  Line pointer
   char*  str;     //  String pointer

   //  Open file
   FILE* f = fopen(file,"r");
   if (!f) Fatal("Cannot open file %s\n",file);

   // Reset materials
   mtl = NULL;
   Nmtl = 0;

   //  Start new displaylist
   int list = glGenLists(1);
   glNewList(list,GL_COMPILE);
   //  Push attributes for textures
   glPushAttrib(GL_TEXTURE_BIT);

   //  Read vertexes and facets
   V  = N  = T  = NULL;
   Nv = Nn = Nt = 0;
   Mv = Mn = Mt = 0;
   while ((line = readline(f)))
   {
      //  Vertex coordinates (always 3)
      if (line[0]=='v' && line[1]==' ')
         readcoord(line+2,3,&V,&Nv,&Mv);
      //  Normal coordinates (always 3)
      else if (line[0]=='v' && line[1] == 'n')
         readcoord(line+2,3,&N,&Nn,&Mn);
      //  Texture coordinates (always 2)
      else if (line[0]=='v' && line[1] == 't')
         readcoord(line+2,2,&T,&Nt,&Mt);
      //  Read and draw facets
      else if (line[0]=='f')
      {
         line++;
         //  Read Vertex/Texture/Normal triplets
         glBegin(GL_POLYGON);
         while ((str = getword(&line)))
         {
            int Kv,Kt,Kn;
            //  Try Vertex/Texture/Normal triplet
            if (sscanf(str,"%d/%d/%d",&Kv,&Kt,&Kn)==3)
            {
               if (Kv<0 || Kv>Nv/3) Fatal("Vertex %d out of range 1-%d\n",Kv,Nv/3);
               if (Kn<0 || Kn>Nn/3) Fatal("Normal %d out of range 1-%d\n",Kn,Nn/3);
               if (Kt<0 || Kt>Nt/2) Fatal("Texture %d out of range 1-%d\n",Kt,Nt/2);
            }
            //  Try Vertex//Normal pairs
            else if (sscanf(str,"%d//%d",&Kv,&Kn)==2)
            {
               if (Kv<0 || Kv>Nv/3) Fatal("Vertex %d out of range 1-%d\n",Kv,Nv/3);
               if (Kn<0 || Kn>Nn/3) Fatal("Normal %d out of range 1-%d\n",Kn,Nn/3);
               Kt = 0;
            }
            //  Try Vertex index
            else if (sscanf(str,"%d",&Kv)==1)
            {
               if (Kv<0 || Kv>Nv/3) Fatal("Vertex %d out of range 1-%d\n",Kv,Nv/3);
               Kn = 0;
               Kt = 0;
            }
            //  This is an error
            else
               Fatal("Invalid facet %s\n",str);
            //  Draw vectors
            if (Kt) glTexCoord2fv(T+2*(Kt-1));
            if (Kn) glNormal3fv(N+3*(Kn-1));
            if (Kv) glVertex3fv(V+3*(Kv-1));
         }
         glEnd();
      }
      //  Use material
      else if ((str = readstr(line,"usemtl")))
         SetMaterial(str);
      //  Load materials
      else if ((str = readstr(line,"mtllib")))
         LoadMaterial(str);
      //  Skip this line
   }
   fclose(f);
   //  Pop attributes (textures)
   glPopAttrib();
   glEndList();

   //  Free materials
   for (k=0;k<Nmtl;k++)
      free(mtl[k].name);
   free(mtl);

   //  Free arrays
   free(V);
   free(T);
   free(N);

   return list;
}



static void Project(double fov,double asp,double dim)
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
      gluPerspective(fov,asp,dim/100,100*dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}
 
static void Vertex(double th,double ph, double r,double g,double b)
{
   glColor3f(r,g,b); // Setting color to yellow
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

static void ball(double x,double y,double z,double r)
{
   int th,ph;
   float yellow[] = {1.0,1.0,0.0,1.0};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,1);
   glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph,1,1,0);
         Vertex(th,ph+inc,1,1,0);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

static void Tree()
{
 const int d=5;
   int th,ph;
   
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslatef(-1.2,0.5,0);
   glScalef(0.3,0.6,0.4);
 
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[3]);
   
   for (ph=-90;ph<90;ph+=d)
   {	
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=d)
      {
      	 glTexCoord2f(sin(th),0);
         Vertex(th,ph,0.6,0.8,0.2);
         glTexCoord2f(cos(th),1);
         Vertex(th,ph+d,0.6,0.8,0.2);
      }
      glEnd();
   }
    glPopMatrix(); 
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslated(-0.3,-0.6,0);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glColor3f(1,1,1);
    glBindTexture(GL_TEXTURE_2D,texture[4]);
    //Start bark of tree
    glBegin(GL_QUADS);
    // Front
    glNormal3f(0,0,1);
    glColor3f(0.4,0.25,0.13);
    glTexCoord2f(0,0);glVertex3f(-1,0.1, 0.1);
    glTexCoord2f(1,0);glVertex3f(-0.8,0.1, 0.1);
    glTexCoord2f(1,1);glVertex3f(-0.8,0.54, 0.1);
    glTexCoord2f(0,1);glVertex3f(-1,0.54, 0.1);
   //  Back
   glNormal3f(0,0,-1);
   glColor3f(0.4,0.25,0.13);
   glTexCoord2f(0,0);glVertex3f(-0.8,0.1,0.1);
   glTexCoord2f(1,0);glVertex3f(-1,0.1,-0.1);
   glTexCoord2f(1,1);glVertex3f(-1,0.54,-0.1);
   glTexCoord2f(0,1);glVertex3f(-0.8,0.54,-0.1);
   //  Right
   glColor3f(0.4,0.25,0.13);
   glNormal3f(1,0,0);
   glTexCoord2f(0,0);glVertex3f(-0.8,0.1,0.1);
   glTexCoord2f(1,0);glVertex3f(-0.8,0.1,-0.1);
   glTexCoord2f(1,1);glVertex3f(-0.8,0.54,-0.1);
   glTexCoord2f(0,1);glVertex3f(-0.8,0.54,0.1);
   //  Left
   glColor3f(0.4,0.25,0.13);
   glNormal3f(-1,0,0);
   glTexCoord2f(0,0);glVertex3f(-1,0.1,-0.1);
   glTexCoord2f(1,0);glVertex3f(-1,0.1,0.1);
   glTexCoord2f(1,1);glVertex3f(-1,0.54,0.1);
   glTexCoord2f(0,1);glVertex3f(-1,0.54,-0.1);
   //  Top
   glColor3f(0.4,0.25,0.13);
   glNormal3f(0,1,0);
   glTexCoord2f(0,0);glVertex3f(-1,0.54,0.1);
   glTexCoord2f(1,0);glVertex3f(-0.8,0.54,0.1);
   glTexCoord2f(1,1);glVertex3f(-0.8,0.54,-0.1);
   glTexCoord2f(0,1);glVertex3f(-1,0.54,-0.1); 
   //  Bottom
   glColor3f(0.4,0.25,0.13);
   glNormal3f(0,-1,0);
   glTexCoord2f(0,0);glVertex3f(-1,0.1,-0.1);
   glTexCoord2f(1,0);glVertex3f(-0.8,0.1,-0.1);
   glTexCoord2f(1,1);glVertex3f(-0.8,0.1,0.1);
   glTexCoord2f(0,1);glVertex3f(-1,0.1,0.1);
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
	
static void walls()
{
   int th;
    float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   glPushMatrix();
   glScaled(0.8,3,0.8);
   
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[1]);
   
   //  Top
   glBegin(GL_TRIANGLE_FAN);
   glColor3f(1,0,0);
   glNormal3f(0,1,0);
   glVertex3f(0,1,0);
   for (th=0;th<=360;th+=15)
   glVertex3f(Cos(th),1,Sin(th));
   glEnd();
    glDisable(GL_TEXTURE_2D);
   //  Sides
  
    glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[0]);
   glBegin(GL_QUAD_STRIP);
   for (th=0;th<=360;th+=15)
   {  glTexCoord2f(Sin(i),0);
      glNormal3f(Cos(th),1,Sin(th));
      glVertex3f(Cos(th),1,Sin(th));
       glTexCoord2f(Sin(i),1);
        glNormal3f(Cos(th),0,Sin(th));
      glVertex3f(Cos(th),0,Sin(th));
   }
   glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1,0,0);
   glNormal3f(0,-1,0);
   glVertex3f(0,0,0);
   for (th=0;th<=360;th+=15)
      glVertex3f(Cos(th),0,Sin(th));
   glEnd();
   glPopMatrix();
    glPushMatrix();
    glPolygonOffset(-1,-1);
    glTranslated(0,0.5,0);
    glScaled(0.8,0.8,0.8);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[1]);
     glBegin(GL_TRIANGLE_FAN);
     glColor3f(1,0,0);
         
		 glVertex3f(0,5,0);
         for (i=0;i<=360;i+=15)
         {
            glTexCoord2f(Cos(i)+0.5,Sin(i)+0.5);
            glVertex3f(Cos(i),3,Sin(i));
         }
         glEnd();
        glBegin(GL_TRIANGLE_FAN);
   glNormal3f(0,-1,0);
   glVertex3f(0,3,0);
   for (th=0;th<=360;th+=15)
      glVertex3f(Cos(th),3,Sin(th));
   glEnd();
   glPopMatrix();
    glDisable(GL_POLYGON_OFFSET_FILL);
     glDisable(GL_TEXTURE_2D);
          
}
static void cylinder()
{
	
	glPushMatrix();
	glTranslated(-300,-20,-250);
	glScaled(200,200,200);
	walls();
	glPopMatrix();
	glPushMatrix();
	glTranslated(300,-20,-250);
	glScaled(200,200,200);
	walls();
	glPopMatrix(); 
}

static void cube(double x, double y, double z)
{
	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glTexCoord2f(0,0); glVertex3f(0,0,0);
	glTexCoord2f(0,1); glVertex3f(0,y,0);
	glTexCoord2f(1,1); glVertex3f(x,y,0);
	glTexCoord2f(1,0); glVertex3f(x,0,0);
	
	glNormal3f(1,0,0);
	glTexCoord2f(0,0); glVertex3f(x,0,z);
	glTexCoord2f(0,1); glVertex3f(x,0,0);
	glTexCoord2f(1,1); glVertex3f(x,y,0);
	glTexCoord2f(1,0); glVertex3f(x,y,z);
	
	glNormal3f(-1,0,0);
	glTexCoord2f(0,0); glVertex3f(0,0,z);
	glTexCoord2f(0,1); glVertex3f(0,y,z);
	glTexCoord2f(1,1); glVertex3f(0,y,0);
	glTexCoord2f(1,0); glVertex3f(0,0,0);
		
	glNormal3f(0,0,-1);
	glTexCoord2f(0,0); glVertex3f(0,0,z);
	glTexCoord2f(1,0); glVertex3f(x,0,z);
	glTexCoord2f(1,1); glVertex3f(x,y,z);
	glTexCoord2f(0,1); glVertex3f(0,y,z);
	
	glNormal3f(0,1,0);
	glTexCoord2f(0,0); glVertex3f(0,y,0);
        glTexCoord2f(0,1); glVertex3f(0,y,z);
        glTexCoord2f(1,1); glVertex3f(x,y,z);
	glTexCoord2f(1,0); glVertex3f(x,y,0);
	
	glNormal3f(0,-1,0);
        glTexCoord2f(0,1); glVertex3f(0,0,z);
	glTexCoord2f(0,0); glVertex3f(0,0,0);
	glTexCoord2f(1,0); glVertex3f(x,0,0);
	glTexCoord2f(1,1); glVertex3f(x,0,z);
	
	glEnd();
}
static void circle()
{
   glBegin(GL_TRIANGLE_FAN);
   glNormal3f(0,0,1);
   glVertex3f(0,0,0);
   for (th=0;th<=360;th+=15)
   {
   	 glNormal3f(0,0,1);
     glVertex3f(Cos(th),Sin(th),0);
   }
   glEnd();
}

static void Sky(double D) //skycube
{
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   //  Sides
   glBindTexture(GL_TEXTURE_2D,sky[2]);
   glBegin(GL_QUADS);
   glTexCoord2f(0,0); glVertex3f(-D,0,+D);
   glTexCoord2f(1,0); glVertex3f(-D,0,-D);
   glTexCoord2f(1,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0,1); glVertex3f(-D,+D,+D);
  
   glTexCoord2f(1,0); glVertex3f(-D,0,-D);
   glTexCoord2f(0,0); glVertex3f(+D,0,-D);
   glTexCoord2f(0,1); glVertex3f(+D,+D,-D);
   glTexCoord2f(1,1); glVertex3f(-D,+D,-D);
   
   glTexCoord2f(0,0); glVertex3f(+D,0,-D);
   glTexCoord2f(1,0); glVertex3f(+D,0,+D);
   glTexCoord2f(1,1); glVertex3f(+D,+D,+D);
   glTexCoord2f(0,1); glVertex3f(+D,+D,-D);
   
   glTexCoord2f(0,0); glVertex3f(+D,0,+D);
   glTexCoord2f(1,0); glVertex3f(-D,0,+D);
   glTexCoord2f(1,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0,1); glVertex3f(+D,+D,+D);
   glEnd();

   //  Top and bottom
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,sky[4]);
   glBegin(GL_QUADS);
   glTexCoord2f(1,0); glVertex3f(+D,+D,-D);
   glTexCoord2f(0,0); glVertex3f(+D,+D,+D);
   glTexCoord2f(0,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(1,1); glVertex3f(-D,+D,-D);
   glEnd();

   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,sky[6]);
   glBegin(GL_QUADS);
   glTexCoord2f(1,1); glVertex3f(-D,0,+D);
   glTexCoord2f(0,1); glVertex3f(+D,0,+D);
   glTexCoord2f(0,0); glVertex3f(+D,0,-D);
   glTexCoord2f(1,0); glVertex3f(-D,0,-D);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}

static void Road()
{
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   glDisable(GL_TEXTURE_2D);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(-1,-1);
   glBegin(GL_QUADS);
  glColor3f(0.2,0.2,0.2);
  glNormal3f(0,1,0);
  glVertex3f(1,-10,-5000);
  glVertex3f(1,-10,5000);         //a long road
  glVertex3f(70,-10,5000);
  glVertex3f(70,-10,-5000);

  glColor3f(1.0,0.75,0.0);
  glVertex3f(-1,-10,-5000);       //a median
  glVertex3f(-1,-10,5000);
  glVertex3f(1,-10,5000);
  glVertex3f(1,-10,-5000);

  glColor3f(0.2,0.2,0.2);
  glVertex3f(-70,-10,-5000);
  glVertex3f(-70,-10,5000);         //a long road
  glVertex3f(-1,-10,5000);
  glVertex3f(-1,-10,-5000); 
  glEnd();
   glDisable(GL_POLYGON_OFFSET_FILL);
}
static void lights() //lights for the lamp
{ 
  float white[] = {1,1,1,1};
	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,white);
	int th,ph,d=5;
	glPushMatrix();
	glRotated(360,1,0,1);
	glScaled(0.7,0.3,0.7);
	for (ph=0;ph<90;ph+=d)
   {
      	
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=d)
      {
         Vertex(th,ph,1,1,0);
         Vertex(th,ph+d,1,1,0);
      }
      glEnd();
      
   }
   glPopMatrix();
}
static void Lamp()
{
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[2]);
	glColor3f(0.7,0.7,0.7);
	
	cube(-1,10,1);
	
	glPushMatrix();
	glTranslated(-0.4,-0.4,0);
	glRotated(10,0,1,0);
	glBegin(GL_QUADS);
	glVertex3f(0,10,1);
	glVertex3f(15,10,1);
	glVertex3f(15,10.2,1);
	glVertex3f(0,10.2,1);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}


static void trafficsignal()
{
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
  glBindTexture(GL_TEXTURE_2D,texture[2]);
	
  glColor3f(0.7,0.7,0.7);
  cube(0.5,7,1);
  glPushMatrix();
  glTranslated(0,7,0);  
  cube(2,3,1);
  glPopMatrix();
  if(k<=70)
  {
  glDisable(GL_TEXTURE_2D); //disable the metal texture over the circle so that red light seems on
  glColor3f(1,0,0);
  glPushMatrix();
  glTranslated(1,9,2); 
  glScaled(0.3,0.3,0.3);
  circle();
  glPopMatrix();
  k++;
}
 glEnable(GL_TEXTURE_2D);
   if(k<=140&&k>70)
   {
    glDisable(GL_TEXTURE_2D); //disable the metal texture over the circle so that yellow light seems on
   glColor3f(1.1,1,0);
  glPushMatrix();
  glTranslated(1,8,2); 
  glScaled(0.3,0.3,0.3);
  circle();
  glPopMatrix();
   k++;
}
else
{
	glEnable(GL_TEXTURE_2D);
}
 glEnable(GL_TEXTURE_2D);
   if(k<=210&&k>140)
   {
    glDisable(GL_TEXTURE_2D); //disable the metal texture over the circle so that green light seems on
   glColor3f(0,1,0);
  glPushMatrix();
  glTranslated(1,7,2); 
  glScaled(0.3,0.3,0.3);
  circle();
  glPopMatrix();
  k++;
}
if(k==210)
{
	k=0;
}
glEnable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_2D);
}

static void buildings()
{
	float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,sky[1]);
   cube(5,15,5);
   glDisable(GL_TEXTURE_2D);
}
static void zebracrossing()
{
	glEnable(GL_TEXTURE_2D);
	 glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glColor3f(1,1,1);
    glBindTexture(GL_TEXTURE_2D,sky[0]);
	glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1,-1);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glNormal3f(0,1,0);
	glTexCoord2f(0,1); glVertex3f(-62,-8.55,-15);
    glTexCoord2f(0,0); glVertex3f(-62,-8.55,15);         //a long road
    glTexCoord2f(1,0); glVertex3f(62,-8.55,15);
    glTexCoord2f(1,1); glVertex3f(62,-8.55,-15); 
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_TEXTURE_2D);
   
}

static void roadpylon()
{
	glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glColor3f(1,1,1);
    glBindTexture(GL_TEXTURE_2D,sky[3]);
    glBegin(GL_QUADS);
    glColor3f(1,1,1);     // white
   glNormal3f(0,5.28,5.28);
    glTexCoord2f(0.0  ,0.0);glVertex3f(-1.2, 0.8, 1.2); 
    glTexCoord2f(1.0  ,0.0);glVertex3f(1.2, 0.8, 1.2);
    glTexCoord2f(1.0  ,1.0);glVertex3f(1.0, 2.0, 0.0);
    glTexCoord2f(0.0  ,1.0);glVertex3f(-1.0, 2.0, 0.0);
      
    glColor3f(1,1,1);     // white
     glNormal3f(0,-5.28,5.28);
    glTexCoord2f(0.0  ,0.0);glVertex3f(1.0, 2.0, 0.0); //p1
    glTexCoord2f(1.0  ,0.0);glVertex3f(-1.0, 2.0, 0.0); //p4
    glTexCoord2f(1.0  ,1.0);glVertex3f( -1.2, 0.8, -1.2); //p2
    glTexCoord2f(0.0  ,1.0);glVertex3f( 1.2, 0.8, -1.2); //p3
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture[2]);
    glBegin(GL_QUADS);
    glColor3f(1,1,1);     // white
    glNormal3f(0,0.48,0.48);// cross product of (p2-p1) and (p3-p4)
    glTexCoord2f(0.0  ,0.0);glVertex3f(-1.4, 0, 1.4); 
    glTexCoord2f(1.0  ,0.0);glVertex3f(-1.2, 0, 1.4);
    glTexCoord2f(1.0  ,1.0);glVertex3f(-0.8, 2.0, 0);
    glTexCoord2f(0.0  ,1.0);glVertex3f(-1.0, 2.0, 0);
    
    glColor3f(1,1,1);     // white
    glNormal3f(0,0.48,-0.48);// cross product of (p2-p1) and (p3-p4)
    glTexCoord2f(0.0  ,0.0);glVertex3f(-0.8, 2, 0.0); //p1
    glTexCoord2f(1.0  ,0.0);glVertex3f(-1.0, 2.0, 0.0); //p4
    glTexCoord2f(1.0  ,1.0);glVertex3f( -1.4, 0, -1.4); //p2
    glTexCoord2f(0.0  ,1.0);glVertex3f( -1.2, 0, -1.4); //p3
    
    glColor3f(1,1,1);     // white
    glNormal3f(0,0.48,0.48);// cross product of (p2-p1) and (p3-p4)
    glTexCoord2f(0.0  ,0.0);glVertex3f(1.2, 0, 1.4); 
    glTexCoord2f(1.0  ,0.0);glVertex3f(1.4, 0, 1.4);
    glTexCoord2f(1.0  ,1.0);glVertex3f(1.0, 2.0, 0.0);
    glTexCoord2f(0.0  ,1.0);glVertex3f(0.8, 2.0, 0.0);
      
    glColor3f(1,1,1);     // white
    glNormal3f(0,0.48,-0.48);// cross product of (p2-p1) and (p3-p4)
    glTexCoord2f(0.0  ,0.0);glVertex3f(1.0, 2.0, 0.0); //p1
    glTexCoord2f(1.0  ,0.0);glVertex3f(0.8, 2.0, 0.0); //p4
    glTexCoord2f(1.0  ,1.0);glVertex3f( 1.2, 0, -1.4); //p2
    glTexCoord2f(0.0  ,1.0);glVertex3f( 1.4, 0, -1.4); //p3
    
    glEnd();   // Done drawing the roof */
    glDisable(GL_TEXTURE_2D);
}

static void roadsign()
{
   glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,sky[5]);
   glPushMatrix();
   glBegin(GL_QUADS);
   {
   	glNormal3f(0,0,1);
   	glTexCoord2f(0,1); glVertex3f(-1.5,11.5,0);
   	glTexCoord2f(0,0); glVertex3f(0,10,0);
   	glTexCoord2f(1,0); glVertex3f(1.5,11.5,0);
   	glTexCoord2f(1,1); glVertex3f(0,13,0);
   }
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);

   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glPushMatrix();
   glTranslated(0,-1,0);
   glScaled(0.1,11,0.1);
   glBegin(GL_QUAD_STRIP);
   for (th=0;th<=360;th+=15)
   {  glTexCoord2f(Sin(i),0);
      glNormal3f(Cos(th),1,Sin(th));
      glVertex3f(Cos(th),1,Sin(th));
       glTexCoord2f(Sin(i),1);
        glNormal3f(Cos(th),0,Sin(th));
      glVertex3f(Cos(th),0,Sin(th));
   }
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
void keyOperations()
{
	if (keySpecialStates['z'] || keySpecialStates['Z'])
       jump+=1;
    if (keySpecialStates['x']||keySpecialStates['X'] )
       jump-=1;
}
void keySpecialOperations(void) 
{     

   if (keySpecialStates[GLUT_KEY_RIGHT] && view==2)
    {
    	collision=0;
    	for(s=0;s<7;s++)
    	{
	     if(collisiondistance[s]<14) 
	     {
	     view=3;
	     collision=1;
	     score-=100;
	     }
	     else
	     {
		 score+=50;
		 }
	    }
	    if(collision!=1 && Ex<64){
		 Ex+=jump;
		 }
		if (Ex>=64)
		{
		  view=4;
		}
		
      }
    // To move to the left
    if (keySpecialStates[GLUT_KEY_LEFT] && view==2)
    {
    	collision=0;
    	for(s=0;s<7;s++)
    	{
    	 if(collisiondistance[s]<14)
    	 {
    	  view=3;
    	  collision=1;
    	  score-=100;
	    }
	    else
	    score+=50;
	}
	    if(collision!=1 && Ex>-64)
	    {
	    Ex-=jump;
		}
		if (Ex<=-64)
		{
			view=4;
		}
        }
    
    // To move forward
    if (keySpecialStates[GLUT_KEY_UP] && view==2)
    {
    	collision=0;
    	for(s=0;s<7;s++)
    	{
    	 if(collisiondistance[s]<14)
    	 {
    	 	 view=3;
    	 	 collision=1;
    	 	 score-=100;
    	 }
    	 else
    	 score+=50;
	    }
	    
	    if(collision!=1)
		{
        
		 Ez-=jump;
	     }
	     else
	     {
	     	Ez+=5;
		 }
    	
    }
    //To move backward
     if (keySpecialStates[GLUT_KEY_DOWN] && view==2)
    {
    	 collision=0;
    for(s=0;s<7;s++)
    	{
    		
    	 if(collisiondistance[s]<14)
    	 {
    	     view=3;
    	 	 collision=1;
    	 	 score-=100;
    	 }
    	 else
    	 score+=50;
	    }
	    if(collision!=1)
		{
			
     	Ez+=jump;
	     }
		 else
	     {
	     	Ez-=5;
		 }
		 
    }
}
static void drawCar1()
{
    glPushMatrix();
    glTranslatef(0,-10,995);
    glRotatef(90, 1, 0, 0);
    glRotatef(180, 0, 1, 0);
    glRotatef(ah,0,0,1);
    glScaled(1,1,1);
    glCallList(car1);
    glPopMatrix();
     
}

static void drawCar2()
{
    glPushMatrix();
    glTranslatef(0,-10,995);
    glRotatef(90, 1, 0, 0);
    glRotatef(180, 0, 1, 0);
    glRotatef(ah,0,0,1);
    glScaled(1,1,1);
    glCallList(car2);
    glPopMatrix();
}

void display1()
{    
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glWindowPos2i(300,450);
    Print(" ",2);
    	glWindowPos2i(200,450);
	Print("### Press SPACE to select car ###",2);
	glWindowPos2i(100,400);
	Print("Controls:",2);
        glWindowPos2i(100,350);
	Print("Up arrow : Move Forward ",2);
	glWindowPos2i(100,300);
	Print("Down arrow : Move Backward ",2);
	glWindowPos2i(100,250);
	Print("Right arrow: Move towards right ",2);
	glWindowPos2i(100,200);
	Print("Left arrow: Move towards left ",2);
	glWindowPos2i(100,150);
	Print("Z: Increase Speed",2);
	glWindowPos2i(100,100);
	Print("X: Decrease Speed ",2);
	glWindowPos2i(100,50);
	Print("+/-: Zoom in/Zoom out ",2);
	glWindowPos2i(100,0);
	Print("ESC: To exit game",2);

	glEnable(GL_TEXTURE_2D);
	 glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[6]);
   glBegin(GL_QUADS);                
   glColor3f(1,1,1); 
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0);glVertex3f(-55,-30,-35);
   glTexCoord2f(1,0);glVertex3f(+55,-30,-35);
   glTexCoord2f(1,1);glVertex3f(+55,+30,-35);
   glTexCoord2f(0,1);glVertex3f(-55,+30,-35);
   glEnd();
   glDisable(GL_TEXTURE_2D);
	
}
void display3()
{
	
    viewx=1;
	glClearColor(0.6, 0.75, 0.85, 1); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	   //  Enable Z-buffering in OpenGL
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
	glWindowPos2i(250,500);
	Print("This is the car you selected: ",1);
	glWindowPos2i(250,50);
	Print("Press Enter to start game",1);
	glPushMatrix();
	glTranslated(0,11,0);
	drawCar1();
	glPopMatrix();
	  
   
}
void display4()
{
	viewx=1;
	glClearColor(0.6, 0.75, 0.85, 1); // sky color is light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glWindowPos2i(250,450);
	Print("This is the car you selected: ",1);
	glWindowPos2i(250,100);
	Print("Press Enter to start game",1);
	glPushMatrix();
	glTranslated(0,11,0);
	drawCar2();
	glPopMatrix();
}
void display2()
{    
   glClearColor(0.6, 0.75, 0.85, 1); // sky color is light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glWindowPos2i(250,500);
	  //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   glCullFace(GL_BACK);
  
   glLoadIdentity();
   // Set the look at position for perspective projection
      		gluLookAt(
			Ex,      Ey,      Ez,
			Ex , Ey, Ez+lz,
			0.0,    1,    0.0);
	Print("Select car :",1);
	glWindowPos2i(250,450);
	Print("1. Mercedes Benz",1);
	glWindowPos2i(250,400);
	Print("2. Ferrari California ",1);

	if(opt==1)
	{
		display3();
	}
	else if(opt==2)
	{
		display4();
	}
		glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[7]);
   glBegin(GL_QUADS);                
   glColor3f(1,1,1); 
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0);glVertex3f(-1200,-1000,200);
   glTexCoord2f(1,0);glVertex3f(+1200,-1000,200);
   glTexCoord2f(1,1);glVertex3f(+1200,+1000,200);
   glTexCoord2f(0,1);glVertex3f(-1200,+1000,200);
   glEnd();
   glDisable(GL_TEXTURE_2D);
}

void displayCollision() //display when car collides with road pylons
{
	 glClearColor(0.6, 0.75, 0.85, 1); // sky color is light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	 glEnable(GL_DEPTH_TEST);
   glCullFace(GL_BACK);
  	Ex=0;
	  	Ey=1;
	  	Ez=1000;
   glLoadIdentity();
   	gluLookAt(
			Ex,      Ey,      Ez,
			Ex , Ey, Ez+lz,
			0.0,    1,    0.0);
     glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   	glWindowPos2i(450,500);
   	Print("COLLISION DETECTED",1);
   	glWindowPos2i(450,400);
   	Print("GAME OVER",1);
    glWindowPos2i(450,350);
    Print("High Score = %d",1,score);
   	glWindowPos2i(450,300);
   	Print("Press ESC to escape game",1);
   	glWindowPos2i(450,250);
   	Print("Press r/R to restart game",1);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,texture[8]);
   glBegin(GL_QUADS);                
   glColor3f(1,1,1); 
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0);glVertex3f(-1200,-1000,200);
   glTexCoord2f(1,0);glVertex3f(+1200,-1000,200);
   glTexCoord2f(1,1);glVertex3f(+1200,+1000,200);
   glTexCoord2f(0,1);glVertex3f(-1200,+1000,200);
   glEnd();
   glDisable(GL_TEXTURE_2D);
	
}
void displayoutofbounds() //display when car goes off the road
{
	 glClearColor(0.6, 0.75, 0.85, 1); // sky color is light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	 glEnable(GL_DEPTH_TEST);
   glCullFace(GL_BACK);
  	Ex=0;
	  	Ey=1;
	  	Ez=1000;
   glLoadIdentity();
   	gluLookAt(
			Ex,      Ey,      Ez,
			Ex , Ey, Ez+lz,
			0.0,    1,    0.0);
     glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   	glWindowPos2i(450,500);
   	Print("CAR OUT OF BOUNDS",1);
   	glWindowPos2i(450,400);
   	Print("GAME OVER",1);
    glWindowPos2i(450,350);
    Print("High Score = %d",1,score);
   	glWindowPos2i(450,300);
   	Print("Press ESC to escape game",1);
   	glWindowPos2i(450,250);
   	Print("Press r/R to restart game",1);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,texture[9]);
   glBegin(GL_QUADS);                
   glColor3f(1,1,1); 
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0);glVertex3f(-1200,-1000,200);
   glTexCoord2f(1,0);glVertex3f(+1200,-1000,200);
   glTexCoord2f(1,1);glVertex3f(+1200,+1000,200);
   glTexCoord2f(0,1);glVertex3f(-1200,+1000,200);
   glEnd();
   glDisable(GL_TEXTURE_2D);
	
}


void display()
  {
      keyOperations();
   keySpecialOperations();
     glClearColor(0.6, 0.75, 0.85, 1); // sky color is light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	/* Clear The Screen And The Depth Buffer */
	if(view==0)
	{   
		display1();
	}

  else if (view==1)
	{
	display2();
   }
   else if(view==3)
   {
   	displayCollision();
   }
   else if(view==4)
   {
   	displayoutofbounds();
   }
   else if(view==2)
   {
   //  Erase the window and the depth buffer
   
   glClearColor(0.6, 0.75, 0.85, 1); // sky color is light blue
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   glCullFace(GL_BACK);
  
   glLoadIdentity();
    
   // Set the look at position for perspective projection
      		gluLookAt(
			Ex,      Ey,      Ez,
			Ex , Ey, Ez+lz,
			0.0,    1,    0.0);
   
    
  glShadeModel(GL_SMOOTH);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glPushMatrix();
  glTranslatef(	Ex , 0, Ez+lz-4);
  Sky(1000); 
  glPopMatrix();
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  
    //  Light switch
   if (light)
   {
        //  Translate intensity to color vectors
        float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
        float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
        float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
        //  Light position
      float Position[]={0,1000,5000};
        //  Draw light position as ball (still no lighting here)
        glColor3f(1,1,1);
         ball(Position[0],Position[1],Position[2] , 0.2);
        //  OpenGL should normalize normal vectors
        glEnable(GL_NORMALIZE);
        //  Enable lighting
        glEnable(GL_LIGHTING);
        //  glColor sets ambient and diffuse color materials
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        //  Enable light 0
        glEnable(GL_LIGHT0);
        //  Set ambient, diffuse, specular components and position of light 0
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position);
        
   }
   else
     glDisable(GL_LIGHTING);
   
	if(opt==1)
	{
		 glPushMatrix();
         glTranslatef(	Ex,-10, Ez+lz-25);
         glRotatef(90, 1, 0, 0);
         glRotatef(180, 0, 1, 0);
         glColor3f(1,1,1);
         glScalef(5,5,5);
		 glCallList(car1);
		 glPopMatrix();
		 glPushMatrix();
		 glDisable(GL_DEPTH_TEST);
		 glEnable(GL_BLEND); 
		 glColor3f(1,1,1);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //so that bounding circle is transparent
         glColor4f(1.0f, 1.0f, 1.0f, 0.0f); 
         glTranslatef(	Ex,-4, Ez+lz-25+4);
         glutSolidSphere(4, 100, 100); //bounding circle to detect collision detection
         glPopMatrix();
         glDisable(GL_BLEND);
          glEnable(GL_DEPTH_TEST);
		 int E= Ez+lz-4;
		 if(E<=10)
		 {
		 	Ex=0;
		 	Ez=1000;
		 	lz=-1;
		 }
		   if(E>1005)
		 {
		 	Ex=0;
		 	Ez=1000;
		 	lz=-1;
		 }
		
	}
	else if(opt==2)
	{
		 glPushMatrix();
         glTranslatef(	Ex, -10, Ez+lz-25);
         glRotatef(90, 1, 0, 0);
         glRotatef(180, 0, 1, 0);
         glScalef(5,5,5 );
         glColor3f(1,1,1);
		 glCallList(car2);
		 glPopMatrix();
		 glPushMatrix();
		 glDisable(GL_DEPTH_TEST);
		 glEnable(GL_BLEND); 
		 glColor3f(1,1,1);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //so that bounding circle is transparent
         glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
         glTranslatef(	Ex,-4, Ez+lz-25+4); 
         glutSolidSphere(4, 100, 100); //bounding circle to detect collision detection
         glPopMatrix();
         glDisable(GL_BLEND);
          glEnable(GL_DEPTH_TEST);
		   int E=Ez+lz-4;
		 if(E<=10)
		 {
		 	Ex=0;
		 	Ez=1000;
		 	lz=-1;
		 }
		   if(E>1005)
		 {
		 	Ex=0;
		 	Ez=1000;
		 	lz=-1;
		 }
	}
	cylinder();
        Road();
    
     
	for(i=0; i<=1000; i+=40)
	{
	glPushMatrix();
    glTranslated(-50,10,i);
    glScaled(50,50,50);
    Tree();
    glPopMatrix();
  }
    for(i=0; i<=1000; i+=40)
	{
	glPushMatrix();
    glTranslated(170,10,i);
    glScaled(50,50,50);
    Tree();
    glPopMatrix();
  }
    for(i=0; i<=1000; i+=60)
	{
	glPushMatrix();
    glTranslated(-70,-10,i);
    glScaled(2,10,10);
    Lamp();
    glPopMatrix();
  }
  for(i=0; i<=1000; i+=60)
	{
	glPushMatrix();
    glTranslated(70,-10,i);
    glRotated(90,0,1,0);
    glScaled(2,10,10);
    Lamp();
    glPopMatrix();
  }
  for(i=-60; i<=1000; i+=60)
	{
	glPushMatrix();
    glTranslated(-38,86.5,i+43);
    glScaled(5,5,5);
    lights();
    glPopMatrix();
  }
   for(i=-60; i<=1000; i+=60)
	{
	glPushMatrix();
    glTranslated(55,87,i+30);
    glScaled(5,5,5);
    lights();
    glPopMatrix();
  }
   for(i=0; i<=1000; i+=400)
	{
	glPushMatrix();
    glTranslated(-74,-10,i);
    glScaled(10,10,10);
    trafficsignal();
    glPopMatrix();
    }
     for(i=0; i<=1000; i+=400)
	{
    glPolygonOffset(-1,-1);
	glPushMatrix();
    glTranslated(0,-1.4,i+40);
    zebracrossing();
    glPopMatrix();
    }
     for(i=-60; i<=1000; i+=100)
	{
	glPushMatrix();
    glTranslated(200,-15,i+30);
    glScaled(10,15,10);
    buildings();
    glPopMatrix();
  }
   for(i=-60; i<=1000; i+=100)
	{
	glPushMatrix();
    glTranslated(-250,-15,i+30);
    glScaled(10,15,10);
    buildings();
    glPopMatrix();
  }
   for(i=-60; i<=1000; i+=100)
	{
	glPushMatrix();
    glTranslated(350,-15,i+30);
    glScaled(10,30,10);
    buildings();
    glPopMatrix();
  }
   for(i=-60; i<=1000; i+=100)
	{
	glPushMatrix();
    glTranslated(-400,-15,i+30);
    glScaled(10,30,10);
    buildings();
    glPopMatrix();
  }
     
   for(e=0; e<=1000; e+=155)
	{
	  if(e%2==0)
	{
		glPushMatrix();
		glTranslated(-16,-10,e);
		glScaled(7,5,4);
        roadpylon();
        glPopMatrix();
         glPushMatrix();
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST); 
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //so that bounding circle is transparent
         glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
         glTranslated(-16,-8,e);
         glutSolidSphere(10, 100, 100); //bounding circle for collision detection
         glPopMatrix();
         glDisable(GL_BLEND);
         glEnable(GL_DEPTH_TEST);
         
    }
   
    else
	{
		glPushMatrix();
		glTranslated(16,-10,e);
        glScaled(6,5,4);
        roadpylon();
        glPopMatrix();
        glPushMatrix();
	 glEnable(GL_BLEND);
	 glDisable(GL_DEPTH_TEST); 
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //so that bounding circle is transparent
         glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
         glTranslated(16,-1.0,e);
         glutSolidSphere(10, 100, 100); //bounding circle for collision detection
         glPopMatrix();
         glDisable(GL_BLEND);
         glEnable(GL_DEPTH_TEST);
       
        
    } 
}
p=0;
	while(p<=1000)
         	{
      for(s=0;s<7;s++)
         {
         if(p%2==0)
         {
         dx=Ex+16;
         dy=4-9;
         dz=(Ez+lz-21)-p;
         }
         else
         {
         dx=Ex-16;
	     dy=4-9;
         dz=(Ez+lz-21)-p;
		 }
         collisiondistance [s]= sqrt(dx*dx + dy*dy + dz*dz);  //array to store the distance between the bounding circle of the car and the bounding circle for each roadpylon
	     p+=155;
	      }
	      
	    }
	   
	   
    for(i=0; i<=1000; i+=400)
	{
	glPushMatrix();
    glTranslated(68,0,i);
    glScaled(10,10,10);
    roadsign();
    glPopMatrix();
    }
  
  xpos = xpos + xvel;

if(xpos >= 30)
{
xvel = -xvel; //to move the dragon down when xpos equals thirty
}
if( xpos <2)
{
xvel = -xvel; //to move the dragon back on top when xpos becomes less than two
}

  glPushMatrix();
  glTranslated(0,xpos,900); //moves the dragon up and down
  glRotated(-90,0,0,1);
  glRotated(-90,0,1,0);
  glScaled(2,2,2);
  glColor3f(1,1,1);
  glCallList(dragon);
  glPopMatrix();
  glWindowPos2i(5,5);
  glColor3f(1,1,1);
  Print("Speed=%f",1,(20+jump));
  Print("km/hr  Score=%d",1,score); 
  Print("Ex =%f",1,Ex); 
   
}
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this function for continuous animation
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
    double t2 = glutGet(GLUT_ELAPSED_TIME)/5000.0;
   xh = fmod(90*t2,90.0);
   ah = 0.1*glutGet(GLUT_ELAPSED_TIME);
  	
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void timerFunction(int value)

{
   
  glutPostRedisplay();
  glutTimerFunc(3000,timerFunction,0);
}


void special(int key,int x,int y)
{
    keySpecialStates[key]=true;
	
    // Raise viewing height
    if (key == GLUT_KEY_PAGE_UP)
        Ey += 0.5;
    // Lower viewing height
    else if (key == GLUT_KEY_PAGE_DOWN && Ey>1)
        Ey -= 0.5;   
   //  Update projection
   Project(fov,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}
void specialup(int key,int x,int y)
{
    keySpecialStates[key]=false;

    // Raise viewing height
    if (key == GLUT_KEY_PAGE_UP)
        Ey += 0.5;
    // Lower viewing height
    else if (key == GLUT_KEY_PAGE_DOWN && Ey>1)
        Ey -= 0.5;   
   //  Update projection
   Project(fov,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}


void keyspeedup(unsigned char ch,int x,int y)
{
	 keySpecialStates[ch]=false;
	 //  Reproject
   Project(fov,asp,dim);
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}
/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
  keySpecialStates[ch]=true;
   //  Exit on ESC
    if (ch == 27)
      exit(0);
    //  Change field of view angle
   else if (ch == '-' && ch>1 && view == 2)
      fov--;
   else if (ch == '+' && ch<179 && view==2)
      fov++;
      
   else if(ch== ' ' && view==0)
  {
  view=1;
   }
   else if(ch==13 && viewx==1)
      {
      	view=2;
	  }
	  else if(ch=='1')
	  {
	  	opt=1;
	  }
	  else if(ch=='2')
	  {
	  	opt=2;
	  }
	  else if(ch=='R' || ch=='r')
	  {
	  	view=0;
	  	Ex=0;
	  	Ey=1;
	  	Ez=1000;
	  }
   
      //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
  //  Reproject
   Project(fov,asp,dim);
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}


/*
 *  GLUT calls this routine when the window is resized
 */
 void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(fov,asp,dim);

}


 /*  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Car Game Simulation");
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   // Tell GLUT to use the method "specialUp" for key up events 
   glutSpecialUpFunc(specialup);
    //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   // Tell GLUT to use the method "keyspeedup" for key up events 
   glutKeyboardUpFunc(keyspeedup);
  
   //  Tell GLUT to call idle function
   glutIdleFunc(idle);
    glutTimerFunc(3000,timerFunction,0);

    //  Load textures
   texture[0] = LoadTexBMP("checks.bmp");
   texture[1] = LoadTexBMP("redmetal.bmp");
   texture[2]=LoadTexBMP("lamp.bmp");
   texture[3]=LoadTexBMP("flower.bmp");
   texture[4]=LoadTexBMP("bark.bmp");
   texture[6]=LoadTexBMP("bestcar.bmp");
   texture[7]=LoadTexBMP("cars.bmp");
   texture[8]=LoadTexBMP("crackedglass.bmp");
   texture[9]=LoadTexBMP("treeoutofbound.bmp");
   sky[0]=LoadTexBMP("crosswalk.bmp");
   sky[1]=LoadTexBMP("building.bmp");
   sky[2]=LoadTexBMP("panorama_360.bmp");
   sky[3]=LoadTexBMP("roadpylon.bmp");
   sky[4]=LoadTexBMP("clouds2.bmp");
   sky[5]=LoadTexBMP("sign.bmp");
   sky[6]=LoadTexBMP("land0.bmp");
  
   car1=LoadOBJ("sls_amg.obj");   
   car2=LoadOBJ("california.obj");
   dragon=LoadOBJ("dragon.obj");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glEnable(GL_DEPTH_TEST);
   glutMainLoop();
   return 0;
}
