#include <QMouseEvent>
#include <QGuiApplication>
#include <QFont>
#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Transformation.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1;

NGLScene::NGLScene(QWindow *_parent) : OpenGLWindow(_parent)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Directional Light");
  m_lightPosition.set(-2,2,3);

}


NGLScene::~NGLScene()
{
  ngl::NGLInit *Init = ngl::NGLInit::instance();
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  delete m_light;
  Init->NGLQuit();
}

void NGLScene::resizeEvent(QResizeEvent *_event )
{
  if(isExposed())
  {
  int w=_event->size().width();
  int h=_event->size().height();
  // set the viewport for openGL
  glViewport(0,0,w,h);
  // now set the camera size values as the screen size has changed
  m_cam->setShape(45,(float)w/h,0.05,350);
  m_text->setScreenSize(w,h);

  renderLater();
  }
}


void NGLScene::initialize()
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
  // we are creating a shader called DirectionalLight
  shader->createShaderProgram("DirectionalLight");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("DirectionalLightVertex",ngl::VERTEX);
  shader->attachShader("DirectionalLightFragment",ngl::FRAGMENT);
  // attach the source
  shader->loadShaderSource("DirectionalLightVertex","shaders/DirectionalLightVert.glsl");
  shader->loadShaderSource("DirectionalLightFragment","shaders/DirectionalLightFrag.glsl");
  // compile the shaders
  shader->compileShader("DirectionalLightVertex");
  shader->compileShader("DirectionalLightFragment");
  // add them to the program
  shader->attachShaderToProgram("DirectionalLight","DirectionalLightVertex");
  shader->attachShaderToProgram("DirectionalLight","DirectionalLightFragment");

  // now we have associated this data we can link the shader
  shader->linkProgramObject("DirectionalLight");
  // and make it active ready to load values
  (*shader)["DirectionalLight"]->use();
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,1,1);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_cam= new ngl::Camera(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam->setShape(45,(float)720.0/576.0,0.05,350);

  // now create our light we do this after the camera so we can pass the transpose
  // projection to the light as it's transform and specify the light in world space
  // all the shader calculations are done in eye space so this will take it into account
  m_light = new ngl::Light(m_lightPosition,ngl::Colour(1,1,1,1),ngl::DIRECTIONALLIGHT);
  //
  ngl::Mat4 iv=m_cam->getViewMatrix();
  m_light->setTransform(iv);
  // load these values to the shader as well
  m_light->loadToShader("light");


  // build the material for our shading model
  ngl::Material m;
  m.setAmbient(ngl::Colour(0.1,0.1,0.1,1.0));
  m.setDiffuse(ngl::Colour(1.0,0.4,0.1));
  m.setSpecular(ngl::Colour(1,1,1));
  m.setSpecularExponent(80);
  m.loadToShader("material");
  m_text = new  ngl::Text(QFont("Arial",14));
  m_text->setScreenSize(this->size().width(),this->size().height());
  glViewport(0,0,width(),height());

}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_mouseGlobalTX;
  MV=M*m_cam->getViewMatrix() ;
  MVP=MV*m_cam->getProjectionMatrix();
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("M",M);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParam3f("viewerPos",m_cam->getEye().m_x,m_cam->getEye().m_y,m_cam->getEye().m_z);
}

void NGLScene::render()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["DirectionalLight"]->use();
  m_light->setPosition(m_lightPosition);
  m_light->loadToShader("light");

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
  // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  // draw
  loadMatricesToShader();
  prim->draw("teapot");
  // now render the text using the QT renderText helper function
  m_text->setColour(ngl::Colour(1,1,1));
  m_text->renderText(10,18,"Use Arrow Keys to move Light i and o to move in and out");
  m_text->setColour(ngl::Colour(1,1,0));

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
    renderLater();

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
    renderLater();

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
	renderLater();
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
    renderLater();
}
