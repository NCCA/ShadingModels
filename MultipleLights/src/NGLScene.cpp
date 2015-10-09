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

}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{
  // set the viewport for openGL we need to take into account retina display
  // etc by using the pixel ratio as a multiplyer
  glViewport(0,0,_w*devicePixelRatio(),_h*devicePixelRatio());
  // now set the camera size values as the screen size has changed
  m_cam->setShape(45.0f,(float)width()/height(),0.05f,350.0f);
  m_text->setScreenSize(_w,_h);
  update();
}



void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called MultipleLights
  shader->createShaderProgram("MultipleLights");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("MultipleLightsVertex",ngl::VERTEX);
  shader->attachShader("MultipleLightsFragment",ngl::FRAGMENT);
  // attach the source
  shader->loadShaderSource("MultipleLightsVertex","shaders/MultipleLightsVert.glsl");
  shader->loadShaderSource("MultipleLightsFragment","shaders/MultipleLightsFrag.glsl");
  // compile the shaders
  shader->compileShader("MultipleLightsVertex");
  shader->compileShader("MultipleLightsFragment");
  // add them to the program
  shader->attachShaderToProgram("MultipleLights","MultipleLightsVertex");
  shader->attachShaderToProgram("MultipleLights","MultipleLightsFragment");
  // now bind the shader attributes for most NGL primitives we use the following
  // layout attribute 0 is the vertex data (x,y,z)
  // now we have associated this data we can link the shader
  shader->linkProgramObject("MultipleLights");
  // and make it active ready to load values
  (*shader)["MultipleLights"]->use();
  shader->autoRegisterUniforms("MultipleLights");
  shader->printRegisteredUniforms("MultipleLights");
  shader->setShaderParam1f("Normalize",1);


  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,2,4);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_cam= new ngl::Camera(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam->setShape(45,(float)720.0/576.0,0.05,350);

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",0.5,50);

  prim->createCylinder("cylinder",0.5,1.4,40,40);

  prim->createCone("cone",0.5,1.4,20,20);

  prim->createDisk("disk",0.8,120);
  prim->createTorus("torus",0.15,0.4,40,40);
  prim->createTrianglePlane("plane",14,14,80,80,ngl::Vec3(0,1,0));


  // now create our light
  m_pointLight = new ngl::Light(ngl::Vec3(3,3,2),ngl::Colour(1,1,1,1),ngl::POINTLIGHT);
  // load these values to the shader as well
  m_pointLight->setAttenuation(1.2,0,0);
  m_pointLight->enable();
  ngl::Mat4 iv=m_cam->getViewMatrix();
  iv.transpose();
  m_pointLight->setTransform(iv);

  m_spot = new ngl::SpotLight(ngl::Vec3(0.2,0,2),ngl::Vec3(0.2,0,0),ngl::Colour(1,1,1));
  m_spot->setSpecColour(ngl::Colour(1,1,1,1));
  m_spot->setExponent(1);
  m_spot->setCutoff(12);
  m_spot->setInnerCutoff(1);
  m_spot->enable();
  m_spot->setAttenuation(1.2,0.0,0.0);
  m_spot->setTransform(iv);

  // now create our light
  m_directionalLight = new ngl::Light(ngl::Vec3(-1,0,0.5),ngl::Colour(1,1,1,1),ngl::DIRECTIONALLIGHT);
  // load these values to the shader as well
  m_directionalLight->setAttenuation(1.0f,0.0f,0.0f);
  m_directionalLight->enable();
  m_directionalLight->setTransform(iv);

  m_text = new  ngl::Text(QFont("Arial",14));
  m_text->setScreenSize(this->size().width(),this->size().height());
  glViewport(0,0,width(),height());


}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["MultipleLights"]->use();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_transform.getMatrix();
  MV=m_transform.getMatrix()
     *m_mouseGlobalTX*m_cam->getViewMatrix() ;
  MVP=MV*m_cam->getProjectionMatrix();
  normalMatrix=MV;
  normalMatrix.inverse();
  normalMatrix.transpose();
  shader->setRegisteredUniform("MV",MV);
  shader->setRegisteredUniform("M",M);
  shader->setRegisteredUniform("MVP",MVP);
  shader->setRegisteredUniform("normalMatrix",normalMatrix);
  shader->setRegisteredUniform("viewerPos",m_cam->getEye().toVec3());

}


void NGLScene::drawScene(const std::string &_shader)
{
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)[_shader]->use();

  m_pointLight->loadToShader("light[0]");
  m_spot->loadToShader("light[1]");
  m_directionalLight->loadToShader("light[2]");


   // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::Material m(ngl::POLISHEDSILVER);
  m_transform.reset();
  {
    loadMatricesToShader();
    m.loadToShader("material");
    prim->draw("teapot");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(-3,0.0,0.0));
    loadMatricesToShader();
    m.change(ngl::BLACKPLASTIC);
    m.loadToShader("material");
    prim->draw("sphere");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(3,0.0,0.0));
    loadMatricesToShader();
    m.change(ngl::BRASS);
    m.loadToShader("material");
    prim->draw("cylinder");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(0.0,0.0,3.0));
    loadMatricesToShader();
    m.change(ngl::BRONZE);
    m.loadToShader("material");
    prim->draw("cube");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(-3.0,0.0,3.0));
    loadMatricesToShader();
    m.change(ngl::CHROME);
    m.loadToShader("material");
    prim->draw("torus");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(3.0,0.5,3.0));
    loadMatricesToShader();
    m.change(ngl::COPPER);
    m.loadToShader("material");
    prim->draw("icosahedron");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(0.0,0.0,-3.0));
    loadMatricesToShader();
    m.change(ngl::GOLD);
    m.loadToShader("material");
    prim->draw("cone");
  }


  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(-3.0,0.5,-3.0));
    loadMatricesToShader();
    m.change(ngl::PEWTER);
    m.loadToShader("material");
    prim->draw("tetrahedron");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(3.0,0.5,-3.0));
    loadMatricesToShader();
    m.change(ngl::SILVER);
    m.loadToShader("material");
    prim->draw("octahedron");
  }


  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(0.0,0.5,-6.0));
    loadMatricesToShader();
    m.change(ngl::POLISHEDSILVER);
    m.loadToShader("material");
    prim->draw("football");
  }
  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(-3.0,0.5,-6.0));
    m_transform.setRotation(0,180,0);
    loadMatricesToShader();
    m.change(ngl::COPPER);
    m.loadToShader("material");
    prim->draw("disk");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(3.0,0.5,-6.0));
    loadMatricesToShader();
    m.change(ngl::BLACKPLASTIC);
    m.loadToShader("material");
    prim->draw("dodecahedron");
  }
  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(1.0,0.35,1.0));
    m_transform.setScale(1.5,1.5,1.5);
    m.change(ngl::PEWTER);
    m.loadToShader("material");
    loadMatricesToShader();
    prim->draw("troll");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(-1.0,-0.5,1.0));
    m_transform.setScale(0.1,0.1,0.1);
    loadMatricesToShader();
    m.change(ngl::COPPER);
    m.loadToShader("material");
    prim->draw("dragon");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(-2.5,-0.5,1.0));
    m_transform.setScale(0.1,0.1,0.1);
    loadMatricesToShader();
    m.change(ngl::CHROME);
    m.loadToShader("material");
    prim->draw("buddah");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(2.5,-0.5,1.0));
    m_transform.setScale(0.1,0.1,0.1);
    loadMatricesToShader();
    m.change(ngl::BRONZE);
    m.loadToShader("material");
    prim->draw("bunny");
  }

  m_transform.reset();
  {
    m_transform.setPosition(ngl::Vec3(0.0,-0.5,0.0));
    loadMatricesToShader();
    prim->draw("plane");
  }

}


void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["MultipleLights"]->use();

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
  drawScene("MultipleLights");
  // now render the text using the QT renderText helper function
  m_text->setColour(ngl::Colour(1,1,1));
  m_text->renderText(10,18,"Point Light 1 On 2 Off");
  m_text->renderText(10,36,"SpotLight Light 3 On 4 Off");
  m_text->renderText(10,52,"Directional Light 5 On 6 Off");
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
  case Qt::Key_1 : m_pointLight->enable(); break;
  case Qt::Key_2 : m_pointLight->disable(); break;
  case Qt::Key_3 : m_spot->enable(); break;
  case Qt::Key_4 : m_spot->disable(); break;
  case Qt::Key_5 : m_directionalLight->enable(); break;
  case Qt::Key_6 : m_directionalLight->disable(); break;
   default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
