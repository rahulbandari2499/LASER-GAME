#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <stdlib.h>//for random number
#include <time.h> //for time
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <thread>
#include <ao/ao.h>
#include <mpg123.h>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
int  fbwidth=800,fbheight=600;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
//global declarations
struct COLOR//for colors 
{
  float r,g,b;
};
typedef struct COLOR COLOR;

/*DEFINE SOME STRUCT FOR EVERY OBJECT*/
struct Things {
  string name;
  COLOR color;
  float x,y;//for translation 
  VAO* object;
  float height,width;//for rectangles
  float radius;//for circle
  int number;//for blocks for identification
  float angle;//angle to be rotated
  int status;//for the laser if 0 then laser not in use and vice versa
};
typedef struct Things Things;

map <string, Things> baskets;
map <string, Things> laser;
Things blocks[1000];
map <string, Things> backgroundObjects;
Things laser_beam[1000];
float release_angle[1000];
//map <string, Things> mirrors;

Things mirror1[6];
Things mirror2[6];
Things mirror3[6];
Things mirror4[6];

float base_x,base_y;
//for points
string scoreLabel="POINTS";
Things scoreLabelObjects[12],score_valueObjects[12];//score label;
float scoreLabel_x=175,scoreLabel_y=250;
float score_value_x=300,score_value_y=250;

//for level 
Things levelLabelObjects[12],level_valueObjects[12];//score label;
float levelLabel_x=-175,levelLabel_y=-250;
float level_value_x=-350,level_value_y=-250;


//for GAME OVER
Things endLabelObjects[12]; //The you win/lose label
string endLabel="";
float endLabel_x=-160,endLabel_y=0;



Things score1[12];
Things score2[12];
Things score3[12];

//you win or you lose

//for displaying text
map <string, Things> *characters[10]; //For displaying text
char characterValues[10];
float characterPosX[10],characterPosY[10];


float x_change=0,y_change=0;//for camera pan
float zoom_camera=1;//for zooming camera



/*MOVEMENT FOR THE OBJECTS*/
//baskets
float red_basket_trans=-100;
float green_basket_trans=100;
int flag_left=0,flag_right=0;
//laser_gun
float laser_gun_trans=0;
float laser_gun_rotate=0;
int no_of_laser_beams=0,no_of_gone_lasers=0;
int shoot_beam=0;
//blocks
float blocks_y_increments = 0;
int no_of_blocks=0;
int no_of_gone_blocks=0;
float block_speed=2;

double last_update_time1 = glfwGetTime(), current_time1;
double last_update_time2 = glfwGetTime(), current_time2;
int score=0;
int misfire=0;
int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0;
int game_over_var=0;
int win_score=50;
int level=1;

//mouse controls
int right_mouse_clicked=0;
double new_mouse_pos_x, new_mouse_pos_y;
double mouse_pos_x, mouse_pos_y;
int m_flag0=0,m_flag1=0,m_flag2=0;
double mouse_x,mouse_y,m_click_x;


float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
/*SOUND*/
void* play_audio(string audioFile){
  mpg123_handle *mh;
  unsigned char *buffer;
  size_t buffer_size;
  size_t done;
  int err;

  int driver;
  ao_device *dev;

  ao_sample_format format;
  int channels, encoding;
  long rate;

  /* initializations */
  ao_initialize();
  driver = ao_default_driver_id();
  mpg123_init();
  mh = mpg123_new(NULL, &err);
  buffer_size = mpg123_outblock(mh);
  buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

  /* open the file and get the decoding format */
  mpg123_open(mh, &audioFile[0]);
  mpg123_getformat(mh, &rate, &channels, &encoding);

  /* set the output format and open the output device */
  format.bits = mpg123_encsize(encoding) * 8;
  format.rate = rate;
  format.channels = channels;
  format.byte_format = AO_FMT_NATIVE;
  format.matrix = 0;
  dev = ao_open_live(driver, &format, NULL);

  /* decode and play */
  char *p =(char *)buffer;
  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
    ao_play(dev, p, done);

  /* clean up */
  free(buffer);
  ao_close(dev);
  mpg123_close(mh);
  mpg123_delete(mh);
}


//it checks for the panning boundaries...
void check_pan()
{
  if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
  else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
  if(y_change-300.0f/zoom_camera<-300)
        y_change=-300+300.0f/zoom_camera;
  else if(y_change+300.0f/zoom_camera>300)
        y_change=300-300.0f/zoom_camera;
}

void check_bounds()
{
  if(red_basket_trans<-235 )
    red_basket_trans=-235;
  if(red_basket_trans>323)
    red_basket_trans=323;
  if(green_basket_trans<-235 )
    green_basket_trans=-235;
  if(green_basket_trans>323)
    green_basket_trans=323;
  if(laser_gun_trans<-160)
    laser_gun_trans=-160;
  if(laser_gun_trans>215)
    laser_gun_trans=215;
  if(laser_gun_rotate>75)
    laser_gun_rotate=75;
  if(laser_gun_rotate<-75)
    laser_gun_rotate=-75;
  if(laser_gun_trans>210 && laser_gun_rotate>20)
    laser_gun_rotate=20;
  if(block_speed>4)
    block_speed=4;
  if(block_speed<0.5)
     block_speed=0.5; 

Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
}


void mousescroll(GLFWwindow* window,double xoffset, double yoffset)
{
    if (yoffset==-1) {
        zoom_camera /= 1.1; //make it bigger than current size
    }
    else if(yoffset==1){
        zoom_camera *= 1.1; //make it bigger than current size
    }
    if (zoom_camera<=1) {
        zoom_camera = 1;
    }
    if (zoom_camera>=4) {
        zoom_camera=4;
    }
    check_pan();
    Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
      if(key==GLFW_KEY_LEFT_CONTROL || key==GLFW_KEY_RIGHT_CONTROL)
      flag1=1;
    if(key==GLFW_KEY_LEFT_ALT || key==GLFW_KEY_RIGHT_ALT)
      flag2=1;
    if(key==GLFW_KEY_LEFT)
      flag3=1;
    if(key==GLFW_KEY_RIGHT)
      flag4=1;
          /*WINDOW SIZING*/
            if(key==GLFW_KEY_UP) //for zoom in
              {
                mousescroll(window,0,1);
                check_pan();
              }
            if(key==GLFW_KEY_DOWN) //for zoom out
                {
                mousescroll(window,0,-1);
                check_pan();
                }
            if(key==GLFW_KEY_LEFT) //for panning left
                {//flag_left=0;
                x_change-=10;
                mousescroll(window,0,0);
                //check_pan();
                }
            if(key==GLFW_KEY_RIGHT) //for panning rigth
                {//flag_right=0;
                x_change+=10;
                mousescroll(window,0,0);
                }
            if(key==GLFW_KEY_T) //for panning up
                {y_change+=10;
                mousescroll(window,0,0);
                }
            if(key==GLFW_KEY_Y) // for panning down
                {y_change-=10;
                mousescroll(window,0,0);
                }
            /*BASKET MOVEMENT*/
            if(flag1 && flag4)
                {//if(flag_right==1){
                red_basket_trans+=10;
                //}
                check_bounds();
                
                }
            if(flag1 && flag3)
                //if(flag_left==1)
                {red_basket_trans-=10;
                check_bounds();
                
                }
            if(flag2 && flag4)
                
                {green_basket_trans+=10;
                check_bounds();
                }
            if(flag2 && flag3)
                //if(flag_left==1)
                {green_basket_trans-=10;
                check_bounds();
               }

            /*LASER GUN MOVEMENT*/
            if(key==GLFW_KEY_A)
                {laser_gun_rotate+=4;
                check_bounds();
                }
            if(key==GLFW_KEY_D)
                {laser_gun_rotate-=4;
                check_bounds();
                }
            if(key==GLFW_KEY_S)
                {laser_gun_trans+=10;
                check_bounds();
                }
            if(key==GLFW_KEY_F)
                {laser_gun_trans-=10;
                check_bounds();
                 }  
            if(key==GLFW_KEY_ESCAPE)
                exit(0);
                
            if(key==GLFW_KEY_SPACE)
                {shoot_beam=1;
                 }
            /*BLOCKS*/
            if(key==GLFW_KEY_N)
                {block_speed+=0.5;
                check_bounds();
                }
            if(key==GLFW_KEY_M)
                {block_speed-=0.5;
                check_bounds();
                }
          
        }
    
    /*else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_SPACE:
                shoot_beam=1;
                break;
            case GLFW_KEY_RIGHT:
                 flag_right=1;
                 break;
            case GLFW_KEY_LEFT:
                 flag_left=1;
                 break;
         
            default:
                break;
        }
    }*/
    else if(action==GLFW_RELEASE){
      
    if(key==GLFW_KEY_LEFT_CONTROL || key==GLFW_KEY_RIGHT_CONTROL)
      flag1=0;
    if(key==GLFW_KEY_LEFT_ALT || key==GLFW_KEY_RIGHT_ALT)
      flag2=0;
    if(key==GLFW_KEY_LEFT)
      flag3=0;
    if(key==GLFW_KEY_RIGHT)
      flag4=0;
        /*LASER BEAM SHOOT*/
        if(key==GLFW_KEY_SPACE)
              shoot_beam=0;
                            
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            exit(0);
            break;
		default:
			break;
	}
}

static void cursor_position(GLFWwindow* window, double xpos, double ypos)
{
  mouse_x= ((800*xpos)/fbwidth)-400;
  mouse_y=-((600*ypos)/fbheight)+300;
  //cout<<"mouse_x:"<<mouse_y<<endl;

  if(m_flag0==1)//red_basket
    red_basket_trans= mouse_x;
  if(m_flag1==1)//green_basket
    green_basket_trans =mouse_x;
  if(m_flag2==1)//canon
    laser_gun_trans = mouse_y;
  check_bounds();
}

/* Executed when a mouse button is pressed/released */

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-400.0f/zoom_camera, 400.0f/zoom_camera, -300.0f/zoom_camera, 300.0f/zoom_camera, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
//creates circle with given number of parts
void createCircle (string name,int weight,COLOR c1,float x,float y,float radius,float parts,string component,int fill) //c1,c2,c3 are colors
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
  int temp1=(int)360*parts;
  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  GLfloat vertex_buffer_data [9*temp1]={0};
  for(int i=0;i<temp1;i++)
  {
    vertex_buffer_data[9*i]=0;
    vertex_buffer_data[9*i+1]=0;
    vertex_buffer_data[9*i+2]=0;
    vertex_buffer_data[9*i+3]=radius*cos(i*M_PI/180);
    vertex_buffer_data[9*i+4]=radius*sin(i*M_PI/180);
    vertex_buffer_data[9*i+5]=0;
    vertex_buffer_data[9*i+6]=radius*cos((i+1)*M_PI/180);
    vertex_buffer_data[9*i+7]=radius*sin((i+1)*M_PI/180);
    vertex_buffer_data[9*i+8]=0;
  }

   GLfloat color_buffer_data [9*temp1];
   
  
  for (int i = 0; i<9*temp1 ; i+=3)
  {
    color_buffer_data[i]=c1.r;
    color_buffer_data[i+1]=c1.g;
    color_buffer_data[i+2]=c1.b;
  }
  VAO* circle;
  if(fill==1)
        circle = create3DObject(GL_TRIANGLES, 3*temp1, vertex_buffer_data, color_buffer_data, GL_FILL);
  else
        circle = create3DObject(GL_TRIANGLES, 3*temp1, vertex_buffer_data, color_buffer_data, GL_LINE);
  Things temp={};
  temp.name=name;
  temp.color=c1;
  temp.number=weight;
  temp.object=circle;
  temp.x=x;
  temp.y=y;
  temp.height=2*radius;
  temp.width=2*radius;
  temp.radius=radius;
  if(component=="basket")
  {
    baskets[name]=temp;
  }
  else if(component=="laser_gun")
  {
    laser[name]=temp;
  }
}

void createRectangle (string name,int weight,COLOR v1,COLOR v2,COLOR v3,COLOR v4,float x,float y,float height,float width,string component)
{
  // GL3 accepts only Triangles. Quads are not supported
  float w=width/2;
  float h=height/2;
  GLfloat vertex_buffer_data [] = {
    -w,-h,0, // vertex 1
    -w,h,0, // vertex 2
    w,h,0, // vertex 3

    w, h,0, // vertex 3
    w, -h,0, // vertex 4
    -w,-h,0  // vertex 1
  };

   GLfloat color_buffer_data [] = {
     v1.r,v1.g,v1.b,
     v2.r,v2.g,v2.b,
     v3.r,v3.g,v3.b,
     v3.r,v3.g,v3.b,
     v4.r,v4.g,v4.b,
     v1.r,v1.g,v1.b
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

  Things temp={};
  temp.name=name;
  temp.color=v1;
  temp.number=weight;
  temp.object=rectangle;
  temp.x=x;
  temp.y=y;
  temp.height=height;
  temp.width=width;
  temp.radius=(sqrt(height*height+width*width))/2;
  if(component=="background")
  {
    backgroundObjects[name]=temp;
  }
  else if(component=="basket")
  {
    baskets[name]=temp;
  }
  else if(component=="laser_gun")
  {
    laser[name]=temp;
  }
  else if(component=="blocks")
  {
    blocks[weight]=temp;
  }
  else if(component=="mirror1")
  {
    mirror1[weight]=temp;
  }
  else if(component=="mirror2")
  {
    mirror2[weight]=temp;
  }
  
  else if(component=="mirror3")
  {
    mirror3[weight]=temp;
  }
  else if(component=="mirror4")
  {
    mirror4[weight]=temp;
  }
  else if(component=="laser_beam")
  {
    laser_beam[weight]=temp;
  }
  else if(component=="scorelabel")
  {
    scoreLabelObjects[weight]=temp;
  }
  else if(component=="endlabel")
  {
    endLabelObjects[weight]=temp;
  }
  else if(component=="score_value")
  {
    score_valueObjects[weight]=temp;
  }
  else if(component=="level_value")
  {
    level_valueObjects[weight]=temp;
  }
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
void game_over()
{
  game_over_var=1;
  //thread(play_audio,"./sounds/gameover.wav").detach();
  system("canberra-gtk-play -f ./sounds/gameover.wav");
  printf("final_score:%d\n",score);
  //exit(0);
}

int baskets_not_intersecting()
{
  if((red_basket_trans+45<green_basket_trans-45) || (red_basket_trans-45>green_basket_trans+45))
    return 1;
  else
    return 0;
}

void check_collection_of_falling_blocks()
{
  COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
    COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
    COLOR black = {30/255.0,30/255.0,21/255.0}; 
    


  
  for(int i=0;i<no_of_blocks;i++)
  {
    if(blocks[i].status==1)
    {
      //printf("nanan%d\n",baskets_not_intersecting());
      if(blocks[i].y<=-210 && blocks[i].y>=-220 && baskets_not_intersecting())
    {
      if(blocks[i].x>=red_basket_trans-22.5 && blocks[i].x<=red_basket_trans+22.5)
      {
        if(blocks[i].color.r==red.r)
          {
            //printf("case 3\n");
            score+=5;
            blocks[i].status=0;
          }
        if(blocks[i].color.r==black.r)
        {
          //printf("case 2\n");
          game_over();
        }
        if(blocks[i].color.r==darkgreen.r)
        {
          //printf("case 4\n");
          score-=3;
          blocks[i].status=0;
        }
      }
      else if(blocks[i].x<=green_basket_trans+22.5 && blocks[i].x>=green_basket_trans-22.5 )
          {
            if(blocks[i].color.r==darkgreen.r)
              {
                //printf("case 3\n");
                score+=5;
                blocks[i].status=0;
              }
            if(blocks[i].color.r==black.r)
            {
              //printf("case 2\n");
              game_over();
            }
            if(blocks[i].color.r==red.r)
            {
              //printf("case 4\n");
              score-=3;
              blocks[i].status=0;
            }
          }
  }}
}
for(int i=0;i<no_of_blocks  ;i++)
  {
    if(blocks[i].y<=-222 && blocks[i].status==1)
    {
      if(blocks[i].color.r==black.r)
      {
        //printf("case 2\n");
        //game_over();
      }
      else if(blocks[i].color.r==red.r || blocks[i].color.r==darkgreen.r)
        {
          //printf("case 7\n");
          score-=3;
        }
      blocks[i].status=0;
      
    }
  }
}
void check_collision_with_black_boxes()
{
  COLOR black = {30/255.0,30/255.0,21/255.0}; 
  COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
    COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
  for(int i=0;i<no_of_blocks;i++)
  {
    for(int rem=0;rem<no_of_laser_beams;rem++)
    {
      if(laser_beam[rem].status==1 && blocks[i].status==1)
      {
        float tempx1=laser_beam[rem].x+(laser_beam[rem].width/2)*cos(laser_beam[rem].angle*M_PI/180.0f);
        float tempy1=laser_beam[rem].y+(laser_beam[rem].width/2)*sin(laser_beam[rem].angle*M_PI/180.0f);
        float w1=blocks[i].x-blocks[i].width/2;
        float w2=blocks[i].x+blocks[i].width/2;
        float w3=blocks[i].y-blocks[i].height/2;
        float w4=blocks[i].y+blocks[i].height/2;
        if(tempx1>=w1 && tempx1<=w2 && tempy1<=w4 && tempy1>=w3)
        {
          if(blocks[i].color.r==black.r && blocks[i].color.g==black.g && blocks[i].color.b==black.b)
            {
              //printf("case 5\n");
              score+=5;
            }
          if(blocks[i].color.r==red.r && blocks[i].color.g==red.g && blocks[i].color.b==red.b)
            {
              //printf("case 6\n");
              score-=3;
              misfire++;
            }
          if(blocks[i].color.r==darkgreen.r && blocks[i].color.g==darkgreen.g && blocks[i].color.b==darkgreen.b)
              {
                //printf("case 6\n");
                score-=3;
                misfire++;
              }
          blocks[i].status=0;
          laser_beam[rem].status=0;
        }
      }

    }

  }
  if(misfire>=5)
  {
    //printf("case 1\n");
    game_over();
  }
}

float x_inter,y_inter;
int check_for_intersection_with_mirror(float x0,float y0,float x1,float y1,int i)
{
    float s1_x, s1_y, s2_x, s2_y, x2, y2, x3, y3, q, p, r;
    if(i==0){//angle=120
      x2=225;y2=162.5;x3=275;y3=77.5;
    }
    if(i==1){//angle=150
      x2=-65;y2=252.5;x3=-15;y3=167.5;
    }
    if(i==2){//angle=30
      x2=202.5;y2=-115;x3=117.5;y3=-165;
    }
    s1_x = x1 - x0;
    s1_y = y1 - y0;
    s2_x = x3 - x2;     
    s2_y = y3 - y2;
    
    r=s1_x*s2_y - s2_x*s1_y;
    if(r==0)
      return 0;

    p = (s1_x*(y0-y2) - s1_y*(x0-x2))/r;
    q = (s2_x*(y0-y2) - s2_y*(x0-x2))/r;

    if (p>=0 && p<=1 && q>=0 && q<=1)
    {
        x_inter = x0 + (q * s1_x);
        y_inter = y0 + (q * s1_y);
        return 1;
    }
    return 0;
}


void falling_blocks()
{ 
    COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
    COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
    COLOR black = {30/255.0,30/255.0,21/255.0}; 
    srand(time(NULL));
    int which_color=rand()%3;

    float temp_x=(rand()%541-200);
    
    if(which_color==0)
    {  

      createRectangle("r1",no_of_blocks,red,red,red,red,temp_x,247.5,45,45,"blocks");
    }
    else if(which_color==1)
    {
      createRectangle("g1",no_of_blocks,darkgreen,darkgreen,darkgreen,darkgreen,temp_x,247.5,45,45,"blocks");
    }
    else
    {
      createRectangle("b1",no_of_blocks,black,black,black,black,temp_x,247.5,45,45,"blocks");
    }

    //cout << no_of_blocks << " " << which_color << endl;

}
void put_crosses()
{
  COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
  if(misfire>0)
  {
    createRectangle("cross_1",1,red,red,red,red,-355,255,3,15,"background");
    createRectangle("cross_2",1,red,red,red,red,-355,255,3,15,"background");
  }
  if(misfire>1)
  {
    createRectangle("cross_3",1,red,red,red,red,-330,255,3,15,"background");
    createRectangle("cross_4",1,red,red,red,red,-330,255,3,15,"background");
  }
  if(misfire>2)
  {
    createRectangle("cross_5",1,red,red,red,red,-305,255,3,15,"background");
    createRectangle("cross_6",1,red,red,red,red,-305,255,3,15,"background");
  }
  if(misfire>3)
  {
    createRectangle("cross_7",1,red,red,red,red,-280,255,3,15,"background");
    createRectangle("cross_8",1,red,red,red,red,-280,255,3,15,"background");
  }
  if(misfire>4)
  {
    createRectangle("cross_9",1,red,red,red,red,-255,255,3,15,"background");
    createRectangle("cross_10",1,red,red,red,red,-255,255,3,15,"background");
  }
}



void generate_laser_beam()
{
  //thread(play_audio,"./sounds/laser_shoot.wav").detach();
  COLOR blue = {0,0,1};
  //laser beam
  createRectangle("laser_beam",no_of_laser_beams,blue,blue,blue,blue,-370,laser_gun_trans,3,50,"laser_beam");
  laser_beam[no_of_laser_beams].angle=laser_gun_rotate;
  laser_beam[no_of_laser_beams].status=1;
  //system("canberra-gtk-play -f ./laser_shoot.wav");
}
void check_laser_bounds()
{
  for(int rem=0;rem<no_of_laser_beams;rem++)
  {
    Things temp=laser_beam[rem];
    if(temp.x>400 || temp.y>300 || temp.y<-300 || temp.x<-400)
    {
      temp.status=0;
    }
  }
}

void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
            {
              m_flag0=0;
              m_flag1=0;
              m_flag2=0;
              shoot_beam=0;
            }

            else if(action==GLFW_PRESS)
            {
              if(mouse_x>=-370 && mouse_x<=-340 && mouse_y>=laser_gun_trans-30 && mouse_y<=laser_gun_trans+30)
                   m_flag2=1;
              else if(mouse_x>=-45+red_basket_trans && mouse_x<=45+red_basket_trans && mouse_y>=-300 && mouse_y<=-240)
                   m_flag0=1;
              else if(mouse_x>=-45+green_basket_trans && mouse_x<=45+green_basket_trans && mouse_y>=-300 && mouse_y<=-240)
                   m_flag1=1;
              else if(mouse_x>-370 && (glfwGetTime()-last_update_time2)>=1)
              {
                  float slope = (mouse_y - laser_gun_trans)/(mouse_x+370);
                  float anglee= (atan(slope)*180.0)/M_PI;
                  if(anglee>=-75 && anglee<=75){
                      
                      last_update_time2=glfwGetTime();;
                      laser_gun_rotate = anglee;   
                                         generate_laser_beam();
                      shoot_beam=1;
                      no_of_laser_beams++;
              }
            }
          }
            
            break;
          
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                right_mouse_clicked=1;
            }
            if (action == GLFW_RELEASE) {
                right_mouse_clicked=0;
            }
            break;
        default:
            break;
    }
}


void set_characters(char arr,Things char_seg[])
{
  for(int i=0;i<12;i++)
    char_seg[i].status=0;
  
    char cur=arr;
    //left1
    if(cur=='P' || cur=='O' || cur=='N' || cur=='S' || cur=='Y' || cur=='U'|| cur=='W' || cur=='L'|| cur=='E' || cur=='1' || cur=='0'|| cur=='4'|| cur=='5'|| cur=='6'|| cur=='8'|| cur=='9')
    {
      char_seg[0].status=1;

    }
    //left2
    if(cur=='P' || cur=='O' || cur=='N'  || cur=='U'|| cur=='W'|| cur=='L'|| cur=='E'|| cur=='1'|| cur=='0'|| cur=='2'||  cur=='6'|| cur=='8')
    {
      char_seg[1].status=1;
    }
    //right1
    if(cur=='U' ||cur=='P' || cur=='O' || cur=='N'  || cur=='Y'|| cur=='W'|| cur=='0'|| cur=='2'|| cur=='3'|| cur=='4'|| cur=='8'|| cur=='7'|| cur=='9')
    {
     char_seg[2].status=1; 
    }
    //right2
    if(cur=='O' ||  cur=='N' || cur=='S' || cur=='Y' || cur=='U'|| cur=='W'|| cur=='0'|| cur=='3'|| cur=='5'|| cur=='6'|| cur=='4'|| cur=='8'|| cur=='7'|| cur=='9')
    {
      char_seg[3].status=1;
    }
    //top
    if(cur=='P' || cur=='O' || cur=='I' || cur=='N' || cur=='T' || cur=='S'|| cur=='E'|| cur=='0'|| cur=='2'|| cur=='3'|| cur=='5'|| cur=='6'|| cur=='8' || cur=='7'|| cur=='9')
    {
      char_seg[4].status=1;
    }
    //middle
    if(cur=='P' || cur=='S'  || cur=='Y' || cur=='E'|| cur=='2'|| cur=='3'|| cur=='4'|| cur=='5'|| cur=='6'|| cur=='8'|| cur=='9' || cur=='-')
    {
      char_seg[5].status=1;
    }
    //bottom
    if(cur=='O' || cur=='I' || cur=='S'  || cur=='Y' || cur=='U'|| cur=='L' || cur=='E'|| cur=='0'|| cur=='2'|| cur=='3'|| cur=='5'|| cur=='6'|| cur=='8'|| cur=='9')
    {
      char_seg[6].status=1;
    }
    
    //middle1
    if(cur=='I' || cur=='T' )
    {
      char_seg[7].status=1;
    }
    //middle2
    if(cur=='I' || cur=='T' || cur=='W')
    {
      char_seg[8].status=1;
    }

  }




/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  //Matrices.model = glm::mat4(1.0f);

  /* Render your scene */
/*
  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform;
  MVP = VP * Matrices.model; // MVP = p * V * M
  */
  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  //draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  put_crosses();
  level=score/10+1;
  //block_speed+=level/10;
   



  COLOR winbackground = {212/255.0,175/255.0,55/255.0};
  COLOR losebackground = {255/255.0,77/255.0,77/255.0};
  if(score>=win_score){
        game_over_var=1;
        endLabel_x=-150;
        createRectangle("endgame",10000,winbackground,winbackground,winbackground,winbackground,0,0,200,600,"background");
        endLabel="YOU WIN";
    }

    else if(game_over_var)
    {
        //game_over=1;
        createRectangle("endgame",10000,losebackground,losebackground,losebackground,losebackground,0,0,200,600,"background");
        endLabel="YOU LOSE";
    }
    //mouse_controls for panning
    glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
    if(right_mouse_clicked==1){
        x_change+=new_mouse_pos_x-mouse_pos_x;
        y_change-=new_mouse_pos_y-mouse_pos_y;
        check_pan();
    }
    Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
    glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);


  /*BACKGROUND OBJECTS*/
  for(map<string,Things>::iterator rem=backgroundObjects.begin();rem!=backgroundObjects.end();rem++)
  {
    float angle_rt=0;
    string cur=rem->first;//first object
    Matrices.model = glm::mat4(1.0f);
    if(backgroundObjects[cur].name=="cross_1" || backgroundObjects[cur].name=="cross_3" ||backgroundObjects[cur].name=="cross_5" ||backgroundObjects[cur].name=="cross_7" ||backgroundObjects[cur].name=="cross_9" )
    {
      angle_rt=135;
    }
    else if(backgroundObjects[cur].name=="cross_2" ||backgroundObjects[cur].name=="cross_4" ||backgroundObjects[cur].name=="cross_6" ||backgroundObjects[cur].name=="cross_8" ||backgroundObjects[cur].name=="cross_10" )
      angle_rt=45;
    glm::mat4 translate_back_matrix = glm::translate (glm::vec3(backgroundObjects[cur].x, backgroundObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_back_matrix = glm::rotate((float)(angle_rt*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_back_matrix * rotate_back_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(backgroundObjects[cur].object);
  }

  /*BASKETS*/
  for(map<string,Things>::iterator rem=baskets.begin();rem!=baskets.end();rem++)
  {
    string cur=rem->first;//first object
    Matrices.model = glm::mat4(1.0f);
    
    float basket_circle_angle;
    float movement;

    //movement of the baskets
    if(baskets[cur].name=="green_basket_upper_circle" || baskets[cur].name=="green_basket_lower_circle" || baskets[cur].name=="green_basket")
    {
      movement=green_basket_trans;
    }
    else if(baskets[cur].name=="red_basket_upper_circle" || baskets[cur].name=="red_basket_lower_circle" || baskets[cur].name=="red_basket")
    {
      movement=red_basket_trans;
    }

    //for rotation of circles to get cylinder alike shape
    if(baskets[cur].name=="green_basket_upper_circle" || baskets[cur].name=="red_basket_upper_circle")
    {
      basket_circle_angle=-75;
      
    }
    else if(baskets[cur].name=="green_basket_lower_circle" || baskets[cur].name=="red_basket_lower_circle")
    {
      basket_circle_angle=75+180;
    }
    else
    {
      basket_circle_angle=0;
    }
    glm::mat4 translate_basket_matrix = glm::translate (glm::vec3(baskets[cur].x+movement, baskets[cur].y, 0.0f)); // glTranslatef    
    glm::mat4 rotate_basket_matrix = glm::rotate((float)(basket_circle_angle*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_basket_matrix * rotate_basket_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(baskets[cur].object);
  }

  /*LASER GUN*/
  for(map<string,Things>::iterator rem=laser.begin();rem!=laser.end();rem++)
  {
    string cur=rem->first;//first object
    Matrices.model = glm::mat4(1.0f);
    float rotation_angle=0;
    float temp=0;
    glm::mat4 rotate_laser_matrix;
    if(laser[cur].name=="laser_circle" || laser[cur].name=="laser_circle1" || laser[cur].name=="laser_circle2" )
    {
      rotation_angle=-90;
      rotate_laser_matrix = glm::rotate((float)(rotation_angle*M_PI/180.0f), glm::vec3(0,0,1));
    }
    else if(laser[cur].name=="laser_pivot" || laser[cur].name=="laser_rod")
    {
      temp=laser[cur].width;
      temp=temp/2;
      rotate_laser_matrix = glm::rotate((float)(laser_gun_rotate*M_PI/180.0f), glm::vec3(0,0,1));
    }


    glm::mat4 translate_laser_matrix = glm::translate (glm::vec3(laser[cur].x-temp, laser[cur].y+laser_gun_trans, 0.0f)); // glTranslatef
     // rotate about vector (-1,1,1)
    glm::mat4 translate_laser_matrix_extra = glm::translate (glm::vec3(temp, 0.0f, 0.0f));
     //glm::mat4 translate_laser_matrix_extra1 = glm::translate (glm::vec3(temp, 0.0f, 0.0f));

    Matrices.model *= (translate_laser_matrix * rotate_laser_matrix * translate_laser_matrix_extra);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(laser[cur].object);
  }

  //laser_beam
 if(!game_over_var)
 {
  current_time2=glfwGetTime();
  if ((current_time2 - last_update_time2) >= 1 && shoot_beam==1) 
  {
      generate_laser_beam();
      last_update_time2=current_time2;
      //release_angle[no_of_laser_beams]=laser_gun_rotate;
      no_of_laser_beams++;
  }
  for(int rem=0;rem<no_of_laser_beams;rem++)
  {
    if(laser_beam[rem].status==1)
    {
    Matrices.model = glm::mat4(1.0f);
    int extra=laser_beam[rem].width;
    glm::mat4 translate_laser_beam_extra_matrix = glm::translate (glm::vec3(extra/2, 0, 0.0f)); // glTranslatef
    glm::mat4 translate_laser_beam_matrix = glm::translate (glm::vec3(laser_beam[rem].x, laser_beam[rem].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_laser_beam_matrix = glm::rotate((float)(laser_beam[rem].angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_laser_beam_matrix * rotate_laser_beam_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(laser_beam[rem].object);
    }
  }
  //check for reflection in mirrors
  for(int rem=0;rem<no_of_laser_beams;rem++)
  {
    if(laser_beam[rem].status==1)
    {for(int i=0;i<3;i++)
    {
      float a,b,c,d;
      a=(laser_beam[rem].x)+50*cos((laser_beam[rem].angle)*M_PI/180);
      b=(laser_beam[rem].y)+50*sin((laser_beam[rem].angle)*M_PI/180);
      //c=(laser_beam[rem].x)-25*cos((laser_beam[rem].angle)*M_PI/180);
      //d=(laser_beam[rem].y)-25*sin((laser_beam[rem].angle)*M_PI/180);

        if(check_for_intersection_with_mirror(laser_beam[rem].x,laser_beam[rem].y,a,b,i)){
          laser_beam[rem].x=x_inter;
          laser_beam[rem].y=y_inter;
          float mirror_angle;
          if(i==0)
            mirror_angle=120;
          else if(i==1)
            mirror_angle=150;
          else if(i==2)
            mirror_angle=30;

          laser_beam[rem].angle=((2*mirror_angle)-laser_beam[rem].angle);
        }
      }
    }}

  //movement of laser beam
  for(int rem=0;rem<no_of_laser_beams;rem++)
  {
    if(laser_beam[rem].status==1)
    {
      laser_beam[rem].x+=20*cos((laser_beam[rem].angle)*M_PI/180.0f);
      laser_beam[rem].y+=20*sin((laser_beam[rem].angle)*M_PI/180.0f);
    }
  }
  check_laser_bounds();
  check_collision_with_black_boxes();
  

  /*BLOCKS*/
  current_time1 = glfwGetTime(); // Time in seconds
  if ((current_time1 - last_update_time1) >= 2) 
  { 
            falling_blocks();
            last_update_time1 = current_time1;
            blocks[no_of_blocks].status=1;
            no_of_blocks++;
  }
  for(int rem=0;rem<no_of_blocks;rem++)
  {
    if(blocks[rem].status==1)
    {int cur=rem;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_block_matrix = glm::translate (glm::vec3(blocks[cur].x, blocks[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_block_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_block_matrix * rotate_block_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(blocks[cur].object);
  }
}

  for(int rem=0;rem<no_of_blocks;rem++)
        if(blocks[rem].status==1){
        blocks[rem].y-=block_speed;
      }
  check_collection_of_falling_blocks();
  //cout<<"score:"<<score<<endl;
}
  /*MIRRORS*/
  //mirror1
  for(int rem=0;rem<6;rem++)
  {
    float base_angle,strike_angle;  
    int cur=rem;
    if(cur==0)
      base_angle=120;
    else
      base_angle=-10;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_mirror1_extra_matrix = glm::translate (glm::vec3(250, 120, 0.0f)); // glTranslatef
    glm::mat4 translate_mirror1_matrix = glm::translate (glm::vec3(mirror1[cur].x, mirror1[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_mirror1_matrix = glm::rotate((float)(base_angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_mirror1_extra_matrix*translate_mirror1_matrix * rotate_mirror1_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(mirror1[cur].object);
  }
  //mirror2
  for(int rem=0;rem<6;rem++)
  {
    float base_angle,strike_angle;  
    int cur=rem;
    if(cur==0)
      base_angle=150;
    else
      base_angle=0;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_mirror2_extra_matrix = glm::translate (glm::vec3(-40, 210, 0.0f)); // glTranslatef
    glm::mat4 translate_mirror2_matrix = glm::translate (glm::vec3(mirror2[cur].x, mirror2[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_mirror2_matrix = glm::rotate((float)(base_angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_mirror2_extra_matrix*translate_mirror2_matrix * rotate_mirror2_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(mirror2[cur].object);
  }
  //mirror3
  for(int rem=0;rem<6;rem++)
  {
    float base_angle,strike_angle;  
    int cur=rem;
    if(cur==0)
      base_angle=30;
    else
      base_angle=0;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_mirror3_extra_matrix = glm::translate (glm::vec3(160, -140, 0.0f)); // glTranslatef
    glm::mat4 translate_mirror3_matrix = glm::translate (glm::vec3(mirror3[cur].x, mirror3[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_mirror3_matrix = glm::rotate((float)(base_angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_mirror3_extra_matrix*translate_mirror3_matrix * rotate_mirror3_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(mirror3[cur].object);
  }

  /*//mirror4
  for(int rem=0;rem<6;rem++)
  {
    float base_angle,strike_angle;  
    int cur=rem;
    if(cur==0)
      base_angle=60;
    else
      base_angle=0;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_mirror4_extra_matrix = glm::translate (glm::vec3(-40, 0, 0.0f)); // glTranslatef
    glm::mat4 translate_mirror4_matrix = glm::translate (glm::vec3(mirror4[cur].x, mirror4[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_mirror4_matrix = glm::rotate((float)(base_angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_mirror4_extra_matrix*translate_mirror4_matrix * rotate_mirror4_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(mirror4[cur].object);
  }
  */
  //"POINTS" label
  base_x=scoreLabel_x;
  base_y=scoreLabel_y;
  for(int z=0;z<scoreLabel.length();z++)
  {

  set_characters(scoreLabel[z],scoreLabelObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(scoreLabelObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_score_label_matrix = glm::translate (glm::vec3(base_x+scoreLabelObjects[cur].x, base_y+scoreLabelObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_score_label_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_score_label_matrix * rotate_score_label_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(scoreLabelObjects[cur].object);
    }
  }
  base_x+=17;
}
//score value label
  base_x=score_value_x;
  base_y=score_value_y;
  char score_value[100];
  int temp=score;
  if(score<0)
    {
      //score_value[0]='-';
      temp=-1*score;
    }
  
for(int i=0;i<100;i++)
  score_value[i]='.';
  score_value[3]='0';
  int x=3;
  while(temp)
  {
    score_value[x]=(temp%10)+'0';
    temp/=10;
    x--;
  }
  if(score<0)
    score_value[x]='-';
  
  for(int z=0;z<=3;z++)
  {
    if(score_value[z]=='.')
      continue;
    set_characters(score_value[z],score_valueObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(score_valueObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_score_value_matrix = glm::translate (glm::vec3(base_x+score_valueObjects[cur].x, base_y+score_valueObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_score_value_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_score_value_matrix * rotate_score_value_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(score_valueObjects[cur].object);
    }
  }
  base_x+=17;
}

// "LEVEL" label
base_x=level_value_x;
base_y=level_value_y;
char level_value[2];
for(int i=0;i<2;i++)
  level_value[i]='0';
  //level_value[1]='0';
   x=1;
  temp=level;
  while(temp)
  {
    level_value[x]=(temp%10)+'0';
    temp/=10;
    x--;
  }
for(int z=0;z<=1;z++)
  {
    set_characters(level_value[z],level_valueObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(level_valueObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_level_value_matrix = glm::translate (glm::vec3(base_x+level_valueObjects[cur].x, base_y+level_valueObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_level_value_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_level_value_matrix * rotate_level_value_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(level_valueObjects[cur].object);
    }
  }
  base_x+=17;
}


//"GAME OVER" label
  base_x=endLabel_x;
  base_y=endLabel_y;
  for(int z=0;z<endLabel.length();z++)
  {

  set_characters(endLabel[z],endLabelObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(endLabelObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_end_label_matrix = glm::translate (glm::vec3(base_x+endLabelObjects[cur].x, base_y+endLabelObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_end_label_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_end_label_matrix * rotate_end_label_matrix);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(endLabelObjects[cur].object);
    }
  }
  base_x+=48;
}


}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetCursorPosCallback(window, cursor_position);
    glfwSetScrollCallback(window, mousescroll);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /*different colors*/
    COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
    COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
    COLOR coingold = {255.0/255.0,223.0/255.0,0.0/255.0};
    COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
    COLOR lightred = {255.0/255.0,125.0/255.0,125.0/255.0};
    COLOR lightgreen = {57/255.0,230/255.0,0/255.0};
    COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
    COLOR black = {30/255.0,30/255.0,21/255.0};
    COLOR blue = {0,0,1};
    COLOR darkbrown = {46/255.0,46/255.0,31/255.0};
    COLOR lightbrown = {95/255.0,63/255.0,32/255.0};
    COLOR brown1 = {117/255.0,78/255.0,40/255.0};
    COLOR brown2 = {134/255.0,89/255.0,40/255.0};
    COLOR brown3 = {46/255.0,46/255.0,31/255.0};
    COLOR cratebrown = {153/255.0,102/255.0,0/255.0};
    COLOR cratebrown1 = {121/255.0,85/255.0,0/255.0};
    COLOR cratebrown2 = {102/255.0,68/255.0,0/255.0};
    COLOR skyblue2 = {113/255.0,185/255.0,209/255.0};
    COLOR skyblue1 = {123/255.0,201/255.0,227/255.0};
    COLOR skyblue = {132/255.0,217/255.0,245/255.0};
    COLOR cloudwhite = {229/255.0,255/255.0,255/255.0};
    COLOR cloudwhite1 = {204/255.0,255/255.0,255/255.0};
    COLOR lightpink = {255/255.0,122/255.0,173/255.0};
    COLOR darkpink = {255/255.0,51/255.0,119/255.0};
    COLOR white = {255/255.0,255/255.0,255/255.0};
    COLOR score = {117/255.0,78/255.0,40/255.0};
    
  // Create the models

	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	
  /*Background objects*/
  //walls
  createRectangle ("wall_upper",0,cratebrown2,cratebrown2,cratebrown2,cratebrown2,0,300,60,800,"background");
  createRectangle ("wall_left",0,cratebrown2,cratebrown2,cratebrown2,cratebrown2,-400,0,600,60,"background");
  createRectangle ("wall_right",0,cratebrown2,cratebrown2,cratebrown2,cratebrown2,400,0,600,60,"background");  
  createRectangle ("boundary_line",0,black,black,black,black,0,-220,2,800,"background");
	
  /*BASKETS*/
  //red_basket
  createRectangle ("red_basket",0,lightred,lightred,lightred,lightred,0,-260,60,90,"basket");
  createCircle ("red_basket_upper_circle",0,red,0,-231,45,1,"basket",1);
  createCircle ("red_basket_lower_circle",0,red,0,-286,45,1,"basket",1);
  //green_basket
  createRectangle ("green_basket",0,lightgreen,lightgreen,lightgreen,lightgreen,0,-260,60,90,"basket");
  createCircle ("green_basket_upper_circle",0,darkgreen,0,-231,45,1,"basket",1);
  createCircle ("green_basket_lower_circle",0,darkgreen,0,-286,45,1,"basket",1);

  /*LASER GUN*/
  //semi circle
  createCircle("laser_circle",0,cratebrown2,-370,0,30,0.5,"laser_gun",1);
  createCircle("laser_circle1",0,black,-370,0,15,0.5,"laser_gun",1);
  createCircle("laser_circle2",0,coingold,-370,0,7.5,0.5,"laser_gun",1);
  createRectangle("laser_rod",0,blue,blue,blue,blue,-345,0,10,50,"laser_gun");


  /*BLOCKS*/
  //in the function above

  /*MIRROR*/
  //mirror1
  
  createRectangle ("mirror_base",0,black,black,black,black,0,0,5,100,"mirror1");
  createRectangle ("mirror_strike1",1,black,black,black,black,-23,42.5+6.5,15,5,"mirror1");
  createRectangle ("mirror_strike2",2,black,black,black,black,-12.5,21.25+10,15,5,"mirror1");
  createRectangle ("mirror_strike3",3,black,black,black,black,0,10,15,5,"mirror1");
  createRectangle ("mirror_strike4",4,black,black,black,black,12.5,-21.25+10,15,5,"mirror1");
  createRectangle ("mirror_strike5",5,black,black,black,black,23,-42.5+8.5,15,5,"mirror1");
  //mirror2
  
  createRectangle ("mirror_base",0,black,black,black,black,0,0,5,100,"mirror2");         
  createRectangle ("mirror_strike1",1,black,black,black,black,-42.5,25+6.5,15,5,"mirror2");
  createRectangle ("mirror_strike2",2,black,black,black,black,-21.25,12.5+10,15,5,"mirror2");
  createRectangle ("mirror_strike3",3,black,black,black,black,0,10,15,5,"mirror2");
  createRectangle ("mirror_strike4",4,black,black,black,black,21.25,-12.5+10,15,5,"mirror2");
  createRectangle ("mirror_strike5",5,black,black,black,black,42.5,-25+8.5,15,5,"mirror2");
//mirror3
  
  createRectangle ("mirror_base",0,black,black,black,black,0,0,5,100,"mirror3");         
  createRectangle ("mirror_strike1",1,black,black,black,black,42.5,25-6.5,15,5,"mirror3");
  createRectangle ("mirror_strike2",2,black,black,black,black,21.25,12.5-10,15,5,"mirror3");
  createRectangle ("mirror_strike3",3,black,black,black,black,0,-10,15,5,"mirror3");
  createRectangle ("mirror_strike4",4,black,black,black,black,-21.25,-12.5-10,15,5,"mirror3");
  createRectangle ("mirror_strike5",5,black,black,black,black,-42.5,-25-8.5,15,5,"mirror3");
//mirror4
  
  createRectangle ("mirror_base",0,black,black,black,black,0,0,5,100,"mirror4");
  createRectangle ("mirror_strike1",1,black,black,black,black,23,42.5-8.5,15,5,"mirror4");
  createRectangle ("mirror_strike2",2,black,black,black,black,12.5,21.25-10,15,5,"mirror4");
  createRectangle ("mirror_strike3",3,black,black,black,black,0,-10,15,5,"mirror4");
  createRectangle ("mirror_strike4",4,black,black,black,black,-12.5,-21.25-10,15,5,"mirror4");
  createRectangle ("mirror_strike5",5,black,black,black,black,-23,-42.5-6.5,15,5,"mirror4");
  
 //mis_fire rectangles
  createRectangle ("box1",0,white,white,white,white,-355,255,20,20,"background");
  createRectangle ("box2",0,white,white,white,white,-330,255,20,20,"background");
  createRectangle ("box3",0,white,white,white,white,-305,255,20,20,"background");
  createRectangle ("box4",0,white,white,white,white,-280,255,20,20,"background");
  createRectangle ("box5",0,white,white,white,white,-255,255,20,20,"background");
 
  //ALL SEGMENT CHARACTERS
  int t;
    for(t=0;t<=3;t++)
    {
        string layer;
        float wid,heig,offset;
        wid=12;heig=4;offset=10;
        COLOR color = score;
      if(t==0)
        {
          layer="scorelabel";
          color=white;
        }
      if(t==1)
      {
        layer="endlabel";
        wid=40;heig=12;offset=30;color=white;
      }
      if(t==2)
      {
        layer="score_value";
      }
      if(t==3)
      {
        layer="level_value";

      }

        createRectangle("top",4,color,color,color,color,0,offset,heig,wid,layer);
        createRectangle("bottom",6,color,color,color,color,0,-offset,heig,wid,layer);
        createRectangle("middle",5,color,color,color,color,0,0,heig,wid,layer);
        createRectangle("left1",0,color,color,color,color,-offset/2,offset/2,wid,heig,layer);
        createRectangle("left2",1,color,color,color,color,-offset/2,-offset/2,wid,heig,layer);
        createRectangle("right1",2,color,color,color,color,offset/2,offset/2,wid,heig,layer);
        createRectangle("right2",3,color,color,color,color,offset/2,-offset/2,wid,heig,layer);
        createRectangle("middle1",7,color,color,color,color,0,offset/2,wid,heig,layer);
        createRectangle("middle2",8,color,color,color,color,0,-offset/2,wid,heig,layer);
     }  

  // Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (113/255.0,185/255.0,209/255.0,0.4f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
time_t old_time;

int main (int argc, char** argv)
{
	 int width = 800;
	 int height = 600;
    
    GLFWwindow* window = initGLFW(width, height);
    old_time = time(NULL);
    thread(play_audio,"./sounds/Audio.mp3").detach();

	initGL (window, width, height);
  /*char diff;
  scanf(" %c",&diff);
  if(diff=='E')
  {
    win_score=10;
  }
  else if(diff=='M')
  {
    win_score=20;
  }
  else if(diff=='H')
  {
    win_score=30;
  }*/

    double last_update_time = glfwGetTime(), current_time;

    glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        
        if((time(NULL)-old_time>=120) && (!game_over_var))
        {
        old_time=time(NULL);
        thread(play_audio,"./sounds/Audio.mp3").detach();
     }
        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
