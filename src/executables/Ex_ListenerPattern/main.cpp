//===================================================================//
//==================Your Includes===================================//
//==================================================================//
#include <GL/glew.h>

#include <GeKo_Graphics/InputInclude.h>
#include <GeKo_Graphics/GeometryInclude.h>
#include <GeKo_Graphics/Camera/StrategyCamera.h>
#include <GeKo_Graphics/Camera/Pilotview.h>
#include <GeKo_Graphics/ShaderInclude.h>

#include <GeKo_Graphics/AIInclude.h>
#include "GeKo_Gameplay/AI_Decisiontree/DecisionTree.h"
#include <GeKo_Gameplay/Object/Geko.h>
#include <GeKo_Gameplay/Object/AI.h>

#include <GeKo_Graphics/Geometry/AntMesh.h>
#include <GeKo_Graphics/Geometry/TreeMesh.h>
#include <GeKo_Graphics/Geometry/GekoMesh.h>
#include <GeKo_Graphics/Geometry/Plane.h>


#include <GeKo_Physics/CollisionTest.h>

#include <GeKo_Graphics/Observer/ObjectObserver.h>
#include <GeKo_Graphics/Observer/CollisionObserver.h>
#include <GeKo_Graphics/Observer/GravityObserver.h>
#include <GeKo_Graphics/Observer/SoundObserver.h>
#include <GeKo_Graphics/Observer/QuestObserver.h>

#include <GeKo_Gameplay/Questsystem/ItemReward.h>
#include <GeKo_Gameplay/Questsystem/ExpReward.h>
#include <GeKo_Gameplay/Questsystem/Goal_Kill.h>
#include <GeKo_Gameplay/Questsystem/Goal_Eaten.h>

//===================================================================//
//==================Things you need globally==========================//
//==================================================================//
InputHandler iH;
StrategyCamera cam("StrategyCam");
Geko geko("Geko", glm::vec3(10.0, 3.0, -5.0));
Renderer *renderer;

//===================================================================//
//==================Callbacks for the Input==========================//
//==================================================================//

void playKey_callback(GLFWwindow* window)
{
	//Recognizing if the player wants to move its character
	if (glfwGetKey(window, GLFW_KEY_UP))
	{
		geko.moveForward();
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN))
	{
		geko.moveBackward();
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT))
	{
		geko.moveRight();
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT))
	{
		geko.moveLeft();
	}

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	std::map<int, std::function<void()>> activeMap = iH.getActiveInputMap()->getMap();

	iH.getActiveInputMap()->setGLFWwindow(window);

	for (std::map<int, std::function<void()>>::iterator it = activeMap.begin(); it != activeMap.end(); it++){
		if (it->first == key)
			activeMap.at(key)();
		if (it == activeMap.end())
			std::cout << "Key is not mapped to an action" << std::endl;
	}
}

void mouse_callback(GLFWwindow* window)
{
	int i = 0;
	if (glfwGetMouseButton(window, i) == GLFW_PRESS)
	{
		std::map<int, std::function<void()>> activeMap = iH.getActiveInputMap()->getMap();

		iH.getActiveInputMap()->setGLFWwindow(window);

		for (std::map<int, std::function<void()>>::iterator it = activeMap.begin(); it != activeMap.end(); it++){
			if (it->first == i)
				activeMap.at(i)();
			if (it == activeMap.end())
				std::cout << "Key is not mapped to an action" << std::endl;
		}
	}
	else{
		cam.updateCursor(window);
	}
}

void mouseScroll_callback(GLFWwindow* window, double offsetX, double offSetY)
{

	std::map<int, std::function<void()>> activeMap = iH.getActiveInputMap()->getMap();

	iH.getActiveInputMap()->setGLFWwindow(window);

	if (offSetY < 0)
	{
		for (std::map<int, std::function<void()>>::iterator it = activeMap.begin(); it != activeMap.end(); it++){
			if (it->first == 001)
				activeMap.at(001)();
			if (it == activeMap.end())
				std::cout << "Key is not mapped to an action" << std::endl;
		}
	}
	else{
		for (std::map<int, std::function<void()>>::iterator it = activeMap.begin(); it != activeMap.end(); it++){
			if (it->first == 002)
				activeMap.at(002)();
			if (it == activeMap.end())
				std::cout << "Key is not mapped to an action" << std::endl;
		}
	}
}

//===================================================================//
//==================Main Method=====================================//
//==================================================================//
int main()
{

	//===================================================================//
	//==================Things you need to start with====================//
	//==================================================================//
	glfwInit();

	Window testWindow(500, 50, 800, 600, "testWindow");
	glfwMakeContextCurrent(testWindow.getWindow());

	cam.setCenter(glm::vec4(0.0, 10.0, 20.0, 1.0));
	cam.setName("StrategyCam");
	cam.setKeySpeed(2.0);
	cam.setNearFar(0.01, 100);

	glfwSetKeyCallback(testWindow.getWindow(), key_callback);
	glfwSetScrollCallback(testWindow.getWindow(), mouseScroll_callback);

	glewInit();

	OpenGL3Context context;
	renderer = new Renderer(context);

	//===================================================================//
	//==================Shaders for your program========================//
	//==================================================================//
	VertexShader vs(loadShaderSource(SHADERS_PATH + std::string("/Vertex-Shaders/TextureShader3D.vert")));
	FragmentShader fs(loadShaderSource(SHADERS_PATH + std::string("/Fragment-Shaders/TextureShader3D.frag")));
	ShaderProgram shader(vs, fs);


	//===================================================================//
	//==================A Graph for the AI-Unit=========================//
	//==================================================================//

	glm::vec3 posFood(10.0, 0.0, -5.0);
	glm::vec3 posSpawn(3.0, 0.0, 3.0);
	Graph<AStarNode, AStarAlgorithm> testGraph;

	//===================================================================//
	//==================A Decision-Tree for the AI-Unit==================//
	//==================================================================//

	DecisionTree defaultTree;
	defaultTree.setAntTreeAggressiv();


	//===================================================================//
	//==================Object declarations - Geometry, Texture, Node=== //
	//==========================Object: AI==============================//

	Teapot teaAnt;

	Texture texPlayer((char*)RESOURCES_PATH "/Snake.jpg");
	SoundFileHandler sfh = SoundFileHandler(1000);

	AStarNode defaultASNode();
	AI ant_Flick;
	ant_Flick.setAntAfraid();

	AntMesh ant;

	Node aiNode("Flick");
	aiNode.addGeometry(&ant);
	aiNode.addTexture(&texPlayer);
	aiNode.addTranslation(ant_Flick.getPosition().x, ant_Flick.getPosition().y, ant_Flick.getPosition().z);
	aiNode.setObject(&ant_Flick);
	ant_Flick.setSoundHandler(&sfh);
	
	ant_Flick.setSourceName(MOVESOUND_AI, "AIFootsteps", RESOURCES_PATH "/Sound/Footsteps.wav");
	ant_Flick.setSourceName(DEATHSOUND_AI, "AIDeath", RESOURCES_PATH "/Sound/death.wav");
	sfh.disableLooping("AIDeath");
	ant_Flick.setSourceName(EATSOUND_AI, "AIEssen", RESOURCES_PATH "/Sound/Munching.wav");
	sfh.disableLooping("AIEssen");




	//===================================================================//
	//==================Object declarations - Geometry, Texture, Node=== //
	//==========================Object: Player===========================//
	Teapot teaPlayer;
	Texture texCV((char*)RESOURCES_PATH "/cv_logo.bmp");

	GekoMesh gekoMesh;
	geko.setExp(0.0);
	geko.setLevelThreshold(100.0);
	geko.setLevel(0);

	Node playerNode("Player");

	playerNode.addGeometry(&gekoMesh);
	playerNode.setObject(&geko);
	playerNode.addTexture(&texCV);

	sfh.generateSource(posFood, RESOURCES_PATH "/Sound/Rascheln.wav");
	geko.setSoundHandler(&sfh);
	geko.setSourceName(MOVESOUND, "SpielerFootsteps", RESOURCES_PATH "/Sound/Rascheln.wav");
	geko.setSourceName(BACKGROUNDMUSIC, "Hintergrund", RESOURCES_PATH "/Sound/jingle2.wav");
	geko.setSourceName(FIGHTSOUND, "Kampfsound", RESOURCES_PATH "/Sound/punch.wav");
	geko.setSourceName(EATSOUND, "Essen", RESOURCES_PATH "/Sound/Munching.wav");
	geko.setSourceName(QUESTSOUND, "Quest", RESOURCES_PATH "/Sound/jingle.wav");
	geko.setSourceName(ITEMSOUND, "Item", RESOURCES_PATH "/Sound/itempickup.wav");

	sfh.disableLooping("Essen");
	sfh.disableLooping("Quest");
	sfh.disableLooping("Item");
	sfh.generateSource("Feuer",posFood, RESOURCES_PATH "/Sound/Feuer kurz.wav");

	playerNode.setCamera(&cam);

	// ==============================================================
	// == Items =====================================================
	// ==============================================================




	//===================================================================//
	//==================Object declarations - Geometry, Texture, Node=== //
	//==========================Object: Tree===========================//

	StaticObject treeStatic;
	treeStatic.setTree();

	TreeMesh tree;
	Node treeNode("Tree");
	treeNode.addGeometry(&tree);
	treeNode.setObject(&treeStatic);
	treeNode.addTranslation(posFood);
	treeNode.getBoundingSphere()->radius = 3.0;

	//===================================================================//
	//==================Object declarations - Geometry, Texture, Node=== //
	//==========================Object: Plane===========================//

	Plane terrain;
	Texture terrainTex((char*)RESOURCES_PATH "/Grass.jpg");

	Node terrainNode("Plane");
	terrainNode.addGeometry(&terrain);
	terrainNode.addTexture(&terrainTex);
	terrainNode.addTranslation(0.0, -0.75, 0.0);
	terrainNode.addRotation(90.0f, glm::vec3(1.0, 0.0, 0.0));
	terrainNode.addScale(20.0, 20.0, 20.0);

	//===================================================================//
	//==================Setting up the Level and Scene==================//
	//==================================================================//

	Level testLevel("testLevel");
	Scene testScene("testScene");
	testLevel.addScene(&testScene);
	testLevel.changeScene("testScene");

	//==================Add Camera to Scene============================//
	testScene.getScenegraph()->addCamera(&cam);
	testScene.getScenegraph()->setActiveCamera("StrategyCam");

	//==================Set Input-Maps and activate one================//
	iH.setAllInputMaps(*(testScene.getScenegraph()->getActiveCamera()));
	iH.changeActiveInputMap("Strategy");

	//==================Add Objects to the Scene=======================//
	//==================Update the Bounding-Sphere 1st time============//
	testScene.getScenegraph()->getRootNode()->addChildrenNode(&aiNode);

	testScene.getScenegraph()->getRootNode()->addChildrenNode(&playerNode);

	testScene.getScenegraph()->getRootNode()->addChildrenNode(&terrainNode);

	testScene.getScenegraph()->getRootNode()->addChildrenNode(&treeNode);


	// ==============================================================
	// == Questsystem ====================================================
	// ==============================================================
	//QuestHandler questhandler;


	/*Quest questCollect(1);
	

	questCollect.setActive(true);
	Goal_Collect goalCollect(1);
	Goal_Collect goalCollect3(3);
	questCollect.addGoal(&goalCollect);
	questCollect.addGoal(&goalCollect3);

	ExpReward expReward(1);
	expReward.setExp(100);
	questCollect.addReward(&expReward);

	Item cookie2(100);
	cookie2.setName("Cookie100");
	cookie2.setTypeId(ItemType::COOKIE);
	ItemReward itemReward(2);
	itemReward.addItem(&cookie2);

	questCollect.addReward(&itemReward);

	Quest questCollect2(2);
	questCollect2.setActive(true);
	Goal_Collect goalCollect2(2);
	questCollect2.addGoal(&goalCollect2);
	questCollect2.addReward(&expReward);
	questCollect2.addReward(&itemReward);

	goalCollect.setGoalCount(50);
	goalCollect2.setGoalCount(50);
	goalCollect3.setGoalCount(50);

	goalCollect.setItemType(ItemType::BRANCH);
	goalCollect2.setItemType(ItemType::BRANCH);
	goalCollect3.setItemType(ItemType::COOKIE);

	testLevel.getQuestHandler()->addQuest(&questCollect);
	testLevel.getQuestHandler()->addQuest(&questCollect2);*/

	Quest questKillAnt(1);
	Quest questEatAnt(2);
	Quest questCollectCookie(3);
	Quest questCollectBranch(4);

	Goal_Kill killAnt(1);
	Goal_Eaten eatAnt(2);
	Goal_Collect collectCookie(3);
	Goal_Collect collectBranch(4);

	questKillAnt.addGoal(&killAnt);
	questEatAnt.addGoal(&eatAnt);
	questCollectCookie.addGoal(&collectCookie);
	questCollectBranch.addGoal(&collectBranch);

	killAnt.setGoalCount(1);
	eatAnt.setGoalCount(1);
	collectCookie.setGoalCount(1);
	collectBranch.setGoalCount(1);

	collectBranch.setItemType(ItemType::BRANCH);
	collectCookie.setItemType(ItemType::COOKIE);

	ExpReward expReward(1);
	expReward.setExp(100);

	questKillAnt.addReward(&expReward);
	questEatAnt.addReward(&expReward);
	questCollectCookie.addReward(&expReward);
	questCollectBranch.addReward(&expReward);

	QuestGraph questGraph;
	QuestGraphNode nodeStart;
	nodeStart.setQuest(&questKillAnt);
	questGraph.addNode(&nodeStart);
	questKillAnt.setActive(true);

	QuestGraphNode nodeSecond;
	nodeSecond.setQuest(&questEatAnt);
	nodeSecond.setParent(&nodeStart);
	questGraph.addNode(&nodeSecond);

	QuestGraphNode nodeThird;
	nodeThird.setQuest(&questCollectCookie);
	nodeThird.setParent(&nodeSecond);
	questGraph.addNode(&nodeThird);

	QuestGraphNode nodeFourth;
	nodeFourth.setQuest(&questCollectBranch);
	nodeFourth.setParent(&nodeThird);
	questGraph.addNode(&nodeFourth);

	testLevel.getQuestHandler()->addQuest(&questKillAnt);
	testLevel.getQuestHandler()->addQuest(&questEatAnt);
	testLevel.getQuestHandler()->addQuest(&questCollectBranch);
	testLevel.getQuestHandler()->addQuest(&questCollectCookie);

	testLevel.getQuestHandler()->setGraph(&questGraph);
	

	//===================================================================//
	//==================Setting up the Collision=========================//
	//==================================================================//
	CollisionTest collision;
	collision.collectNodes(testScene.getScenegraph()->getRootNode());

	//===================================================================//
	//==================Setting up the Observers========================//
	//==================================================================//
	ObjectObserver aiObserver(&testLevel);
	SoundObserver soundAIObserver(&testLevel);
	ant_Flick.addObserver(&aiObserver);
	ant_Flick.addObserver(&soundAIObserver);

	ObjectObserver playerObserver(&testLevel);
	SoundObserver soundPlayerObserver(&testLevel);
	geko.addObserver(&playerObserver);
	geko.addObserver(&soundPlayerObserver);

	CollisionObserver colObserver(&testLevel);
	collision.addObserver(&colObserver);
	collision.addObserver(&soundPlayerObserver);

	GravityObserver gravityObserver;
	collision.addObserver(&gravityObserver);

	QuestObserver questObserver(&testLevel);
	/*questCollect.addObserver(&questObserver);
	questCollect2.addObserver(&questObserver);
	questCollect.addObserver(&soundPlayerObserver);
	questCollect2.addObserver(&soundPlayerObserver);
	
	goalCollect.addObserver(&questObserver);
	goalCollect2.addObserver(&questObserver);
	goalCollect3.addObserver(&questObserver);*/
	questKillAnt.addObserver(&questObserver);
	questEatAnt.addObserver(&questObserver);
	questCollectCookie.addObserver(&questObserver);
	questCollectBranch.addObserver(&questObserver);

	killAnt.addObserver(&questObserver);
	eatAnt.addObserver(&questObserver);
	collectCookie.addObserver(&questObserver);
	collectBranch.addObserver(&questObserver);

	testLevel.getFightSystem()->addObserver(&questObserver);



	//===================================================================//
	//==================Setting up the Gravity===========================//
	//==================================================================//
	Gravity gravity;
	playerNode.addGravity(&gravity);
	aiNode.addGravity(&gravity);

	//===================================================================//
	//==================The Render-Loop==================================//
	//==================================================================//
	float lastTime = glfwGetTime();

	sfh.playSource("Feuer");
	sfh.playSource("Hintergrund");
	sfh.setGain("Hintergrund", 0.5f);

	while (!glfwWindowShouldClose(testWindow.getWindow()))
	{
		
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		
		mouse_callback(testWindow.getWindow());

		
		//===================================================================//
		//==================Update your Objects per Frame here =============//
		//==================================================================//
		collision.update();

		ant_Flick.update();


		//===================================================================//
		//==================Input and update for the Player==================//
		//==================================================================//

		playKey_callback(testWindow.getWindow());
		geko.update(deltaTime);

		//===================================================================//
		//==================Render your Objects==============================//
		//==================================================================//
	
		renderer->renderScene(testScene, testWindow);

	}

	glfwDestroyWindow(testWindow.getWindow());
	glfwTerminate();

	
	return 0;

}

