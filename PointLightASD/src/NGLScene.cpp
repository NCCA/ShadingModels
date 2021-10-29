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
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // we are creating a shader called PointLightDiffuse
  ngl::ShaderLib::createShaderProgram("PointLightDiffuse");
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader("PointLightDiffuseVertex",ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("PointLightDiffuseFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  ngl::ShaderLib::loadShaderSource("PointLightDiffuseVertex","shaders/PointLightDiffuseVert.glsl");
  ngl::ShaderLib::loadShaderSource("PointLightDiffuseFragment","shaders/PointLightDiffuseFrag.glsl");
  // compile the shaders
  ngl::ShaderLib::compileShader("PointLightDiffuseVertex");
  ngl::ShaderLib::compileShader("PointLightDiffuseFragment");
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram("PointLightDiffuse","PointLightDiffuseVertex");
  ngl::ShaderLib::attachShaderToProgram("PointLightDiffuse","PointLightDiffuseFragment");
  // now bind the shader attributes for most NGL primitives we use the following
  // now we have associated this data we can link the shader
  ngl::ShaderLib::linkProgramObject("PointLightDiffuse");
  // and make it active ready to load values
  ngl::ShaderLib::use("PointLightDiffuse");


  ngl::Vec4 lightPos(-2.0f,5.0f,2.0f,0.0f);
  ngl::ShaderLib::setUniform("light.position",lightPos);
  ngl::ShaderLib::setUniform("light.ambient",0.0f,0.0f,0.0f,1.0f);
  ngl::ShaderLib::setUniform("light.diffuse",1.0f,1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("light.specular",0.8f,0.8f,0.8f,1.0f);
  ngl::ShaderLib::setUniform("material.ambient",0.274725f,0.1995f,0.0745f,0.0f);
  ngl::ShaderLib::setUniform("material.diffuse",0.75164f,0.60648f,0.22648f,0.0f);
  ngl::ShaderLib::setUniform("material.specular",0.628281f,0.555802f,0.3666065f,0.0f);
  ngl::ShaderLib::setUniform("material.shininess",51.2f);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  m_eye.set(0,0,3);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_view=ngl::lookAt(m_eye,to,up);
  ngl::ShaderLib::setUniform("viewerPos",m_eye);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45,720.0f/576.0f,0.05f,350);

  m_text=std::make_unique<ngl::Text>("fonts/Arial.ttf",14); 
  m_text->setScreenSize(width(),height());
  glViewport(0,0,width(),height());
}


void NGLScene::loadMatricesToShader()
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  MV=m_view*m_mouseGlobalTX*M;
  MVP=m_project*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MV",MV);
  ngl::ShaderLib::setUniform("M",M);
  ngl::ShaderLib::setUniform("MVP",MVP);
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
  ngl::ShaderLib::setUniform("viewerPos",m_eye);
  ngl::ShaderLib::setUniform("light.position",m_lightPosition);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ngl::ShaderLib::use("PointLightDiffuse");

  // Rotation based on the mouse position for our global transform
  ngl::Transformation trans;
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
  // draw
  loadMatricesToShader();
  ngl::VAOPrimitives::draw("teapot");
  // now render the text using the QT renderText helper function
  m_text->setColour(1,1,1);
  m_text->renderText(10,700,"Use Arrow Keys to move Light i and o to move in and out");
  m_text->setColour(1,1,0);

  std::string text=fmt::format("Light Position [{:0.4f},{:0.4f},{:0.4f}]",m_lightPosition.m_x,m_lightPosition.m_y,m_lightPosition.m_z);
  m_text->renderText(10,680,text );
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
  case Qt::Key_Left : m_lightPosition.m_x-=0.5; break;
  case Qt::Key_Right : m_lightPosition.m_x+=0.5; break;
  case Qt::Key_Up : m_lightPosition.m_y+=0.5; break;
  case Qt::Key_Down : m_lightPosition.m_y-=0.5; break;
  case Qt::Key_I : m_lightPosition.m_z-=0.5; break;
  case Qt::Key_O : m_lightPosition.m_z+=0.5; break;

  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
