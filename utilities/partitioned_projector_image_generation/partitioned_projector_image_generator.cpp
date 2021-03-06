
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
//#include <opencv/highgui.h>
/*  Create checkerboard texture  */
#define checkImageWidth 1024
#define checkImageHeight 768
#define num_of_proj 9
#define num_of_proj_row 3
#define num_of_proj_col 3


#define checker_X 29
#define checker_Y 21
#define total_features_X (checker_X+4)
#define total_features_Y (checker_Y+4)

#define checker_square_width 32

#define margin_X 32
#define margin_Y 32

unsigned int tile_width;
unsigned int tile_height;

static GLubyte **checkImage[4]; /// agreed, better will be [tile_width][tile_height][4]

static GLuint texName;
unsigned int proj_id;
unsigned int tile_coordinates[4];
static GLubyte partition_image[checkImageHeight][checkImageWidth][4]; //this will contain the texture corresponding to proj_id

char filename[100];

void makeCheckImage(void)
{

    //Initialize checkImage...
    for(unsigned int i=0; i<4; i++)
    {
        checkImage[i]=new GLubyte*[tile_height];
        for(unsigned int j=0; j<tile_height; j++)
            checkImage[i][j]=new GLubyte[tile_width];
    }

   char c;


    //Lets make the margins white
    for (unsigned int i = 0; i < tile_height; i++)
    {
        for (unsigned int j = 0; j < tile_width; j++)
        {
  /*          if((i<margin_Y) || (i>=(tile_height-margin_Y)) || (j<margin_X) || (j>=(tile_width-margin_X)))
            {
                checkImage[0][i][j]=(GLubyte)255;
                checkImage[1][i][j]=(GLubyte)255;
                checkImage[2][i][j]=(GLubyte)255;
                checkImage[3][i][j]=(GLubyte)255;
                continue;
            }
*/
            c = (((i&0b1000000)==0)^((j&0b1000000)==0))*255;
            checkImage[0][i][j] = (GLubyte) c;
            checkImage[1][i][j] = (GLubyte) c;
            checkImage[2][i][j] = (GLubyte) c;
            checkImage[3][i][j] = (GLubyte) 255;
        }
    }

    return;
}


///Note: This function will extract texture for specific projector
void create_partition_image()
{
    //Read tile coordinates
    sprintf(filename,"../chromium_proj_%d.txt",proj_id);
    FILE*fd_tile_coordinates=fopen(filename,"r");

    for(unsigned int i=0; i<4; i++)
        fscanf(fd_tile_coordinates,"%u",&tile_coordinates[i]);

    for(unsigned int r=0; r<checkImageHeight; r++)
        for(unsigned int c=0; c<checkImageWidth; c++)
        {
            for(unsigned int b=0; b<4; b++)
                partition_image[r][c][b]=checkImage[b][tile_coordinates[1]+r][tile_coordinates[0]+c];
        }

    return;
}

void init(void)
{
    // Setting screen background
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);

    makeCheckImage();
    create_partition_image();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);

    //Setting OGL texture paramters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);

    // Coupling loaded image to 2D texture OGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth,
                 checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 partition_image);
    return;
}




void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //glTranslatef(0.0, 0.0, -3.6);
}


void display()
{
    sprintf(filename,"mapping_tile_proj_%d.txt",proj_id);
    FILE*fd_warp_coordinates=fopen(filename,"r");

    //float warp_coordinates[checker_X][checker_Y][2]; // stores warp projector image coordinates
    unsigned total_points=total_features_X*total_features_Y;
    float vertex[total_features_X][total_features_Y][2],tex[total_features_X][total_features_Y][2];

    //Lets read warped coordinates...
    for(unsigned int r=0; r<total_features_Y; r++)
    {
        for(unsigned int c=0; c<total_features_X; c++)
        {
            fscanf(fd_warp_coordinates,"%f\t",&vertex[c][r][0]);
            fscanf(fd_warp_coordinates,"%f\t",&vertex[c][r][1]);
            fscanf(fd_warp_coordinates,"%f\t",&tex[c][r][0]);
            fscanf(fd_warp_coordinates,"%f\n",&tex[c][r][1]);

        }
    }

    unsigned int n=0,k1,k2,row=0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texName);

   glBegin(GL_TRIANGLE_STRIP);
    while(row<(total_features_Y-1))
    {
        row++;
        k1=k2=0;
        n=0;
        while(n<(total_features_X*2))
        {
            if(n%2==0) //vertex_number for a vertex in triangles along a row of projectors
            {
                glTexCoord2f(tex[k1][row][0],tex[k1][row][1]);
                glVertex2f(vertex[k1][row][0],vertex[k1][row][1]);
                k1++;
            }

            else
            {
                glTexCoord2f(tex[k2][row-1][0],tex[k2][row-1][1]);
                glVertex2f(vertex[k2][row-1][0],vertex[k2][row-1][1]);
                k2++;
            }
            n++;
        }
    }

    glEnd();
    glFlush();
    glDisable(GL_TEXTURE_2D);

    return;
}




void keyboard (unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;

        // case 'w': // start projecting warped image
        //   warp_image();

    default:
        break;
    }
}

int main(int argc, char** argv)
{

    unsigned int proj_tile_coordinates[num_of_proj][4];
    FILE*fd_tile_config;

    for(unsigned int n=0; n<num_of_proj; n++)
    {
        sprintf(filename,"../chromium_proj_%u.txt",n);
        fd_tile_config=fopen(filename,"r");
        for(unsigned int i=0; i<4; i++)
            fscanf(fd_tile_config,"%u",&proj_tile_coordinates[n][i]);

    }

    tile_width=proj_tile_coordinates[num_of_proj_row-1][0]+proj_tile_coordinates[num_of_proj_row-1][2];
    tile_height=proj_tile_coordinates[(num_of_proj_col-1)*num_of_proj_row][1]+proj_tile_coordinates[(num_of_proj_col-1)*num_of_proj_row][3];

    printf("Hello!!:\n%u %u",tile_width,tile_height);

    printf("Enter proj_id:");
    scanf("%u",&proj_id);
    //proj_id=atoi(argv[1]); // this will contain the projector id
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(argv[0]);

    init();
    glutDisplayFunc(display);
    //glutFullScreen();
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}












/*
TARGET:
To generate partitioned images for projectors, so that when all partitions are projected by the projector array it should be seamless.

Purpose:
To test chromium rendering.

Interface:
./prog_name n(proj_id)  //generates texture for projector proj_id

Steps:
1. Read global tile configuration.
2. Generate global image.(i.e.,texture)
3. Generate subimage depending upon n,tile coordinates for projector n.
4. Texture map from subimage using mapping{vertex,texture} info. given.
5. Project image on fullscreen.
*/
/*
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"


//#include <opencv/highgui.h>
/*  Create checkerboard texture  */
/*
#define checkImageWidth 1024
#define checkImageHeight 768
#define num_of_proj 9
#define num_of_proj_row 3
#define num_of_proj_col 3


#define checker_X 29
#define checker_Y 21

#define checker_square_width 32

#define margin_X 32
#define margin_Y 32

unsigned int tile_width;
unsigned int tile_height;

static GLubyte **checkImage[4]; /// agreed, better will be [tile_height][tile_width][4]

static GLuint texName;
unsigned int proj_id;
unsigned int tile_coordinates[4];
static GLubyte partition_image[checkImageHeight][checkImageWidth][4]; //this will contain the texture corresponding to proj_id

char filename[100];


IplImage*test_image;

void makeCheckImage(void)
{
    int i, j, c;

    //Initialize checkImage...
    for(unsigned int i=0; i<4; i++)
    {
        checkImage[i]=new GLubyte*[tile_height];
        for(unsigned int j=0; j<tile_height; j++)
            checkImage[i][j]=new GLubyte[tile_width];
    }

    //Lets make the margins white
    for (i = 0; i < tile_height; i++)
    {
        for (j = 0; j < tile_width; j++)
        {
          /*  if((i<margin_Y) || (i>=(tile_height-margin_Y)) || (j<margin_X) || (j>=(tile_width-margin_X)))
            {
                checkImage[0][i][j]=(GLubyte)255;
                checkImage[1][i][j]=(GLubyte)255;
                checkImage[2][i][j]=(GLubyte)255;
                checkImage[3][i][j]=(GLubyte)255;
                continue;
            }
*/
 /*           c = (((i&0b1000000)==0)^((j&0b1000000)==0))*255;
            checkImage[0][i][j] = (GLubyte) c;
            checkImage[1][i][j] = (GLubyte) c;
            checkImage[2][i][j] = (GLubyte) c;
            checkImage[3][i][j] = (GLubyte) 255;
        }
    }

    return;
}


///Note: This function will extract texture for specific projector
void create_partition_image()
{
    //Read tile coordinates
    sprintf(filename,"chromium_proj_%d.txt",proj_id);
    FILE*fd_tile_coordinates=fopen(filename,"r");

    for(unsigned int i=0; i<4; i++)
        {
            fscanf(fd_tile_coordinates,"%u",&tile_coordinates[i]);
            printf("%u\t",tile_coordinates[i]);
        }


        test_image=cvCreateImage(cvSize(1024,768),IPL_DEPTH_8U,1);
        IplImage*full_texture=cvCreateImage(cvSize(tile_width,tile_height),IPL_DEPTH_8U,1);

       for(unsigned int r=0; r<tile_height; r++)
        for(unsigned int c=0; c<tile_width; c++)
        {
            full_texture->imageData[r*full_texture->widthStep+c]=checkImage[0][r][c];

        }

    for(unsigned int r=0; r<checkImageHeight; r++)
        for(unsigned int c=0; c<checkImageWidth; c++)
        {
            for(unsigned int b=0; b<4; b++)
                {
                    partition_image[r][c][b]=checkImage[b][tile_coordinates[1]+r][tile_coordinates[0]+c];

                }


                test_image->imageData[r*test_image->widthStep+c]=partition_image[r][c][0];


        }


        cvSaveImage("test_image.bmp",test_image);
        cvSaveImage("full_texture.bmp",full_texture);

    return;
}

void init(void)
{
    // Setting screen background
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);

    makeCheckImage();
    create_partition_image();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);

    //Setting OGL texture paramters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);

    // Coupling loaded image to 2D texture OGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth,
                 checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 partition_image);
    return;
}




void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //glTranslatef(0.0, 0.0, -3.6);
}


void display()
{
    sprintf(filename,"mapping_tile_proj_%d.txt",proj_id);
    FILE*fd_warp_coordinates=fopen(filename,"r");

    //float warp_coordinates[checker_X][checker_Y][2]; // stores warp projector image coordinates
    unsigned total_points=checker_X*checker_Y;
    float vertex[checker_X][checker_Y][2],tex[checker_X][checker_Y][2];

    //Lets read warped coordinates...
    for(unsigned int r=0; r<checker_Y; r++)
    {
        for(unsigned int c=0; c<checker_X; c++)
        {
            fscanf(fd_warp_coordinates,"%f\t",&vertex[c][r][0]);
            fscanf(fd_warp_coordinates,"%f\t",&vertex[c][r][1]);
            fscanf(fd_warp_coordinates,"%f\t",&tex[c][r][0]);
            fscanf(fd_warp_coordinates,"%f\n",&tex[c][r][1]);

            //printf("%f\t%f\n",(warp_coordinates[c][r][0]/511.5)-1.0,(warp_coordinates[c][r][1]/383.5)-1.0);
        }
    }

    //Generate texture coordinates:
    //Lets initialize texture & vertex coordinates for internal corners of checkerboard:

    /*
        for(unsigned int r=0; r<checker_Y; r++)
            for(unsigned int c=0; c<checker_X; c++)
            {
                //t=((warp_coordinates[c][r][0]*2.0f/checkImageWidth)-1.0);
                vertex[c][r][0]=warp_coordinates[c][r][0];

                //t=((warp_coordinates[c][r][1]*2.0f/checkImageHeight)-1.0);
                vertex[c][r][1]=warp_coordinates[c][r][1];

               // tex[c][r][0]=(margin_X+(c+1)*checker_square_width)/(checkImageWidth-1.0); // texture coordinates
              //  tex[c][r][1]=1.0f-(margin_Y+(r+1)*checker_square_width)/(checkImageHeight-1.0);//since OpenGL coordinate system has Y in opposite direction.
            }
    */
    /*
    unsigned int n=0,k1,k2,row=0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texName);
    glBegin(GL_TRIANGLE_STRIP);
    while(row<(checker_Y-1))
    {
        row++;
        k1=k2=0;
        n=0;
        while(n<(checker_X*2))
        {
            if(n%2==0) //vertex_number for a vertex in triangles along a row of projectors
            {
                glTexCoord2f(tex[k1][row][0],tex[k1][row][1]);
                glVertex2f(vertex[k1][row][0],vertex[k1][row][1]);
                k1++;
            }

            else
            {
                glTexCoord2f(tex[k2][row-1][0],tex[k2][row-1][1]);
                glVertex2f(vertex[k2][row-1][0],vertex[k2][row-1][1]);
                k2++;
            }
            n++;
        }
    }
    glEnd();
    glFlush();
    glDisable(GL_TEXTURE_2D);

    return;
}




void keyboard (unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;

        // case 'w': // start projecting warped image
        //   warp_image();

    default:
        break;
    }
}

int main(int argc, char** argv)
{

    unsigned int proj_tile_coordinates[num_of_proj][4];
    FILE*fd_tile_config;

    for(unsigned int n=0; n<num_of_proj; n++)
    {
        sprintf(filename,"chromium_proj_%u.txt",n);
        fd_tile_config=fopen(filename,"r");
        for(unsigned int i=0; i<4; i++)
            fscanf(fd_tile_config,"%u",&proj_tile_coordinates[n][i]);

    }

    tile_width=proj_tile_coordinates[num_of_proj_row-1][0]+proj_tile_coordinates[num_of_proj_row-1][2];
    tile_height=proj_tile_coordinates[(num_of_proj_col-1)*num_of_proj_row][1]+proj_tile_coordinates[(num_of_proj_col-1)*num_of_proj_row][3];
    printf("tile width:%u\ttile height:%u\n",tile_width,tile_height);
    printf("projector id:\n");
    scanf("%u",&proj_id);
    //proj_id=atoi(argv[1]); // this will contain the projector id

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(argv[0]);

    init();
    glutDisplayFunc(display);
    //glutFullScreen();
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
*/
