#include <iostream>
#include <exception>

#include "App.h"

App::App()
	: mUpVelocity(0),
	mRightVelocity(0),
	mForwardVelocity(0),
	mMapRenderType(0),
	mPapaya(mTerrain)
{
	// get user data directory
	char* homedir = getenv("HOME");
	if(homedir) {
		mUserDataDir = std::string(homedir) + "/.army/";
		struct stat st;
		if(stat(mUserDataDir.c_str(), &st) != 0) {
			mkdir(mUserDataDir.c_str(), 0755);
		}
		// suppress ogre output to stderr
		Ogre::LogManager* logMgr = new Ogre::LogManager();
		logMgr->createLog(mUserDataDir + "Ogre.log", true, false, false);
	}
	mRoot.reset(new Ogre::Root("", "", ""));
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
		params["border"] = "fixed";
		mWindow = mRoot->createRenderWindow("Army", 800, 600, false, &params);
		if(!mWindow) {
			mRoot.reset();
			throw std::runtime_error("Could not create the render window.\n");
		}
		mScene = mRoot->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
		mRootNode = mScene->getRootSceneNode();
		mCamera = mScene->createCamera("Camera");
		mCamNode = mRootNode->createChildSceneNode("CameraNode");
		mCamNode->attachObject(mCamera);
		mViewport = mWindow->addViewport(mCamera);
		mViewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
		mCamera->setAspectRatio(float(mViewport->getActualWidth()) / float(mViewport->getActualHeight()));
		mCamera->setNearClipDistance(1.5f);
		mCamera->setFarClipDistance(3000.0f);
		mCamera->setPosition(64, 64, 100.0f);
		mCamera->lookAt(64, 64, 0);

		initResources();

		initInput();

		createUnitMesh();
		createTerrain();
		mPapaya.addEventListener(this);

		mRunning = true;
	}
}

App::~App()
{
	mWindow->removeAllViewports();
	mScene->destroyAllCameras();
	mScene->destroyAllEntities();
	mRootNode->removeAndDestroyAllChildren();
}

#define APP_RESOURCE_NAME "Resources"

void App::initResources()
{
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup(APP_RESOURCE_NAME);
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("share", "FileSystem", APP_RESOURCE_NAME, false);
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(APP_RESOURCE_NAME);
	Ogre::ResourceGroupManager::getSingleton().loadResourceGroup(APP_RESOURCE_NAME);
}

void App::initInput()
{
	size_t hWnd = 0;
	mWindow->getCustomAttribute("WINDOW", &hWnd);
	OIS::ParamList pl;
	std::ostringstream windowHndStr;

	windowHndStr << hWnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
#if defined OIS_WIN32_PLATFORM
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
	pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
	pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif
	mInputManager = OIS::InputManager::createInputSystem(pl);
	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
	mKeyboard->setEventCallback(this);
}

void App::createUnitMesh()
{
	Ogre::Plane plane(Ogre::Vector3::UNIT_Z, 0.0f);
	Ogre::MeshManager::getSingleton().createPlane("UnitMesh",
			APP_RESOURCE_NAME, plane, 0.4f, 0.4f, 4, 4, true,
			1, 1.0f, 1.0f, Ogre::Vector3::UNIT_Y);
	mTeamColors[1] = Ogre::ColourValue::Blue;
	mTeamColors[2] = Ogre::ColourValue::Red;
}

void App::createTexture(const std::string& name, size_t width, size_t height,
		std::function<std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8> (size_t, size_t)> func)
{
	Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(name,
			APP_RESOURCE_NAME,
			Ogre::TEX_TYPE_2D,
			width, height,
			0,
			Ogre::PF_BYTE_BGRA,
			Ogre::TU_DEFAULT);
	Ogre::HardwarePixelBufferSharedPtr pixelBuffer = texture->getBuffer();
	pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
	const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();
	Ogre::uint8* pDest = static_cast<Ogre::uint8*>(pixelBox.data);

	for (size_t j = 0; j < height; j++) {
		for(size_t i = 0; i < width; i++) {
			auto tpl = func(i, j);
			*pDest++ = std::get<2>(tpl);
			*pDest++ = std::get<1>(tpl);
			*pDest++ = std::get<0>(tpl);
			*pDest++ = 255; // A
		}
	}

	// Unlock the pixel buffer
	pixelBuffer->unlock();
}

static const char* materialnames[] = { "TerrainMaterial", "HeightMaterial", "VegetationMaterial"};

void App::updateTerrain()
{
	if(mMapRenderType > 2)
		mMapRenderType = 0;
	mScene->getEntity("plane1")->setMaterialName(materialnames[mMapRenderType]);
}

void App::createTerrainTextures()
{
	const char* texturenames[] = { "TerrainTexture", "HeightTexture", "VegetationTexture"};
	createTexture(texturenames[0], mTerrain.getWidth(), mTerrain.getWidth(), [&](size_t i, size_t j) {
			float tHeight = mTerrain.getHeightAt(i, j);
			float tVeg = mTerrain.getVegetationAt(i, j);
			float heightDiff = (tHeight - mTerrain.getHeightAt(i - 1, j)) * mTerrain.getHeightScale();
			float texLen = sqrt(heightDiff * heightDiff + 1);
			float lightnessCoeff = 1.0f + heightDiff * 0.8f / texLen;
			float r, g, b;
			r = (1.0f - tVeg) * 135 * lightnessCoeff; // R
			g = 135 * lightnessCoeff; // G
			b = (1.0f - tVeg) * 135 * lightnessCoeff; // B
			return std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8>(r, g, b);
			});

	createTexture(texturenames[1], mTerrain.getWidth(), mTerrain.getWidth(), [&](size_t i, size_t j) {
                        float r = mTerrain.getHeightAt(i, j) * 255;
			return std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8>(r, r, r);
			});

	createTexture(texturenames[2], mTerrain.getWidth(), mTerrain.getWidth(), [&](size_t i, size_t j) {
                        float r = mTerrain.getVegetationAt(i, j) * 255;
			return std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8>(r * 0.2f, r, r * 0.2f);
			});

	// Create a material using the texture
	for(int i = 0; i < 3; i++) {
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(materialnames[i],
				APP_RESOURCE_NAME);
		material->getTechnique(0)->getPass(0)->createTextureUnitState(texturenames[i]);
		material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	}
}

void App::createTerrain()
{
	createTerrainTextures();
	Ogre::Plane plane(Ogre::Vector3::UNIT_Z, 0.0f);
	Ogre::MeshManager::getSingleton().createPlane("terrain1",
			APP_RESOURCE_NAME, plane, 128, 128, 4, 4, true,
			1, 1.0f, 1.0f, Ogre::Vector3::UNIT_Y);
	Ogre::Entity* planeEnt = mScene->createEntity("plane1", "terrain1");
	updateTerrain();
	planeEnt->setCastShadows(false);
	Ogre::SceneNode* planeNode = mRootNode->createChildSceneNode();
	planeNode->attachObject(planeEnt);
	planeNode->setPosition(64, 64, 0);
}

void App::run()
{
	while(mRunning && !mWindow->isClosed()) {
		mPapaya.process(1.0f);
		mRoot->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
		mKeyboard->capture();
		mCamNode->translate(mRightVelocity, mUpVelocity, mForwardVelocity);
	}
}

bool App::keyPressed(const OIS::KeyEvent &arg)
{
	switch(arg.key) {
		case OIS::KC_ESCAPE:
			mRunning = false;
			break;
		case OIS::KC_UP:
			mUpVelocity = 0.1f;
			break;
		case OIS::KC_DOWN:
			mUpVelocity = -0.1f;
			break;
		case OIS::KC_PGUP:
			mForwardVelocity = 0.5f;
			break;
		case OIS::KC_PGDOWN:
			mForwardVelocity = -0.5f;
			break;
		case OIS::KC_RIGHT:
			mRightVelocity = 0.1f;
			break;
		case OIS::KC_LEFT:
			mRightVelocity = -0.1f;
			break;
		case OIS::KC_M:
			mMapRenderType++;
			if(mMapRenderType > 2)
				mMapRenderType = 0;
			updateTerrain();
			break;
		default:
			break;
	}
	return true;
}

bool App::keyReleased(const OIS::KeyEvent &arg)
{
	switch(arg.key) {
		case OIS::KC_UP:
		case OIS::KC_DOWN:
			mUpVelocity = 0.0f;
			break;
		case OIS::KC_PGUP:
		case OIS::KC_PGDOWN:
			mForwardVelocity = 0.0f;
			break;
		case OIS::KC_RIGHT:
		case OIS::KC_LEFT:
			mRightVelocity = 0.0f;
			break;
		default:
			break;
	}
	return true;
}

void App::PlatoonStatusChanged(const Platoon* p)
{
	const auto& it = mPlatoonEntities.find(p->getPlatoonID());
	Ogre::SceneNode* unitNode;
	if(it == mPlatoonEntities.end()) {
		const std::string unitsize("Platoon");
		const std::string unittype("Infantry");
		std::ostringstream ss;
		std::ostringstream materialstr;
		ss << unitsize << p->getPlatoonID();
		std::cerr << "Creating entity " << ss.str() << "\n";
		Ogre::Entity* unitEnt = mScene->createEntity(ss.str(), "UnitMesh");
		materialstr << unittype << unitsize << p->getSide();
		std::string materialname = materialstr.str();
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialname,
				APP_RESOURCE_NAME);
		if(material.isNull()) {
			std::cerr << "Creating material " << materialname << "\n";
			material = Ogre::MaterialManager::getSingleton().create(materialname,
					APP_RESOURCE_NAME);
			material->setAmbient(mTeamColors[p->getSide()]);
			material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
			material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
			std::string texturename1 = unittype + ".png";
			std::string texturename2 = unitsize + ".png";
			Ogre::TextureUnitState* t1 = material->getTechnique(0)->getPass(0)->createTextureUnitState(texturename1);
			t1->setColourOperation(Ogre::LBO_MODULATE);
			t1->setAlphaOperation(Ogre::LBX_MODULATE);
			Ogre::TextureUnitState* t2 = material->getTechnique(0)->getPass(0)->createTextureUnitState(texturename2);
			t2->setColourOperation(Ogre::LBO_MODULATE);
			t2->setAlphaOperation(Ogre::LBX_ADD);
		}
		unitEnt->setMaterialName(materialname);
		unitNode = mRootNode->createChildSceneNode();
		unitNode->attachObject(unitEnt);
		mPlatoonEntities[p->getPlatoonID()] = unitNode;
	}
	else {
		unitNode = it->second;
	}
	unitNode->setPosition(p->getPosition().x, p->getPosition().y, 0.1);
}
