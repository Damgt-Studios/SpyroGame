// Include the precompiled headers
#include "pchgame.h"
#include <vector>
#include <windows.gaming.input.h>
#include <collection.h>
#include "Engine.h"
#include "ADPhysics.h"
#include "ADQuadTree.h"
#include "TheGreatGolem.h"

#include "ADUserInterface.h"
#include "GameUserInterface.h"
//#include "AudioManager.h"
#include "ADAudio.h"
#include "GameUtilities.h"
#include "GameObjectClasses.h"
#include "MeshLoader.h"
#include "ADAI.h"
#include "ADPathfinding.h"
#include "AnimationStateMachine.h"
#include "Listeners.h"

#define ShowColliders

// Use some common namespaces to simplify the code
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Media::Playback;
using namespace Windows::Media::Core;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::UI::ViewManagement;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Platform;
using namespace Platform::Collections;

// Settings
bool FULLSCREEN = false;
// Settings


// the class definition for the core "framework" of our app
ref class App sealed : public IFrameworkView
{
private:
	Engine* engine;
	TheGreatGolem* game;
	ADResource::ADGameplay::Golem* golem;
	AnimationStateMachine GolemAnimController;
	AD_ULONG golem_collider = 0;

	//AudioManager* audio_manager;
	AD_AUDIO::ADAudio* audio;

	bool shutdown = false;

	//// Temp music
	//int effect_id;
	//bool effect_triggered = false;

	// Timing
	XTime game_time;
	float timer = 0;
	float delta_time = 0;

	const float physics_rate = 0.2f;
	float physics_timer = 0;

	//// Audio
	//float main_music_loop_timer = 3;
	//bool music_triggered = false;

	// Rotation
	float rot = 0;

	float yaw = 180.0f;
	float pitch = 30.0f;

	float default_yaw = 180.0f;
	float default_pitch = 30.0f;

	// Physics
	ADPhysics::AABB test_colider;
	ADPhysics::AABB test_colider1;
	ADPhysics::Plane test_plane;



public:
	// some functions called by Windows
	virtual void Initialize(CoreApplicationView^ AppView)
	{

		if (FULLSCREEN)
		{
			Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode =
				Windows::UI::ViewManagement::ApplicationViewWindowingMode::FullScreen;
		}
		else
		{
			Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode =
				Windows::UI::ViewManagement::ApplicationViewWindowingMode::CompactOverlay;
		}

		// set the OnActivated function to handle to Activated "event"
		AppView->Activated += ref new TypedEventHandler
			<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

		Gamepad::GamepadAdded += ref new EventHandler<Gamepad^>(&Input::OnGamepadAdded);
		Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^>(&Input::OnGamepadRemoved);
	}

	virtual void SetWindow(CoreWindow^ Window)
	{
		Window->KeyDown += ref new TypedEventHandler
			<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyDown);
	}

	virtual void Load(String^ EntryPoint) {}

	virtual void Run()
	{
		CoreWindow^ Window = CoreWindow::GetForCurrentThread();
		//---------------------Create The Engines
		engine = new Engine;
		AD_AUDIO::ADAudio audioEngine;
		audioEngine.Init();

		//Implement Audio Engine
		audioEngine.LoadBank("files//audio//Master.bank", FMOD_STUDIO_LOAD_BANK_NORMAL);
		audioEngine.LoadBank("files//audio//Master.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL);


		AD_AUDIO::AudioSource golemSteps;
		golemSteps.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemSteps.engine = &audioEngine;
		golemSteps.personalVolume = 0.5f;
		golemSteps.restartOnRepeat = true;
		golemSteps.LoadSound("event:/Sfx_GolemStep", true, false, true, true);

		AD_AUDIO::AudioSource titleMusic;
		titleMusic.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::MUSIC;
		titleMusic.engine = &audioEngine;
		titleMusic.personalVolume = 0.02f;
		titleMusic.LoadSound("files\\audio\\Opening.mp3", false, false, true, true);

		AudioSourceEvent playTitleEvent(titleMusic);
		ADEvents::ADEventSystem::Instance()->RegisterClient("PlayTitle", &playTitleEvent);

		AD_AUDIO::AudioSource golemPunchSound;
		golemPunchSound.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemPunchSound.engine = &audioEngine;
		golemPunchSound.personalVolume = 0.5f;
		golemPunchSound.restartOnRepeat = false;
		golemPunchSound.LoadSound("event:/Sfx_MinorGrunt", true, true, false, false);
		AudioSourceEvent golemPunchEvent(golemPunchSound);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemPunch", &golemPunchEvent);

		AD_AUDIO::AudioSource golemKickSound;
		golemKickSound.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemKickSound.engine = &audioEngine;
		golemKickSound.personalVolume = 0.5f;
		golemKickSound.restartOnRepeat = false;
		golemKickSound.LoadSound("event:/Sfx_EarthHit2", true, true, false, false);
		AudioSourceEvent golemKickEvent(golemKickSound);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemKick", &golemKickEvent);

		AD_AUDIO::AudioSource golemSlamSound;
		golemSlamSound.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemSlamSound.engine = &audioEngine;
		golemSlamSound.personalVolume = 0.5f;
		golemSlamSound.restartOnRepeat = false;
		golemSlamSound.LoadSound("event:/Sfx_EarthHit", true, true, false, false);
		AudioSourceEvent golemSlamEvent(golemSlamSound);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemSlam", &golemSlamEvent);

		AD_AUDIO::AudioSource golemWaveSound;
		golemWaveSound.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemWaveSound.engine = &audioEngine;
		golemWaveSound.personalVolume = 0.5f;
		golemWaveSound.restartOnRepeat = false;
		golemWaveSound.LoadSound("event:/Sfx_WaterWaveSpell", true, true, false, false);
		AudioSourceEvent golemWaveEvent(golemWaveSound);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemWaterWave", &golemWaveEvent);

		AD_AUDIO::AudioSource golemFireball;
		golemFireball.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemFireball.engine = &audioEngine;
		golemFireball.personalVolume = 0.5f;
		golemFireball.restartOnRepeat = false;
		golemFireball.LoadSound("event:/Sfx_FireBallSpell", true, true, false, false);
		AudioSourceEvent golemFireballEvent(golemFireball);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemFireball", &golemFireballEvent);

		AD_AUDIO::AudioSource golemIronHide;
		golemIronHide.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemIronHide.engine = &audioEngine;
		golemIronHide.personalVolume = 0.5f;
		golemIronHide.restartOnRepeat = false;
		golemIronHide.LoadSound("event:/Sfx_IronSkinSpell", true, true, false, false);
		AudioSourceEvent golemTauntEvent(golemIronHide);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemTaunt", &golemTauntEvent);



		AD_AUDIO::AudioSource golemRootingSpell;
		golemRootingSpell.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemRootingSpell.engine = &audioEngine;
		golemRootingSpell.personalVolume = 0.5f;
		golemRootingSpell.restartOnRepeat = false;
		golemRootingSpell.LoadSound("event:/Sfx_RootCrushSpell", true, true, false, false);
		AudioSourceEvent golemRootEvent(golemRootingSpell);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemRooting", &golemRootEvent);

		AD_AUDIO::AudioSource golemEatMinion;
		golemEatMinion.audioSourceType = AD_AUDIO::AUDIO_SOURCE_TYPE::SOUND_FX;
		golemEatMinion.engine = &audioEngine;
		golemEatMinion.personalVolume = 0.5f;
		golemEatMinion.restartOnRepeat = false;
		golemEatMinion.LoadSound("event:/Sfx_MinnionScream", true, true, false, false);
		AudioSourceEvent golemEatMinionEvent(golemEatMinion);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemEat", &golemEatMinionEvent);

		DefinitionReader df;
		df.ReadMasterFile();

		ApplyEffectEvent golemEatingEvent(*DefinitionDatabase::Instance()->effectsDatabase["GolemEat"]);
		ADEvents::ADEventSystem::Instance()->RegisterClient("Apply_GolemEat", &golemEatingEvent);


		// Initialize the engine
		engine->SetCamera(XMFLOAT3(0, 10000.0f, -100.0f), 0, 0, 45);

		Light light;
		ZeroMemory(&light, sizeof(Light));
		light.lightType = (int)LIGHTTYPE::DIRECTIONAL;
		light.diffuse =
			light.ambientUp =
			light.ambientDown =
			light.specular =
			XMFLOAT4(1, 1, 1, 1);
		light.ambientIntensityDown = .1;
		light.ambientIntensityUp = .1;
		light.lightDirection = XMFLOAT4(0, -1, 0, 1);
		light.diffuseIntensity = 1;
		light.specularIntensity = .2;
		light.diffuse =
			light.ambientUp =
			light.ambientDown =
			light.specular =
			XMFLOAT4(1, 1, 1, 1);
		ResourceManager::AddLight(light);

		// Point light
		Light light1;
		ZeroMemory(&light1, sizeof(Light));
		light1.ambientIntensityDown = .1;
		light1.ambientIntensityUp = .1;
		light1.lightDirection = XMFLOAT4(0, 0, 10, 1);
		light1.diffuseIntensity = .5;
		light1.specularIntensity = .2;
		light1.diffuse =
			light1.ambientUp =
			light1.ambientDown =
			light1.specular =
			XMFLOAT4(1, 1, 1, 1);
		light1.lightType = (int)LIGHTTYPE::POINT;
		light1.position = XMFLOAT4(10, 0, 0, 1);
		light1.lightRadius = 100;
		ResourceManager::AddLight(light1);

		std::vector<std::string> animations;
		animations.push_back("files/models/Golem_1_Idle.animfile");
		animations.push_back("files/models/Golem_1_Born.animfile");
		animations.push_back("files/models/Golem_1_Run.animfile");
		animations.push_back("files/models/Golem_1_Command.animfile");
		animations.push_back("files/models/Golem_1_Death.animfile");
		animations.push_back("files/models/Golem_1_Eat.animfile");
		animations.push_back("files/models/Golem_1_GroundSlam.animfile");
		animations.push_back("files/models/Golem_1_Heavy_Hit_Back.animfile");
		animations.push_back("files/models/Golem_1_Heavy_Hit_Front.animfile");
		animations.push_back("files/models/Golem_1_Heavy_Hit_Left.animfile");
		animations.push_back("files/models/Golem_1_Heavy_Hit_Right.animfile");
		animations.push_back("files/models/Golem_1_Idle.animfile");
		animations.push_back("files/models/Golem_1_IdleLook.animfile");
		animations.push_back("files/models/Golem_1_Kick.animfile");
		animations.push_back("files/models/Golem_1_Run.animfile");
		animations.push_back("files/models/Golem_1_Special.animfile");
		animations.push_back("files/models/Golem_1_SummonMinions.animfile");
		animations.push_back("files/models/Golem_1_TowerPunch.animfile");

		golem = GameUtilities::LoadGolemFromModelFile("files/models/Golem_1.AnimMesh", "files/textures/Golem_1.mat", animations, XMFLOAT3(10, 0.00001, 10), XMFLOAT3(0.1, 0.1, 0.1), XMFLOAT3(0, 0, 0));

		// Orbit camera
		engine->GetOrbitCamera()->SetLookAt((XMFLOAT3&)(Float3ToVector((*ResourceManager::GetSimpleModelPtrFromMeshId(golem->GetMeshId()))->position)));
		engine->GetOrbitCamera()->SetRadius(20);
		engine->GetOrbitCamera()->Rotate(yaw, pitch);

		GolemGameUISetup::GameUserInterface gameUI;

		if (!engine->Initialize())
		{
			return;
		}


		std::vector<std::string> stoneMinionAnimations;
		stoneMinionAnimations.push_back("files/models/Minion_3_Idle.animfile");
		std::vector<std::string> waterMinionAnimations;
		waterMinionAnimations.push_back("files/models/Minion_4_Idle.animfile");
		std::vector<std::string> fireMinionAnimations;
		fireMinionAnimations.push_back("files/models/Minion_2_Idle.animfile");
		std::vector<std::string> woodMinionAnimations;
		woodMinionAnimations.push_back("files/models/Minion_1_Idle.animfile");

		ResourceManager::AddSkybox("files/models/Skybox.mesh", "files/textures/Skybox.mat", XMFLOAT3(0, 0, 0), XMFLOAT3(-10, -10, -10), XMFLOAT3(0, 0, 0));

		GolemAnimController.Initialize(golem);
		golem->GetAnimationController(GolemAnimController);

		Renderable* cube = GameUtilities::AddSimpleAsset("files/models/Cube.mesh", "files/textures/Ground.mat", XMFLOAT3(0, 1, 10), XMFLOAT3(10, 10, 10), XMFLOAT3(0, 0, 0));

#ifdef _DEBUG
		Renderable* golemCollider = GameUtilities::AddRenderableCollider(XMFLOAT3(0, 0, 0), XMFLOAT3(0.1, 0.1, 0.1), XMFLOAT3(0, 0, 0));
		Renderable* cubeCollider = GameUtilities::AddRenderableCollider(XMFLOAT3(0, 1, 10), XMFLOAT3(10, 10, 10), XMFLOAT3(0, 0, 0));
		cubeCollider->colliderPtr = nullptr;
#endif


		ADAI::FlockingGroup commandFlock;
		ADAI::FlockingGroup idleFlock;

		std::vector<Destructable*> stoneMinions;
		std::vector<ADAI::AIUnit*> stoneMinionsAI;
		std::vector<Destructable*> waterMinions;
		std::vector<ADAI::AIUnit*> waterMinionsAI;
		std::vector<Destructable*> fireMinions;
		std::vector<ADAI::AIUnit*> fireMinionsAI;
		std::vector<Destructable*> woodMinions;
		std::vector<ADAI::AIUnit*> woodMinionsAI;

		for (int i = 0; i < 10; i++)
		{
			stoneMinions.push_back(GameUtilities::AddDestructableFromModelFile("files/models/Minion_3.AnimMesh", "files/textures/Minion_3.mat", stoneMinionAnimations, XMFLOAT3(-130, 5, -130), XMFLOAT3(0.03f, 0.03f, 0.03f), XMFLOAT3(0, 0, 0)));
			stoneMinionsAI.push_back(GameUtilities::AttachMinionAI(stoneMinions[i], golem->flockingGroups[STONE]));
			waterMinions.push_back(GameUtilities::AddDestructableFromModelFile("files/models/Minion_4.AnimMesh", "files/textures/Minion_4.mat", waterMinionAnimations, XMFLOAT3(-130, 5, 130), XMFLOAT3(0.03f, 0.03f, 0.03f), XMFLOAT3(0, 0, 0)));
			waterMinionsAI.push_back(GameUtilities::AttachMinionAI(waterMinions[i], golem->flockingGroups[WATER]));
			fireMinions.push_back(GameUtilities::AddDestructableFromModelFile("files/models/Minion_2.AnimMesh", "files/textures/Minion_2.mat", fireMinionAnimations, XMFLOAT3(130, 5, -130), XMFLOAT3(0.03f, 0.03f, 0.03f), XMFLOAT3(0, 0, 0)));
			fireMinionsAI.push_back(GameUtilities::AttachMinionAI(fireMinions[i], golem->flockingGroups[FIRE]));
			woodMinions.push_back(GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(130, 5, 130), XMFLOAT3(0.03f, 0.03f, 0.03f), XMFLOAT3(0, 0, 0)));
			woodMinionsAI.push_back(GameUtilities::AttachMinionAI(woodMinions[i], golem->flockingGroups[WOOD]));
		}
		//Destructable* e3 = GameUtilities::AddDestructableFromModelFile("files/models/Golem_1.AnimMesh", "files/textures/Golem_1.mat", animations, XMFLOAT3(-15, 5, -40), XMFLOAT3(0.05f, 0.05f, 0.05f), XMFLOAT3(0, 0, 0));
		//Destructable* e4 = GameUtilities::AddDestructableFromModelFile("files/models/Golem_1.AnimMesh", "files/textures/Golem_1.mat", animations, XMFLOAT3(-5, 5, -40), XMFLOAT3(0.05f, 0.05f, 0.05f), XMFLOAT3(0, 0, 0));
		//Destructable* e5 = GameUtilities::AddDestructableFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(5, 5, -40), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0, 0, 0));
		//Destructable* e6 = GameUtilities::AddDestructableFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(15, 5, -40), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0, 0, 0));
		//Destructable* e7 = GameUtilities::AddDestructableFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(30, 5, -40), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0, 0, 0));

		//ADAI::AIUnit* ai2 = GameUtilities::AttachMinionAI(e3, &commandFlock, &idleFlock);
		//ADAI::AIUnit* ai3 = GameUtilities::AttachMinionAI(e4, &commandFlock, &idleFlock);
		//ADAI::AIUnit* ai4 = GameUtilities::AttachMinionAI(e5, &commandFlock, &idleFlock);
		//ADAI::AIUnit* ai5 = GameUtilities::AttachMinionAI(e6, &commandFlock, &idleFlock);
		//ADAI::AIUnit* ai6 = GameUtilities::AttachMinionAI(e7, &commandFlock, &idleFlock);

		Destructable* m1 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", stoneMinionAnimations, XMFLOAT3(0, 5, 0), XMFLOAT3(0.02f, 1.02f, 0.02f), XMFLOAT3(0, 0, 0));
		//Destructable* m2 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", minionAnimations, XMFLOAT3(50, 5, 30), XMFLOAT3(0.02f, 0.02f, 0.02f), XMFLOAT3(0, 0, 0));

		golem->targetMarker = m1;
		golemEatingEvent.SetTarget(golem);


		// No more.
		  /////////////////////////////////////////

			 //Destructable* e2 = GameUtilities::AddEnemyFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, -10), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));
			 //Destructable* e3 = GameUtilities::AddEnemyFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, -20), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));
			 //Trigger* t1 = GameUtilities::AddEndGameTriggerFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, 30), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));
	 //=======
			 //ResourceManager::AddPBRModel("files/models/mapped_skybox.wobj", XMFLOAT3(0, -1.3, 0), XMFLOAT3(100, .1, 100), XMFLOAT3(0, 0, 0));

			 //Renderable* a1 = GameUtilities::AddPBRStaticAsset("files/models/oildrum.wobj", XMFLOAT3(3, 0, -1), XMFLOAT3(.03, .03, .03), XMFLOAT3(0, 0, 0));
			 //Renderable* a2 = GameUtilities::AddPBRStaticAsset("files/models/text.wobj", XMFLOAT3(1, 0, 0), XMFLOAT3(.03, .03, .03), XMFLOAT3(0, 0, 0));
			 //Collectable** collectables = new Collectable*[10];
			 //for (int i = 0; i < 10; ++i)
			 //{
			 //	collectables[i] = GameUtilities::AddCollectableFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3((i&10) *-5, 0, 5*(i%2)), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));
			 //	GameUtilities::AddGameObject(collectables[i]);
			 //}
			 //Enemy* e1 = GameUtilities::AddEnemyFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, -5), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));
			 //Enemy* e2 = GameUtilities::AddEnemyFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, -10), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));
			 //Enemy* e3 = GameUtilities::AddEnemyFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, -20), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));
			 //Trigger* t1 = GameUtilities::AddTriggerFromModelFile("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, 30), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));


			 //ADPhysics::AABB a3c = ADPhysics::AABB(XMFLOAT3(10, 0, 0), XMFLOAT3(1, 1, 1));

			 //Trigger* myHitBox = GameUtilities::AddHitbox("files/models/mapped_skybox.wobj", XMFLOAT3(0, 0, -30), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 0));

			 // Colliders
			 //Trigger* ess1 = GameUtilities::AddTinyEssenceFromModelFile("files/models/Minion_3.AnimMesh", "files/textures/Minion_3.mat", stoneMinionAnimations, XMFLOAT3(300, 0, 100), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));

			 /*
			 Renderable* c1 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(300, 0, 100), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c2 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(200, 0, 100), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c3 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(400, 0, 200), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c7 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(350, 0, 150), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c8 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(250, 0, 150), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c9 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(450, 0, 250), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c4 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(-200, 0, 300), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c5 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(-300, 0, 400), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));
			 Renderable* c6 = GameUtilities::AddDestructableFromModelFile("files/models/Minion_1.AnimMesh", "files/textures/Minion_1.mat", woodMinionAnimations, XMFLOAT3(-400, 0, 100), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT3(0, 0, 0));

			 c1->physicsType = ADResource::ADGameplay::STATIC;
			 c2->physicsType = ADResource::ADGameplay::STATIC;
			 c3->physicsType = ADResource::ADGameplay::STATIC;
			 c4->physicsType = ADResource::ADGameplay::STATIC;
			 c5->physicsType = ADResource::ADGameplay::STATIC;
			 c6->physicsType = ADResource::ADGameplay::STATIC;
			 c7->physicsType = ADResource::ADGameplay::STATIC;
			 c8->physicsType = ADResource::ADGameplay::STATIC;
			 c9->physicsType = ADResource::ADGameplay::STATIC;*/

		float mapWidth = 1000;
		float mapLength = 1000;
		//golem->testAttack->hitbox = ess1;  

		Renderable* tempPlane = GameUtilities::AddSimpleAsset("files/models/Ground.mesh", "files/textures/Ground.mat", XMFLOAT3(0, 0, 0), XMFLOAT3(mapWidth, 100, mapLength), XMFLOAT3(0, 0, 0));

		//GameUtilities::AddGameObject(cube);

		GameUtilities::AddGameObject(dynamic_cast<GameObject*>(golem));

		//GameUtilities::AddGameObject(ess1);
		//GameUtilities::AddGameObject(c1);
		//GameUtilities::AddGameObject(c2);
		//GameUtilities::AddGameObject(c3);
		//GameUtilities::AddGameObject(c4);
		//GameUtilities::AddGameObject(c5);
		//GameUtilities::AddGameObject(c6);
		//GameUtilities::AddGameObject(c7);
		//GameUtilities::AddGameObject(c8);
		//GameUtilities::AddGameObject(c9);
		//GameUtilities::AddGameObject(c2);
		////GameUtilities::AddGameObject(a1);
		//GameUtilities::AddGameObject(a2);

		for (int i = 0; i < 10; i++)
		{
			GameUtilities::AddGameObject(stoneMinions[i]);
			GameUtilities::AddGameObject(waterMinions[i]);
			GameUtilities::AddGameObject(fireMinions[i]);
			GameUtilities::AddGameObject(woodMinions[i]);
		}
		GameUtilities::AddGameObject(m1);
		//GameUtilities::AddGameObject(m2);
		//GameUtilities::AddGameObject(t1);
		//GameUtilities::AddGameObject(testPlane);
		//GameUtilities::AddGameObject(AnimationTester);
		GameUtilities::AddGameObject(tempPlane);

		/*GameUtilities::GenerateHouse1(XMFLOAT3(0, 0, 0), XMFLOAT3(0,45,0));

		GameUtilities::GenerateHouse2(XMFLOAT3(0, 0, -2.5), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateHouse3(XMFLOAT3(0, 0, -5), XMFLOAT3(0, -45, 0));

		GameUtilities::GenerateHouse4(XMFLOAT3(0, 0, -7.5), XMFLOAT3(0, 45, 0));

		GameUtilities::GenerateBarn1(XMFLOAT3(-10, 0, 0), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateBarn2(XMFLOAT3(-10, 0, 2.5), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateGateway(XMFLOAT3(10, 0, 0), XMFLOAT3(0, -90, 0));

		GameUtilities::GenerateScaffolding(XMFLOAT3(10, 0, 1), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateWall(XMFLOAT3(10, 0, 5), XMFLOAT3(0, 45, 0));

		GameUtilities::GenerateWell(XMFLOAT3(-5, 0, 0), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateTavern(XMFLOAT3(-5, 0, 5), XMFLOAT3(0, 45, 0));

		GameUtilities::GenerateTree(XMFLOAT3(-2.5, 0, 2.5), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateLadder(XMFLOAT3(-2.5, 0, 0), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateRock1(XMFLOAT3(-2, 0, 2), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateLamp(XMFLOAT3(2.5, 0, 2.5), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateMeetingHall(XMFLOAT3(5, 0, 5), XMFLOAT3(0, 225, 0));

		GameUtilities::GenerateTower(XMFLOAT3(2.5, 0, 0), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateCart(XMFLOAT3(-2.5, 0, 1), XMFLOAT3(0,0,0));

		GameUtilities::GenerateStraw(XMFLOAT3(-2.5, 0, 2), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateDebris1(XMFLOAT3(-2.5, 0, 5), XMFLOAT3(0, 0, 0));

		GameUtilities::GenerateRubble1(XMFLOAT3(0, 0, 5), XMFLOAT3(0, 0, 0));
		GameUtilities::GenerateRubble2(XMFLOAT3(-2, 0, 5), XMFLOAT3(0, 0, 0));
		GameUtilities::GenerateRubble3(XMFLOAT3(2, 0, 5), XMFLOAT3(0, 0, 0));*/

#ifdef _DEBUG
#ifdef ShowColliders
		GameUtilities::AddGameObject(golemCollider);
		GameUtilities::AddGameObject(cubeCollider);

		for (auto it = DefinitionDatabase::Instance()->hitboxDatabase.begin(), itEnd = DefinitionDatabase::Instance()->hitboxDatabase.end(); it != itEnd; ++it)
		{
			GameUtilities::AddGameObject(it->second);
		}
#endif
#endif

		SimpleModel** tempPlaneModel = ResourceManager::GetSimpleModelPtrFromMeshId(tempPlane->GetMeshId());
		std::vector<ADPhysics::Triangle> ground;
		std::vector<ADQuadTreePoint<ADPhysics::Triangle>> treePoints;
		XMMATRIX groundWorld = XMMatrixIdentity();
		SimpleStaticModel* planeModel = static_cast<SimpleStaticModel*>(*tempPlaneModel);
		tempPlane->GetWorldMatrix(groundWorld);
		for (unsigned int i = 0; i < (*planeModel).indices.size(); i += 3)
		{
			XMFLOAT3 A = planeModel->vertices[(*planeModel).indices[i]].Position;
			XMFLOAT3 B = planeModel->vertices[(*planeModel).indices[i + 1]].Position;
			XMFLOAT3 C = planeModel->vertices[(*planeModel).indices[i + 2]].Position;

			A = (XMFLOAT3&)(XMVector3Transform(Float3ToVector(A), groundWorld));
			B = (XMFLOAT3&)(XMVector3Transform(Float3ToVector(B), groundWorld));
			C = (XMFLOAT3&)(XMVector3Transform(Float3ToVector(C), groundWorld));

			ADPhysics::Triangle* tri = new Triangle(A, B, C);

			ground.push_back(*tri);

			XMFLOAT3 centroid = (XMFLOAT3&)((Float3ToVector(tri->a) + Float3ToVector(tri->b) + Float3ToVector(tri->c)) / 3);
			ADQuadTreePoint<ADPhysics::Triangle> point = ADQuadTreePoint<ADPhysics::Triangle>(centroid.x, centroid.z, *tri);
			treePoints.push_back(point);
		}

		ADQuad boundary = ADQuad((*planeModel).position.x, (*planeModel).position.z, 1000, 1000);
		QuadTree<ADPhysics::Triangle>* tree = new QuadTree<ADPhysics::Triangle>(boundary);

		for (unsigned int i = 0; i < treePoints.size(); i++)
		{
			tree->Insert(treePoints[i]);
		}

		float minionWidth = 10;





		//FountainEmitter femitter;
		//femitter.Initialize(engine->GetPBRRenderer()->GetRendererResources()->device.Get(), 100, XMFLOAT4(1, 1, 1, 0), L"files\\textures\\Particle_Dust.dds", 1000);
		//ParticleEmitterEvent golemPunchParticles(femitter);
		//golemPunchParticles.lifespan = 1000.f;
		//ADEvents::ADEventSystem::Instance()->RegisterClient("Sfx_GolemPunch", &golemPunchParticles);



		// Timing
		game_time.Restart();

		// String shit
		std::string fr; std::wstring tfw; const wchar_t* wchar;

		// Construct physics stuff
		//test_colider1 = ADPhysics::AABB(XMFLOAT3(0, 5, 15), XMFLOAT3(2, 2, 2));
		//test_plane = ADPhysics::Plane(XMMatrixTranslation(0, -5, 0), XMFLOAT3(10, 0, 10));

		//Needed to add this to the colliders for the collision queue

		/*c2->colliderPtr = &test_colider1;
		c2->type = OBJECT_TYPE::STATIC;*/

		ADAI::ADPathfinding pathfinder;
		pathfinder.Initialize(&planeModel->vertices, XMFLOAT2(mapWidth, mapLength), minionWidth, 20.f);
		gameUI.SetupUI(engine->GetUI(), golem, &audioEngine, pathfinder.GetPlaneNodes(), pathfinder.tileMap.columns, mapWidth, mapLength);

		ADEvents::ADEventSystem::Instance()->SendEvent("PlayTitle", (void*)0);

		while (!shutdown)
		{
			if (Input::QueryButtonDown(GamepadButtons::RightShoulder))
			{
				//pathfinder.clearDebug();
				UINT row;
				UINT column;
				pathfinder.tileMap.GetColumnRowFromPosition(XMFLOAT2(golem->GetPosition().x, golem->GetPosition().z), column, row);
				pathfinder.enter(0, 0, column, row);
			}


			game_time.Signal();
			delta_time = static_cast<float>(game_time.SmoothDelta());
			timer += delta_time;
			//main_music_loop_timer -= delta_time;

			ProcessInput();

			if (!pathfinder.isDone())
			{
				pathfinder.update(0.00001f);
			}
			pathfinder.UpdatePlayerNode(golem->GetPosition().x, golem->GetPosition().z, mapWidth, mapLength);

			ADEvents::ADEventSystem::Instance()->ProcessEvents();

			// Debug draw
			//ResourceManager::GetModelPtrFromMeshId(golem_collider)->position = (*ResourceManager::GetSimpleModelPtrFromMeshId(golem->GetMeshId()))->position;

			//engine->GetOrbitCamera()->SetRadius(200);
			engine->GetOrbitCamera()->SetRadius(50);
			engine->GetOrbitCamera()->SetLookAtAndRotate((XMFLOAT3&)(Float3ToVector(golem->GetPosition()) + XMVectorSet(0, 15, 0, 1)), yaw, pitch, delta_time);
			XMMATRIX view;
			engine->GetOrbitCamera()->GetViewMatrix(view);
			golem->GetView(view);


			XMFLOAT3 CamPosition = engine->GetOrbitCamera()->GetPosition();
			audioEngine.Set3dListenerAndOrientation({ CamPosition.x, CamPosition.y, CamPosition.z });
			audioEngine.Update();

			XMFLOAT4X4 viewPass;
			XMStoreFloat4x4(&viewPass, view);
			XMFLOAT4 cpos = XMFLOAT4(CamPosition.x, CamPosition.y, CamPosition.z, 0);


			XMMATRIX pers = XMMatrixPerspectiveFovLH(engine->GetOrbitCamera()->GetFOV(), (Window->Bounds.Width / Window->Bounds.Height), engine->GetOrbitCamera()->GetNear(), engine->GetOrbitCamera()->GetFar());
			XMFLOAT4X4 persPass;
			XMStoreFloat4x4(&persPass, pers);
			//femitter.UpdateParticles(delta_time, viewPass, persPass, cpos);

#ifdef _DEBUG
			golemCollider->transform = golem->GetColliderInfo();
			XMMATRIX colliderLocation = cube->transform;

			cube->colliderPtr = &AABB(VectorToFloat3(colliderLocation.r[3]), XMFLOAT3(1, 1, 1));
			cube->physicsType = COLLIDABLE;
			cubeCollider->transform = colliderLocation;
#endif

			//Did this to represent layers, Triggers won't collider with other triggers so there is no need to test them

			//This is just tmporary code for a simple collision layer loop, this will be slow but multithreading should help

		//	Works the exact same as the commented code above
			int OBJ_COUNT = ResourceManager::GetGameObjectCount();
			ADResource::ADGameplay::GameObject** OBJS = ResourceManager::GetGameObjectPtr();

			for (int i = 0; i < OBJ_COUNT; i++)
			{
				for (unsigned int j = 0; j < OBJ_COUNT; j++)
				{
					if (i != j)
					{
						if (OBJS[i]->colliderPtr != nullptr && OBJS[j]->colliderPtr != nullptr)
						{
							if (!OBJS[i]->colliderPtr->trigger || !OBJS[j]->colliderPtr->trigger)
							{
								if (OBJS[i]->colliderPtr->type != ColliderType::Plane || OBJS[j]->colliderPtr->type != ColliderType::Plane)
								{
									if (OBJS[i]->colliderPtr->type != ColliderType::Triangle || OBJS[j]->colliderPtr->type != ColliderType::Triangle)
									{
										OBJS[i]->CheckCollision(OBJS[j]);
									}
								}
							}
						}
					}
				}
			}

			//Resolve all collisions that occurred this frame
			ADResource::ADGameplay::ResolveCollisions();

			physics_timer += delta_time;
			if (physics_timer > physics_rate)
			{
				physics_timer = 0;
				for (int i = 0; i < 10; i++)
				{
					GroundClamping(stoneMinions[i], tree, delta_time);
					GroundClamping(waterMinions[i], tree, delta_time);
					GroundClamping(fireMinions[i], tree, delta_time);
					GroundClamping(woodMinions[i], tree, delta_time);
				}


			}

			GroundClamping(golem, tree, delta_time);
			//GroundClamping(cube, tree, delta_time);
			//cube->transform.r[3].m128_f32[1] += 5;

			// Poll input
			Window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			GolemAnimController.SetModel_To_CurrentAnimation();

			// D3d11 shit
			if (!engine->Update()) break;
			if (!engine->Render()) break;
			//femitter.RenderParticles(engine->GetPBRRenderer()->GetRendererResources()->context.Get());

			// Update framerate
			if (timer > 1)
			{
				timer = 0;

				fr = std::to_string(1.0 / delta_time) + " FPS";
				tfw = std::wstring(fr.begin(), fr.end());
				wchar = tfw.c_str();
				ApplicationView::GetForCurrentView()->Title = ref new String(wchar);
			}
		}
		audioEngine.Shutdown();
	}
	virtual void Uninitialize() {}

	// an "event" that is called when the application window is ready to be activated
	void OnActivated(CoreApplicationView^ CoreAppView, IActivatedEventArgs^ Args)
	{
		CoreWindow^ Window = CoreWindow::GetForCurrentThread();
		Window->Activate();
	}

	void ProcessInput()
	{
		//if (Input::QueryButtonDown(GamepadButtons::DPadLeft))
		//{
		//	if (effect_triggered) audio_manager->ResumeEffect(7, effect_id);
		//	else {
		//		effect_triggered = true;
		//		effect_id = audio_manager->PlayEffect(7);
		//	}
		//}
		//if (Input::QueryButtonDown(GamepadButtons::DPadRight))
		//{
		//	audio_manager->PauseEffect(0, effect_id);
		//}

		static float camera_rotation_thresh = 250;
		float dt = delta_time;

		if (Input::ControllerPresent())
		{
			// Camera rotation
			if (Input::QueryThumbStickLeftRightX(Input::THUMBSTICKS::RIGHT_THUMBSTICK) == (int)Input::DIRECTION::RIGHT)
			{
				yaw += camera_rotation_thresh * dt;
			}
			else if (Input::QueryThumbStickLeftRightX(Input::THUMBSTICKS::RIGHT_THUMBSTICK) == (int)Input::DIRECTION::LEFT)
			{
				yaw += -camera_rotation_thresh * dt;
			}
			if (Input::QueryThumbStickUpDownY(Input::THUMBSTICKS::RIGHT_THUMBSTICK) == (int)Input::DIRECTION::UP)
			{
				pitch += -camera_rotation_thresh * dt;
			}
			else if (Input::QueryThumbStickUpDownY(Input::THUMBSTICKS::RIGHT_THUMBSTICK) == (int)Input::DIRECTION::DOWN)
			{
				pitch += camera_rotation_thresh * dt;
			}

			if (Input::QueryButtonDown(GamepadButtons::LeftThumbstick) && Input::QueryButtonDown(GamepadButtons::RightThumbstick))
			{
				shutdown = true;
			}
			else if (Input::QueryButtonDown(GamepadButtons::RightThumbstick))
			{
				yaw = default_yaw;
				pitch = default_pitch;
			}
		}
	}

	void OnKeyDown(CoreWindow^ Window, KeyEventArgs^ args)
	{
		bool enabled = false;

		if (enabled)
		{
			static float camera_movement_thresh = 5;
			static float camera_rotation_thresh = 500;

			float dt = delta_time;

			if (args->VirtualKey == VirtualKey::Escape)
			{
				shutdown = true;
			}
			if (args->VirtualKey == VirtualKey::W || args->VirtualKey == VirtualKey::GamepadLeftThumbstickUp)
			{
			}
			else if (args->VirtualKey == VirtualKey::S || args->VirtualKey == VirtualKey::GamepadLeftThumbstickDown)
			{
			}
			else if (args->VirtualKey == VirtualKey::A || args->VirtualKey == VirtualKey::GamepadLeftThumbstickLeft)
			{
			}
			else if (args->VirtualKey == VirtualKey::D || args->VirtualKey == VirtualKey::GamepadLeftThumbstickRight)
			{
			}

			if (args->VirtualKey == VirtualKey::Left || args->VirtualKey == VirtualKey::GamepadRightThumbstickRight)
			{
				yaw += camera_rotation_thresh * dt;
			}
			else if (args->VirtualKey == VirtualKey::Right || args->VirtualKey == VirtualKey::GamepadRightThumbstickLeft)
			{
				yaw += -camera_rotation_thresh * dt;
			}

			if (args->VirtualKey == VirtualKey::Up || args->VirtualKey == VirtualKey::GamepadRightThumbstickDown)
			{
				pitch += camera_rotation_thresh * dt;
			}
			else if (args->VirtualKey == VirtualKey::Down || args->VirtualKey == VirtualKey::GamepadRightThumbstickUp)
			{
				pitch += -camera_rotation_thresh * dt;
			}
		}
	}
};

// the class definition that creates an instance of our core framework class
ref class AppSource sealed : IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView()
	{
		return ref new App();    // create an App class and return it
	}
};
[MTAThread]    // define main() as a multi-threaded-apartment function

// the starting point of all programs
int main(Array<String^>^ args)
{
	CoreApplication::Run(ref new AppSource());    // create and run a new AppSource class
	return 0;
}