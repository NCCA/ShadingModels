#include <QMouseEvent>
#include <QGuiApplication>
#include <QFont>
#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <boost/format.hpp>
#include <ngl/NGLStream.h>

//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Multiple Point Lights Using Uniform Interface Blocks");

}


void APIENTRY openglCallbackFunction(GLenum source,
                                           GLenum type,
                                           GLuint id,
                                           GLenum severity,
                                           GLsizei length,
                                           const GLchar* message,
                                           const void* userParam){

    std::cout << "---------------------opengl-callback-start------------" << std::endl;
    std::cout << "message: "<< message << std::endl;
    std::cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "OTHER";
        break;
    }
    std::cout << std::endl;

    std::cout << "id: " << id << std::endl;
    std::cout << "severity: ";
    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "HIGH";
        break;
    }
    std::cout << std::endl;
    std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}



NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(QResizeEvent *_event)
{
  m_width=_event->size().width()*devicePixelRatio();
  m_height=_event->size().height()*devicePixelRatio();
  // now set the camera size values as the screen size has changed
  m_cam.setShape(45.0f,(float)width()/height(),0.05f,350.0f);
}

void NGLScene::resizeGL(int _w , int _h)
{
  m_cam.setShape(45.0f,(float)_w/_h,0.05f,350.0f);
  m_width=_w*devicePixelRatio();
  m_height=_h*devicePixelRatio();
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called MultipleLights
  shader->createShaderProgram("MultipleLights");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("MultipleLightsVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("MultipleLightsFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("MultipleLightsVertex","shaders/MultiplePointLightVert.glsl");
  shader->loadShaderSource("MultipleLightsFragment","shaders/MultiplePointLightFrag.glsl");
  // compile the shaders
  shader->compileShader("MultipleLightsVertex");
  shader->compileShader("MultipleLightsFragment");
  // add them to the program
  shader->attachShaderToProgram("MultipleLights","MultipleLightsVertex");
  shader->attachShaderToProgram("MultipleLights","MultipleLightsFragment");

  // now we have associated this data we can link the shader
  shader->linkProgramObject("MultipleLights");
  // and make it active ready to load values
  (*shader)["MultipleLights"]->use();

  // now we have associated this data we can link the shader
  shader->linkProgramObject("MultipleLights");
  shader->printRegisteredUniforms("MultipleLights");
  // and make it active ready to load values
  (*shader)["MultipleLights"]->use();
  // now we need to set the material and light values
  /*
   *struct MaterialInfo
   {
        // Ambient reflectivity
        vec3 Ka;
        // Diffuse reflectivity
        vec3 Kd;
        // Specular reflectivity
        vec3 Ks;
        // Specular shininess factor
        float shininess;
  };*/
  shader->setUniform("material.Ka",0.4f,0.4f,0.4f);
  // red diffuse
  shader->setUniform("material.Kd",1.0f,1.0f,1.0f);
  // white spec
  shader->setUniform("material.Ks",1.0f,1.0f,1.0f);
  shader->setUniform("material.shininess",20.0f);
  // now for  the lights values (all set to white)
  /*struct LightInfo
  {
  // Light position in eye coords.
  vec4 position;
  // Ambient light intensity
  vec3 La;
  // Diffuse light intensity
  vec3 Ld;
  // Specular light intensity
  vec3 Ls;
  };*/
  // this data was generated by my light dome tool
  GLfloat positions[]={
                        -4.279, 2.06819, 1.5533, 4.86468, 0.0153398, 1.15529,
                        -0.79594, 4.92105, 0.386964,0.859757, 4.34285, -2.32388,
                        2.34462, 3.39738 ,2.82145 , 1.71452 ,4.67046, 0.497201 ,
                        0.875125, 4.73547, 1.34516, 0.481648, 4.8963, -0.89121,
                        -1.22515, 3.38046, 3.4744,4.70511, 0.0153398, -1.69166,
                        4.49673, 0.00766949, -2.18618,-4.52646, 0.0153398, -2.12389,
                        1.27728 ,4.28864, 2.23072, 0.641939, 4.95855, 0.0266027,
                        0.994878, 4.81477, 0.910079 ,-0.434167, 4.94811, -0.572456,
                        -2.34066, 4.28074, 1.0939, 2.48517, 4.29258 ,0.630612 ,
                          0.0659806 ,4.82697, 1.3023 , -1.2889 ,3.38046, -3.45126
                      };

  GLfloat colours[]={
                      0.0294922, 0.0140869, 0.00583496 ,0.0105774, 0.0153564, 0.0223511 ,
                      0.0307617, 0.0132324, 0.00444031 ,0.00517273, 0.00758057, 0.0123474 ,
                      0.00786743, 0.0118896, 0.0183716 ,0.0609863, 0.0765137, 0.0846191 ,
                      0.0485596, 0.0646484, 0.0788574 ,0.0232666, 0.0159668, 0.0126709 ,
                      0.0133301, 0.00983276, 0.00909424 ,0.00621948, 0.00994873, 0.0161011 ,
                      0.00406799, 0.00239716, 0.0013855,0.0313721, 0.0149292, 0.0061554 ,
                      0.0305176, 0.0415771, 0.0527344 ,0.0296875, 0.0298828, 0.0276367 ,
                      0.067334, 0.0844238, 0.0954102 ,0.0060791, 0.0039917, 0.00276794 ,
                      0.0229614, 0.0107727, 0.00412598 ,0.035376, 0.04646, 0.0565918 ,
                      0.0148682, 0.0121704, 0.0106323 ,0.0171753, 0.00960693, 0.00534973
                    };
  int index=0;
  for(int i=0; i<20; ++i)
  {
    // char name[50];
    // sprintf(name,"light[%d]",i);
    std::string name=boost::str(boost::format("light[%d]") % i );
    shader->setUniform(name+".position",positions[index],positions[index+1],positions[index+2]);
    shader->setUniform(name+".Ld",colours[index],colours[index+1],colours[index+2]);
    shader->setUniform(name+".Ls",colours[index],colours[index+1],colours[index+2]);

    shader->setUniform(name+".La",0.0f,0.0f,0.0f);

    index+=3;
  }
  if (GLEW_ARB_program_interface_query)
  {
    std::cout<<"vertex program \n";
  }
  // grab the index into the shader block for the uniforms will use this
  // in the load matrices to shader routine.
  m_transformsIndex = shader->getUniformBlockIndex("transforms");
  // generate a buffer ID for the transfom
  glGenBuffers( 1, &m_transformUboHandle );

  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,0.5,2);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45.0f,(float)720.0f/576.0f,0.05f,350.0f);
  ngl::VAOPrimitives *prim = ngl::VAOPrimitives::instance();
  prim->createTrianglePlane("plane",30,30,20,20,ngl::Vec3(0,1,0));
  shader->printProperties();
  shader->printRegisteredUniforms("MultipleLights");

 /* {
    auto prog=shader->getProgramID("MultipleLights");
    GLint numBlocks = 0;
    glGetProgramInterfaceiv(prog, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
    const GLenum blockProperties[1] = {GL_NUM_ACTIVE_VARIABLES};
    const GLenum activeUnifProp[1] = {GL_ACTIVE_VARIABLES};
    const GLenum unifProperties[3] = {GL_NAME_LENGTH, GL_TYPE , GL_LOCATION};

    for(int blockIx = 0; blockIx < numBlocks; ++blockIx)
    {
      GLint numActiveUnifs = 0;
      glGetProgramResourceiv(prog, GL_UNIFORM_BLOCK, blockIx, 1, blockProperties, 1, NULL, &numActiveUnifs);

      if(!numActiveUnifs)
        continue;

      std::vector<GLint> blockUnifs(numActiveUnifs);
      glGetProgramResourceiv(prog, GL_UNIFORM_BLOCK, blockIx, 1, activeUnifProp, numActiveUnifs, NULL, &blockUnifs[0]);

      for(int unifIx = 0; unifIx < numActiveUnifs; ++unifIx)
      {
        GLint values[3];
        glGetProgramResourceiv(prog, GL_UNIFORM, blockUnifs[unifIx], 3, unifProperties, 3, NULL, values);

        //Get the name. Must use a std::vector rather than a std::string for C++03 standards issues.
        //C++11 would let you use a std::string directly.
        std::vector<char> nameData(values[0]);
        glGetProgramResourceName(prog, GL_UNIFORM, blockUnifs[unifIx], nameData.size(), NULL, &nameData[0]);
        std::string name(nameData.begin(), nameData.end() - 1);
      }
    }

  }*/



}


void NGLScene::loadMatricesToShader()
{
  struct transform
  {
    ngl::Mat4 MVP;
    ngl::Mat4 MV;
    ngl::Mat4 normalMatrix;
  };

  transform t;
  t.MV=m_cam.getViewMatrix()*
       m_mouseGlobalTX*
       m_transform.getMatrix();
  t.MVP=m_cam.getProjectionMatrix()*t.MV;
  t.normalMatrix=t.MV;
  t.normalMatrix.inverse().transpose();
  std::cout<<"transformUBO ID "<<m_transformUboHandle<<"\n";
  glBindBuffer( GL_UNIFORM_BUFFER, m_transformUboHandle );
  glBufferData( GL_UNIFORM_BUFFER, sizeof(transform),t.MVP.openGL(),GL_DYNAMIC_DRAW );
  glBindBufferBase( GL_UNIFORM_BUFFER, m_transformsIndex, m_transformUboHandle );
  glUnmapBuffer(GL_UNIFORM_BUFFER);

}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["MultipleLights"]->use();
  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
  // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  // draw
  m_transform.reset();

  loadMatricesToShader();
  prim->draw("teapot");
  m_transform.setPosition(0,-0.45,0);
  loadMatricesToShader();
  prim->draw("plane");

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    update();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
	}
	update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;

  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
