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
const static float ZOOM=0.1;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Directional Light / Point Light");
  m_lightPos.set(0,2,0);

}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL(int _w , int _h)
{
  m_project=ngl::perspective(45.0f,(float)width()/height(),0.05f,350.0f);
  m_width=_w*devicePixelRatio();
  m_height=_h*devicePixelRatio();
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // we are creating a shader called PointDirLight
  ngl::ShaderLib::createShaderProgram("PointDirLight");
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader("PointDirLightVertex",ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("PointDirLightFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  ngl::ShaderLib::loadShaderSource("PointDirLightVertex","shaders/PointDirLightVert.glsl");
  ngl::ShaderLib::loadShaderSource("PointDirLightFragment","shaders/PointDirLightFrag.glsl");
  // compile the shaders
  ngl::ShaderLib::compileShader("PointDirLightVertex");
  ngl::ShaderLib::compileShader("PointDirLightFragment");
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram("PointDirLight","PointDirLightVertex");
  ngl::ShaderLib::attachShaderToProgram("PointDirLight","PointDirLightFragment");

  // now we have associated this data we can link the shader
  ngl::ShaderLib::linkProgramObject("PointDirLight");
  // and make it active ready to load values
  ngl::ShaderLib::use("PointDirLight");
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
  ngl::ShaderLib::setUniform("material.Ka",0.1f,0.1f,0.1f);
  // red diffuse
  ngl::ShaderLib::setUniform("material.Kd",0.8f,0.0f,0.0f);
  // white spec
  ngl::ShaderLib::setUniform("material.Ks",1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("material.shininess",100.0f);
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
  ngl::ShaderLib::setUniform("light.position",m_lightPos);
  ngl::ShaderLib::setUniform("light.La",0.1f,0.1f,0.1f);
  ngl::ShaderLib::setUniform("light.Ld",1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("light.Ls",0.9f,0.9f,0.9f);

  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,1,2);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_view=ngl::lookAt(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45,720.0f/576.0f,0.05f,350);
  m_text = std::make_unique<ngl::Text>("fonts/Arial.ttf",14);
  m_text->setScreenSize(width(),height());

}


void NGLScene::loadMatricesToShader()
{

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_transform.getMatrix();
  MV= m_view*M;
  MVP=m_project*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP",MVP);
  ngl::ShaderLib::setUniform("MV",MV);
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
  ngl::ShaderLib::setUniform("light.position",m_lightPos);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);
  ngl::ShaderLib::use("PointDirLight");
  // Rotation based on the mouse position for our global transform
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  ngl::Mat4 final=rotY*rotX;
  // add the translations
  final.m_m[3][0] = m_modelPos.m_x;
  final.m_m[3][1] = m_modelPos.m_y;
  final.m_m[3][2] = m_modelPos.m_z;
  // set this in the TX stack
  m_transform.setMatrix(final);
  // draw
  loadMatricesToShader();
  ngl::VAOPrimitives::draw("teapot");
  // now render the text using the QT renderText helper function
   m_text->setColour(ngl::Vec3(1,1,1));
   m_text->renderText(10,680,"Use Arrow Keys to move Light i and o to move in and out P & D for Light Type");
   m_text->setColour(ngl::Vec3(1,1,0));

   std::string text=fmt::format("Light Position [{:0.4f},{:0.4f},{:0.4f}] {}",m_lightPos.m_x,m_lightPos.m_y,m_lightPos.m_z,m_lightPos.m_w ? "Point Light " : "Directional Light");
   m_text->renderText(10,700,text );

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
	if(_event->angleDelta().x() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->angleDelta().x() <0 )
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
  case Qt::Key_Left : m_lightPos.m_x-=0.5; break;
  case Qt::Key_Right : m_lightPos.m_x+=0.5; break;
  case Qt::Key_Up : m_lightPos.m_y+=0.5; break;
  case Qt::Key_Down : m_lightPos.m_y-=0.5; break;
  case Qt::Key_I : m_lightPos.m_z-=0.5; break;
  case Qt::Key_O : m_lightPos.m_z+=0.5; break;
  case Qt::Key_P : m_lightPos.m_w=1.0; break;
  case Qt::Key_D : m_lightPos.m_w=0.0; break;

  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
