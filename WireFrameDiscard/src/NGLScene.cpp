#include <QMouseEvent>
#include <QGuiApplication>
#include <QFont>
#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1f;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Directional Light");
  m_lightPosition.set(0,2,2);

}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}


void NGLScene::resizeGL(int _w , int _h)
{
  m_project=ngl::perspective(45.0f,(float)_w/_h,0.05f,350.0f);
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
  // we are creating a shader called PerVertASD
  shader->createShaderProgram("PerVertASD");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("PerVertASDVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("PerVertASDFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
//  shader->loadShaderSource("PerVertASDVertex","shaders/PerVertASDVert.glsl");
  shader->loadShaderSource("PerVertASDVertex","shaders/PerVertASDVertFunc.glsl");
  shader->loadShaderSource("PerVertASDFragment","shaders/PerVertASDFrag.glsl");
  // compile the shaders
  shader->compileShader("PerVertASDVertex");
  shader->compileShader("PerVertASDFragment");
  // add them to the program
  shader->attachShaderToProgram("PerVertASD","PerVertASDVertex");
  shader->attachShaderToProgram("PerVertASD","PerVertASDFragment");

  // now we have associated this data we can link the shader
  shader->linkProgramObject("PerVertASD");
  // and make it active ready to load values
  (*shader)["PerVertASD"]->use();
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
  shader->setUniform("material.Ka",0.1f,0.1f,0.1f);
  // red diffuse
  shader->setUniform("material.Kd",0.8f,0.0f,0.0f);
  // white spec
  shader->setUniform("material.Ks",1.0f,1.0f,1.0f);
  shader->setUniform("material.shininess",100.0f);
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
  shader->setUniform("light.position",m_lightPosition);
  shader->setUniform("light.La",0.1f,0.1f,0.1f);
  shader->setUniform("light.Ld",1.0f,1.0f,1.0f);
  shader->setUniform("light.Ls",0.9f,0.9f,0.9f);
 // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,2,2);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_view=ngl::lookAt(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45,(float)720.0/576.0,0.05,350);
  m_text.reset(new  ngl::Text(QFont("Arial",14)));
  m_text->setScreenSize(width(),height());

}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV= m_view*m_mouseGlobalTX;
  MVP=m_project*MV ;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("MVP",MVP);
  shader->setUniform("MV",MV);
  shader->setUniform("normalMatrix",normalMatrix);
  shader->setUniform("light.position",m_lightPosition);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["PerVertASD"]->use();
  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotX*rotY;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
  // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  // draw
  loadMatricesToShader();
  prim->draw("teapot");
  // now render the text using the QT renderText helper function
   m_text->setColour(1,1,1);
   m_text->renderText(10,18,"Use Arrow Keys to move Light i and o to move in and out");
   m_text->setColour(1,1,0);

   QString text=QString("Light Position [%1,%2,%3]")
                       .arg(m_lightPosition.m_x,4,'f',1,'0')
                       .arg(m_lightPosition.m_y,4,'f',1,'0')
                       .arg(m_lightPosition.m_z,4,'f',1,'0');
   m_text->renderText(10,36,text );
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
  static float s_scale=200.0f;
  static float s_edge=0.2f;
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  shader->use("PerVertASD");

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
  case Qt::Key_Left : m_lightPosition.m_x-=0.5; break;
  case Qt::Key_Right : m_lightPosition.m_x+=0.5; break;
  case Qt::Key_Up : m_lightPosition.m_y+=0.5; break;
  case Qt::Key_Down : m_lightPosition.m_y-=0.5; break;
  case Qt::Key_I : m_lightPosition.m_z-=0.5; break;
  case Qt::Key_O : m_lightPosition.m_z+=0.5; break;
  case Qt::Key_Plus :
  {
    s_scale+=5;
    shader->setUniform("scale",s_scale);
  break;
  }
  case Qt::Key_Minus :
  {
    s_scale-=5;
    shader->setUniform("scale",s_scale);
  break;
  }
  case Qt::Key_1 :
  {
    s_edge-=0.1;
    shader->setUniform("edge",s_edge);
  break;
  }
  case Qt::Key_2 :
  {
    s_edge+=0.1;
    shader->setUniform("edge",s_edge);
  break;
  }

  default : break;
  }
  // finally update the GLWindow and re-draw
  update();
}
