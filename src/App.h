#ifndef APP_H
#define APP_H

#include <string>
#include <memory>

#include <Ogre.h>
#include <OIS.h>

#include "Terrain.h"
#include "Papaya.h"
#include "Messaging.h"

class App : public OIS::KeyListener, public PapayaEventListener, public WorldEntity {
	public:
		App();
		~App();
		void run();
		bool keyPressed(const OIS::KeyEvent &arg);
		bool keyReleased(const OIS::KeyEvent &arg);
		void PlatoonStatusChanged(const Platoon* p);
		void receiveMessage(const Message& m);
	private:
		void initResources();
		void initInput();
		void createUnitMesh();
		void createTerrain();
		void createTerrainTextures();
		void updateTerrain();
		void createTexture(const std::string& name, size_t width, size_t height,
				std::function<std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8> (size_t, size_t)> func);
		void setupUnitDisplay();
		Ogre::Entity* createUnitNode(const MilitaryUnit& m);
		void updateUnitPosition(const MilitaryUnit* m, Vector2 pos);
		void setUnitNodeScale(Ogre::SceneNode* n, const MilitaryUnit& m);

		struct UnitDrawInfo {
			UnitDrawInfo(Ogre::SceneNode* node)
				: mNode(node) { }
			std::map<EntityID, Vector2> mPositions;
			Ogre::SceneNode *mNode;
			Vector2 getPosition() const
			{
				Vector2 p;
				for(auto& it : mPositions) {
					p += it.second;
				}
				if(mPositions.size())
					p *= (1.0f / mPositions.size());
				return p;
			}
		};

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
		std::map<const Platoon*, UnitDrawInfo> mPlatoonEntities;
		std::map<const MilitaryUnit*, UnitDrawInfo> mCompanyEntities;
		std::map<const MilitaryUnit*, UnitDrawInfo> mBattalionEntities;
		std::map<const MilitaryUnit*, UnitDrawInfo> mBrigadeEntities;
		std::map<int, Ogre::ColourValue> mTeamColors;
		UnitSize mUnitScale;
		bool mUnitScaleChanged;
};

#endif
