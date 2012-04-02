#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#include <memory>
#include <iostream>
#include <exception>

#include "App.h"
#include "GUIController.h"

static const float lineHeight = 0.51f;

App::App()
	: mUpVelocity(0),
	mRightVelocity(0),
	mForwardVelocity(0),
	mMapRenderType(0),
	mUnitScale(UnitSize::Platoon),
	mUnitScaleChanged(true),
	mWindowWidth(0),
	mWindowHeight(0),
	mTimeScale(1),
	mOwnUnit(nullptr),
	mObserver(false)
{
	Papaya::instance().setup(&mTerrain);
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
		mWindowWidth = 800;
		mWindowHeight = 600;
		mWindow = mRoot->createRenderWindow("Army", mWindowWidth, mWindowHeight, false, &params);
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

		mRaySceneQuery = mScene->createRayQuery(Ogre::Ray());

		initResources();

		initInput();
		checkWindowResize();

		createUnitMesh();
		createExtraMaterials();
		createTerrain();
		Papaya::instance().addEventListener(this);
		MessageDispatcher::instance().registerWorldEntity(this);
		if(!mObserver)
			setupHumanControls();

		mRunning = true;
	}
}

App::~App()
{
	mScene->destroyQuery(mRaySceneQuery);
	mWindow->removeAllViewports();
	mScene->destroyAllCameras();
	mScene->destroyAllEntities();
	mRootNode->removeAndDestroyAllChildren();
}

void App::setupHumanControls()
{
	std::shared_ptr<Army> a = Papaya::instance().getArmy(0);
	if(a) {
		std::shared_ptr<MilitaryUnit> m = a->getUnits().at(0);
		for(int i = 0; i < 2; i++) {
			m = m->getUnits().at(0);
		}
		std::shared_ptr<GUIController> c(new GUIController(this, m.get()));
		mOwnUnit = m;
		m->setController(c);
		for(auto& p : m->getPlatoons()) {
			mControlledUnits.push_back(std::pair<MilitaryUnit*, std::shared_ptr<GUIController>>(p, c));
		}
	}
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
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
	mMouse->setEventCallback(this);
}

void App::createUnitMesh()
{
	Ogre::Plane plane(Ogre::Vector3::UNIT_Z, 0.0f);
	Ogre::MeshManager::getSingleton().createPlane("UnitMesh",
			APP_RESOURCE_NAME, plane, 0.4f, 0.4f, 4, 4, true,
			1, 1.0f, 1.0f, Ogre::Vector3::UNIT_Y);
	mTeamColors[1] = Ogre::ColourValue(0.42, 0.65, 1.0);
	mTeamColors[2] = Ogre::ColourValue(1.0, 0.42, 0.42);
}

void App::createExtraMaterials()
{
	// line
	Ogre::MaterialPtr lineMaterial = Ogre::MaterialManager::getSingleton().create("LineMaterial",
			APP_RESOURCE_NAME);
	lineMaterial->setReceiveShadows(false); 
	lineMaterial->getTechnique(0)->setLightingEnabled(true); 
	lineMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,0); 
	lineMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,1); 
	lineMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,1); 
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
			float tHeight = mTerrain.getHeightAt(Vector2(i, j));
			float tVeg = mTerrain.getVegetationAt(Vector2(i, j));
			float heightDiff = (tHeight - mTerrain.getHeightAt(Vector2(i - 1, j))) * mTerrain.getHeightScale();
			float texLen = sqrt(heightDiff * heightDiff + 1);
			float lightnessCoeff = 1.0f + heightDiff * 0.8f / texLen;
			float r, g, b;
			r = (1.0f - tVeg) * 135 * lightnessCoeff; // R
			g = 135 * lightnessCoeff; // G
			b = (1.0f - tVeg) * 135 * lightnessCoeff; // B
			return std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8>(r, g, b);
			});

	createTexture(texturenames[1], mTerrain.getWidth(), mTerrain.getWidth(), [&](size_t i, size_t j) {
                        float r = mTerrain.getHeightAt(Vector2(i, j)) * 255;
			return std::tuple<Ogre::uint8, Ogre::uint8, Ogre::uint8>(r, r, r);
			});

	createTexture(texturenames[2], mTerrain.getWidth(), mTerrain.getWidth(), [&](size_t i, size_t j) {
                        float r = mTerrain.getVegetationAt(Vector2(i, j)) * 255;
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
	mTerrainPlane = Ogre::Plane(Ogre::Vector3::UNIT_Z, 0.0f);
	Ogre::MeshManager::getSingleton().createPlane("terrain1",
			APP_RESOURCE_NAME, mTerrainPlane, 128, 128, 4, 4, true,
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
		Papaya::instance().process(mTimeScale * 0.01f);
		setupUnitDisplay();
		mRoot->renderOneFrame();
		checkWindowResize();
		Ogre::WindowEventUtilities::messagePump();
		mKeyboard->capture();
		mMouse->capture();
		mCamNode->translate(mRightVelocity, mUpVelocity, mForwardVelocity);
	}
}

bool App::checkWindowResize()
{
	unsigned int mOldWidth, mOldHeight;
	mOldWidth = mWindowWidth;
	mOldHeight = mWindowHeight;
	mWindowWidth = mWindow->getWidth();
	mWindowHeight = mWindow->getHeight();
	if(mWindowWidth != mOldWidth || mWindowHeight != mOldHeight) {
		mMouse->getMouseState().width = mWindowWidth;
		mMouse->getMouseState().height = mWindowHeight;
		std::cout << "New window size: " << mWindowWidth << " x " << mWindowHeight << "\n";
		return true;
	}
	return false;
}

void App::setupUnitDisplay()
{
	if(!mUnitScaleChanged)
		return;

	mUnitScaleChanged = false;
	for(auto& it : mPlatoonEntities) {
		it.second.mNode->setVisible(mUnitScale == UnitSize::Platoon);
	}
	for(auto& it : mCompanyEntities) {
		it.second.mNode->setVisible(mUnitScale == UnitSize::Company);
	}
	for(auto& it : mBattalionEntities) {
		it.second.mNode->setVisible(mUnitScale == UnitSize::Battalion);
	}
	for(auto& it : mBrigadeEntities) {
		it.second.mNode->setVisible(mUnitScale == UnitSize::Brigade);
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
		case OIS::KC_1:
		case OIS::KC_2:
		case OIS::KC_3:
			mUnitScale = UnitSize::Platoon;
			mUnitScaleChanged = true;
			break;
		case OIS::KC_4:
			mUnitScale = UnitSize::Company;
			mUnitScaleChanged = true;
			break;
		case OIS::KC_5:
			mUnitScale = UnitSize::Battalion;
			mUnitScaleChanged = true;
			break;
		case OIS::KC_6:
		case OIS::KC_7:
			mUnitScale = UnitSize::Brigade;
			mUnitScaleChanged = true;
			break;
		case OIS::KC_ADD:
			if(mTimeScale < 128)
				mTimeScale *= 2;
			std::cout << "Time scale: " << mTimeScale << "\n";
			break;
		case OIS::KC_SUBTRACT:
			if(mTimeScale > 1)
				mTimeScale /= 2;
			std::cout << "Time scale: " << mTimeScale << "\n";
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

bool App::mouseMoved(const OIS::MouseEvent& arg)
{
	return true;
}

bool App::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID button)
{
	Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(arg.state.X.abs / float(arg.state.width),
			arg.state.Y.abs / float(arg.state.height));
	std::pair<bool, Ogre::Real> intres = mouseRay.intersects(mTerrainPlane);

	if(button == OIS::MB_Left) {
		mRaySceneQuery->setRay(mouseRay);
		mRaySceneQuery->setSortByDistance(true);
		Ogre::RaySceneQueryResult& result = mRaySceneQuery->execute();
		for (Ogre::RaySceneQueryResult::iterator itr = result.begin(); itr != result.end(); itr++) {
			// Is this result a MovableObject?
			if (itr->movable && itr->movable->getName() != std::string("Camera"))
			{
				std::cout << "MovableObject: " << itr->movable->getName() << "\n";
				Ogre::Entity* e = dynamic_cast<Ogre::Entity*>(itr->movable);
				if(e) {
					setSelectedUnit(e);
				}
			}
		}

		if(intres.first) {
			Ogre::Vector3 point = mouseRay.getPoint(intres.second);
			mLineEnd.x = point.x;
			mLineEnd.y = point.y;
			std::cout << "Point at " << mLineEnd << "\n";
			std::vector<Vector2> points;
			points.push_back(Vector2(0, 0));
			points.push_back(Vector2(mLineEnd.x, mLineEnd.y));
			createLine("LineObject", points);
		}
	}
	else if(button == OIS::MB_Right && intres.first) {
		for(auto& pair : mSelectedUnits) {
			if(humanControlled(pair.first)) {
				Ogre::Vector3 op = mouseRay.getPoint(intres.second);
				Vector2 point = Vector2(op.x, op.y);
				MessageDispatcher::instance().dispatchMessage(Message(mOwnUnit->getEntityID(),
							pair.first->getEntityID(),
							0.0f, 0.0f, MessageType::Goto, point));
			}
		}
	}
	return true;
}

bool App::mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID button)
{
	return true;
}

void App::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::PlatoonDied:
			PlatoonStatusChanged(m.mData->platoon);
			break;
		default:
			std::cout << "Unhandled message " << int(m.mType) << " in App.\n";
			break;
	}
}

void App::updateUnitMaterial(const MilitaryUnit& m, Ogre::Entity* ent)
{
	const std::string unittype(branchToName(m.getBranch()));
	const std::string unitsize(unitSizeToName(m.getUnitSize()));
	std::string materialname = getUnitMaterialName(m);
	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialname,
			APP_RESOURCE_NAME);
	if(material.isNull()) {
		Ogre::LogManager::getSingleton().stream() << "Creating material " << materialname << "\n";
		material = Ogre::MaterialManager::getSingleton().create(materialname,
				APP_RESOURCE_NAME);
		material->setAmbient(mTeamColors[m.getSide()]);
		material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
		std::string texturename1 = unittype + ".png";
		std::string texturename2 = unitsize + ".png";
		Ogre::TextureUnitState* t1 = material->getTechnique(0)->getPass(0)->createTextureUnitState(texturename1);
		t1->setColourOperation(Ogre::LBO_MODULATE);
		t1->setAlphaOperation(Ogre::LBX_MODULATE);
		Ogre::TextureUnitState* t2 = material->getTechnique(0)->getPass(0)->createTextureUnitState(texturename2);
		t2->setColourOperation(Ogre::LBO_ALPHA_BLEND);
		t2->setAlphaOperation(Ogre::LBX_ADD);
		if(humanControlled(&m)) {
			Ogre::TextureUnitState* t3 = material->getTechnique(0)->getPass(0)->createTextureUnitState();
			t3->setColourOperationEx(Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_CURRENT,
					Ogre::ColourValue(1.0, 0.5, 0.5, 1.0));
		}
		if(unitSelected(&m)) {
			Ogre::TextureUnitState* t4 = material->getTechnique(0)->getPass(0)->createTextureUnitState("Spot.png");
			t4->setColourOperation(Ogre::LBO_ALPHA_BLEND);
			t4->setAlphaOperation(Ogre::LBX_ADD);
		}
	}
	ent->setMaterialName(materialname);
}

Ogre::Entity* App::createUnitNode(const MilitaryUnit& m)
{
	std::ostringstream ss;
	const std::string unitsize(unitSizeToName(m.getUnitSize()));
	ss << unitsize << m.getEntityID();
	Ogre::Entity* unitEnt = mScene->createEntity(ss.str(), "UnitMesh");
	updateUnitMaterial(m, unitEnt);
	return unitEnt;
}

bool App::humanControlled(const MilitaryUnit* m) const
{
	return std::find_if(mControlledUnits.begin(), mControlledUnits.end(),
			[=](const std::pair<MilitaryUnit*, std::shared_ptr<GUIController>>& m1) { return m1.first == m; })
		!= mControlledUnits.end();
}

bool App::unitSelected(const MilitaryUnit* m) const
{
	return std::find_if(mSelectedUnits.begin(), mSelectedUnits.end(),
			[=](const std::pair<MilitaryUnit*, Ogre::Entity*>& m1) { return m1.first == m; })
		!= mSelectedUnits.end();
}

std::string App::getUnitMaterialName(const MilitaryUnit& m) const
{
	const std::string unitsize(unitSizeToName(m.getUnitSize()));
	const std::string unittype(branchToName(m.getBranch()));
	std::ostringstream ss;
	std::ostringstream materialstr;
	ss << unitsize << m.getEntityID();
	materialstr << unittype << unitsize << m.getSide() << (humanControlled(&m) ? "Human" : "") << (unitSelected(&m) ? "Selected" : "");
	return materialstr.str();
}

void App::updateUnitPosition(const MilitaryUnit* m, Vector2 pos)
{
	const MilitaryUnit* lower = m;
	const MilitaryUnit* comm;
	while(1) {
		comm = lower->getCommandingUnit();
		if(!comm)
			break;
		std::map<const MilitaryUnit*, UnitDrawInfo>* unitmap = nullptr;
		switch(comm->getUnitSize()) {
			case UnitSize::Company:
				unitmap = &mCompanyEntities;
				break;
			case UnitSize::Battalion:
				unitmap = &mBattalionEntities;
				break;
			case UnitSize::Brigade:
				unitmap = &mBrigadeEntities;
				break;
			default:
				break;
		}
		if(unitmap) {
			auto commit = unitmap->find(comm);
			if(commit == unitmap->end()) {
				Ogre::SceneNode* newnode = mRootNode->createChildSceneNode();
				newnode->attachObject(createUnitNode(*comm));
				setUnitNodeScale(newnode, *comm);
				commit = unitmap->insert(unitmap->begin(),
						std::make_pair(comm, UnitDrawInfo(newnode)));
			}
			if(pos.x || pos.y) {
				commit->second.mPositions[lower->getEntityID()] = pos;
			}
			else {
				commit->second.mPositions.erase(lower->getEntityID());
			}
			pos = commit->second.getPosition();
			lower = comm;
			if(pos.x || pos.y) {
				commit->second.mNode->setPosition(pos.x, pos.y, 0.1f);
			}
			else {
				// All lower units are gone.
				commit->second.mNode->setVisible(false);
				unitmap->erase(commit);
			}
		}
		else {
			break;
		}
	}
}

void App::PlatoonStatusChanged(const Platoon* p)
{
	const auto& it = mPlatoonEntities.find(p);
	Ogre::SceneNode* unitNode;
	if(it == mPlatoonEntities.end()) {
		unitNode = mRootNode->createChildSceneNode();
		unitNode->attachObject(createUnitNode(*p));
		setUnitNodeScale(unitNode, *p);
		mPlatoonEntities.insert(std::make_pair(p, UnitDrawInfo(unitNode)));
	}
	else {
		unitNode = it->second.mNode;
	}
	if(!p->isDead()) {
		unitNode->setPosition(p->getPosition().x, p->getPosition().y, 0.1f);
		updateUnitPosition(p, p->getPosition());
	}
	else {
		unitNode->setVisible(false);
		mRootNode->removeChild(unitNode);
		mPlatoonEntities.erase(it);
		updateUnitPosition(p, Vector2());
	}
}

void App::setUnitNodeScale(Ogre::SceneNode* n, const MilitaryUnit& m)
{
	float scale = 1.0f;
	switch(m.getUnitSize()) {
		case UnitSize::Single: scale = 0.1f; break;
		case UnitSize::Squad: scale = 0.5f; break;
		case UnitSize::Platoon: scale = 1.0f; break;
		case UnitSize::Company: scale = 2.0f; break;
		case UnitSize::Battalion: scale = 4.0f; break;
		case UnitSize::Brigade: scale = 8.0f; break;
		case UnitSize::Division: scale = 16.0f; break;
	}
	n->setScale(scale, scale, 1.0f);
}

void App::createLine(const std::string& name, const std::vector<Vector2>& points)
{
	Ogre::SceneNode* node;
	Ogre::ManualObject* obj;
	bool newobject = false;

	if(!mScene->hasSceneNode(name)) {
		node = mScene->getRootSceneNode()->createChildSceneNode(name); 
		newobject = true;
	}
	else {
		node = mScene->getSceneNode(name); 
	}
	if(newobject) {
		obj = mScene->createManualObject(name);
		obj->begin("LineMaterial", Ogre::RenderOperation::OT_LINE_STRIP); 
	}
	else {
		obj = mScene->getManualObject(name);
		obj->beginUpdate(0);
	}
	for(auto& p : points) {
		obj->position(p.x, p.y, lineHeight);
	}
	obj->end(); 
	if(newobject)
		node->attachObject(obj);
}

void App::setTargetArea(const Area2& area)
{
	std::vector<Vector2> points;
	points.push_back(Vector2(area.x1, area.y1));
	points.push_back(Vector2(area.x1, area.y2));
	points.push_back(Vector2(area.x2, area.y2));
	points.push_back(Vector2(area.x2, area.y1));
	points.push_back(Vector2(area.x1, area.y1));
	createLine("Target Area Line", points);
}

void App::setSelectedUnit(Ogre::Entity* e)
{
	int entityid;
	if(sscanf(e->getName().c_str(), "Platoon%d", &entityid) == 1) {
		Entity* en = EntityManager::instance().getEntity(entityid);
		if(en) {
			Platoon* p = dynamic_cast<Platoon*>(en);
			if(p) {
				auto prevsel = mSelectedUnits;
				mSelectedUnits.clear();
				for(auto& m : prevsel) {
					updateUnitMaterial(*m.first, m.second);
				}
				mSelectedUnits.push_back(std::pair<MilitaryUnit*, Ogre::Entity*>(p, e));
				updateUnitMaterial(*p, e);
			}
		}
	}
}


