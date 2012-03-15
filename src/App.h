#ifndef APP_H
#define APP_H

#include <string>
#include <memory>

#include <Ogre.h>
#include <OIS.h>

#include "Terrain.h"
#include "Papaya.h"

class App : public OIS::KeyListener, public PapayaEventListener {
	public:
		App();
		~App();
		void run();
		bool keyPressed(const OIS::KeyEvent &arg);
		bool keyReleased(const OIS::KeyEvent &arg);
		void PlatoonStatusChanged(const Platoon* p);
	private:
		void initResources();
		void initInput();
		void createUnitMesh();
		void createTerrain();
		void createTerrainTextures();
		void updateTerrain();
		void createTexture(const std::string& name, size_t width, size_t height,
				std::function<std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8> (size_t, size_t)> func);
		std::unique_ptr<Ogre::Root> mRoot;
		Ogre::RenderWindow* mWindow;
		Ogre::SceneManager* mScene;
		Ogre::SceneNode* mRootNode;
		Ogre::Camera* mCamera;
		Ogre::SceneNode* mCamNode;
		Ogre::Viewport* mViewport;
		OIS::InputManager* mInputManager;
		OIS::Keyboard* mKeyboard;
		std::string mUserDataDir;
		Terrain mTerrain;
		bool mRunning;
		float mUpVelocity;
		float mRightVelocity;
		float mForwardVelocity;
		int mMapRenderType;
		Papaya mPapaya;
		std::map<int, Ogre::SceneNode*> mPlatoonEntities;
		std::map<int, Ogre::ColourValue> mTeamColors;
};

#endif
