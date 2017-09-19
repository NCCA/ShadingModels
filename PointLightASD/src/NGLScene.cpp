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
  // we are creating a shader called PointLightDiffuse
  shader->createShaderProgram("PointLightDiffuse");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("PointLightDiffuseVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("PointLightDiffuseFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("PointLightDiffuseVertex","shaders/PointLightDiffuseVert.glsl");
  shader->loadShaderSource("PointLightDiffuseFragment","shaders/PointLightDiffuseFrag.glsl");
  // compile the shaders
  shader->compileShader("PointLightDiffuseVertex");
  shader->compileShader("PointLightDiffuseFragment");
  // add them to the program
  shader->attachShaderToProgram("PointLightDiffuse","PointLightDiffuseVertex");
  shader->attachShaderToProgram("PointLightDiffuse","PointLightDiffuseFragment");
  // now bind the shader attributes for most NGL primitives we use the following
  // now we have associated this data we can link the shader
  shader->linkProgramObject("PointLightDiffuse");
  // and make it active ready to load values
  (*shader)["PointLightDiffuse"]->use();
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,0,2);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45,(float)720.0/576.0,0.05,350);

  // now create our light we need to do this after the camera as we need to
  // load the projection to the light transform
  m_light.reset( new ngl::Light(m_lightPosition,ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT));
  ngl::Mat4 iv=m_cam.getViewMatrix();
  iv.transpose();
  // we load the transpose of the projection matrix to the light, the light
  // position is then transformed by the matrix before being loaded to the
  // shader
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
  m_text.reset(  new  ngl::Text(QFont("Arial",14)));
  m_text->setScreenSize(width(),height());
  glViewport(0,0,width(),height());
}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  MV=M*m_mouseGlobalTX*m_cam.getViewMatrix();
  MVP=MV*m_cam.getProjectionMatrix();
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setUniform("MV",MV);
  shader->setUniform("M",M);
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
  shader->setUniform("viewerPos",m_cam.getEye().toVec3());
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["PointLightDiffuse"]->use();
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
