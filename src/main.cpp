#include <iostream>
#include <memory>
#include <exception>

#include <Ogre.h>
#include <OIS.h>

class App {
	public:
		App();
		~App();
		void run();
	private:
		void initResources();
		void initInput();
		void createCube();
		void createTerrain();
		std::unique_ptr<Ogre::Root> mRoot;
		Ogre::RenderWindow* mWindow;
		Ogre::SceneManager* mScene;
		Ogre::SceneNode* mRootNode;
		Ogre::Camera* mCamera;
		Ogre::SceneNode* mCamNode;
		Ogre::Viewport* mViewport;
		Ogre::String resourcename;
		OIS::InputManager* mInputManager;
		OIS::Keyboard* mKeyboard;
};

App::App()
	: mRoot(new Ogre::Root("", "", "")),
	resourcename("Resources")
{
	mRoot->loadPlugin(OGRE_PLUGIN_DIR "/RenderSystem_GL");
	mRoot->loadPlugin(OGRE_PLUGIN_DIR "/Plugin_OctreeSceneManager");
	const Ogre::RenderSystemList& rsys = mRoot->getAvailableRenderers();
	if(rsys.size() == 0) {
		throw std::runtime_error("No rendersystems found.\n");
	}
	else {
		mRoot->setRenderSystem(rsys[0]);
		mRoot->initialise(false, "", "");
		Ogre::NameValuePairList params;
		params["FSAA"] = "0";
		params["vsync"] = "true";
		mWindow = mRoot->createRenderWindow("Army", 800, 600, false, &params);
		if(!mWindow) {
			mRoot.reset();
			throw std::runtime_error("Could not create the render mWindow.\n");
		}
		mScene = mRoot->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
		mRootNode = mScene->getRootSceneNode();
		mCamera = mScene->createCamera("Camera");
		mCamNode = mRootNode->createChildSceneNode("CameraNode");
		mCamNode->attachObject(mCamera);
		mViewport = mWindow->addViewport(mCamera);
		mViewport->setBackgroundColour(Ogre::ColourValue(1, 1, 1));
		mCamera->setAspectRatio(float(mViewport->getActualWidth()) / float(mViewport->getActualHeight()));
		mCamera->setNearClipDistance(1.5f);
		mCamera->setFarClipDistance(3000.0f);
		mCamera->setPosition(0, 0, 100.0f);
		mCamera->lookAt(0, 0, 0);

		initResources();

		initInput();

		createCube();
		createTerrain();
	}
}

void App::initResources()
{
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup(resourcename);
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("share", "FileSystem", resourcename, false);
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(resourcename);
	Ogre::ResourceGroupManager::getSingleton().loadResourceGroup(resourcename);
}

void App::initInput()
{
	size_t hWnd = 0;
	mWindow->getCustomAttribute("WINDOW", &hWnd);
	mInputManager = OIS::InputManager::createInputSystem(hWnd);
	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, false));
}

void App::createCube()
{
	Ogre::Entity* cubeEntity = mScene->createEntity("Cube.mesh");
	Ogre::SceneNode* cubeNode = mRootNode->createChildSceneNode();
	cubeNode->attachObject(cubeEntity);
	cubeNode->translate(1, 1, 1);
}

void App::createTerrain()
{
	Ogre::Plane plane;
	plane.normal = Ogre::Vector3::UNIT_Z;
	plane.d = 0.0f;
	Ogre::MeshManager::getSingleton().createPlane("terrain1",
			resourcename, plane, 128, 128, 4, 4, true,
			1, 1.0f, 1.0f, Ogre::Vector3::UNIT_Y);
	Ogre::Entity* planeEnt = mScene->createEntity("plane1", "terrain1");
	planeEnt->setMaterialName("Cube");
	planeEnt->setCastShadows(false);
	mRootNode->createChildSceneNode()->attachObject(planeEnt);
}

void App::run()
{
	while(!mWindow->isClosed()) {
		mRoot->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
		mKeyboard->capture();
		if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
			break;
		if(mKeyboard->isKeyDown(OIS::KC_UP))
			mCamNode->translate(0, 0.1, 0);
		if(mKeyboard->isKeyDown(OIS::KC_DOWN))
			mCamNode->translate(0, -0.1, 0);
		if(mKeyboard->isKeyDown(OIS::KC_LEFT))
			mCamNode->translate(-0.1, 0, 0);
		if(mKeyboard->isKeyDown(OIS::KC_RIGHT))
			mCamNode->translate(0.1, 0, 0);
	}
}

App::~App()
{
	mWindow->removeAllViewports();
	mScene->destroyAllCameras();
	mScene->destroyAllEntities();
	mRootNode->removeAndDestroyAllChildren();
}

int main(int argc, char** argv)
{
	try {
		App app;
		app.run();
	} catch (Ogre::Exception& e) {
		std::cerr << "Ogre exception: " << e.what() << std::endl;
		return 1;
	} catch (std::exception& e) {
		std::cerr << "std::exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
