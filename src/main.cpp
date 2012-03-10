#include <iostream>
#include <memory>
#include <exception>

#include <Ogre.h>

class App {
	public:
		App();
		~App();
		void run();
	private:
		std::unique_ptr<Ogre::Root> mRoot;
		Ogre::RenderWindow* mWindow;
		Ogre::SceneManager* mScene;
		Ogre::SceneNode* mRootNode;
		Ogre::Camera* mCamera;
		Ogre::SceneNode* mCamNode;
		Ogre::Viewport* mViewport;
};

App::App()
	: mRoot(new Ogre::Root("", "", ""))
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

		Ogre::String resourcename("Resources");
		Ogre::ResourceGroupManager::getSingleton().createResourceGroup(resourcename);
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation("share", "FileSystem", resourcename, false);
		Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(resourcename);
		Ogre::ResourceGroupManager::getSingleton().loadResourceGroup(resourcename);

		Ogre::Entity* cubeEntity = mScene->createEntity("Cube.mesh");
		Ogre::SceneNode* cubeNode = mRootNode->createChildSceneNode();
		cubeNode->attachObject(cubeEntity);
		cubeNode->translate(0, 0, -20);
	}
}

void App::run()
{
	while(!mWindow->isClosed()) {
		mRoot->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
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

