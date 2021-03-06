#include <GL/glew.h>
#include <GeKo_Graphics/InputInclude.h>
#include <GeKo_Graphics/MaterialInclude.h>
#include <GeKo_Graphics/GeometryInclude.h>
#include <GeKo_Graphics/ShaderInclude.h>
#include <GeKo_Graphics/ScenegraphInclude.h>
#include "GeKo_Graphics/Camera/Pilotview.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

InputHandler iH;
Pilotview cam("Pilotview");

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	std::map<int, std::function<void()>> activeMap = iH.getActiveInputMap()->getMap();

	for (std::map<int, std::function<void()>>::iterator it = activeMap.begin(); it != activeMap.end(); it++){
		if (it->first == key)
			activeMap.at(key)();
		if (it == activeMap.end())
			std::cout << "Key is not mapped to an action" << std::endl;
	}
}

int main()
{
	glfwInit();

	Window testWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, "Blur");
	glfwMakeContextCurrent(testWindow.getWindow());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	

	cam.setName("PilotviewCam");
	cam.setPosition(glm::vec4(1.0, 1.0, 3.0, 1.0));
	cam.setNearFar(0.01f, 100.0f);

	iH.setAllInputMaps(cam);
	iH.changeActiveInputMap("Pilotview");

	//Callback
	glfwSetKeyCallback(testWindow.getWindow(), key_callback);

	glewInit();

	//our shader
	VertexShader vsGBuffer(loadShaderSource(SHADERS_PATH + std::string("/GBuffer/GBuffer.vert")));
	FragmentShader fsGBuffer(loadShaderSource(SHADERS_PATH + std::string("/GBuffer/GBuffer.frag")));
	ShaderProgram shaderGBuffer(vsGBuffer, fsGBuffer);

	VertexShader vsSM(loadShaderSource(SHADERS_PATH + std::string("/ShadowMapping/ShadowMap.vert")));
	FragmentShader fsSM(loadShaderSource(SHADERS_PATH + std::string("/ShadowMapping/ShadowMap.frag")));
	ShaderProgram shadowmapShader(vsSM, fsSM);

	VertexShader vsSfq(loadShaderSource(SHADERS_PATH + std::string("/ScreenFillingQuad/ScreenFillingQuad.vert")));
	FragmentShader fsSfq(loadShaderSource(SHADERS_PATH + std::string("/ScreenFillingQuad/ScreenFillingQuad.frag")));
	ShaderProgram shaderSFQ(vsSfq, fsSfq);

	//our renderer
	OpenGL3Context context;
	Renderer renderer(context);

	FBO fboGBuffer(WINDOW_WIDTH, WINDOW_HEIGHT, 3, true, false);
	FBO fboDepth(WINDOW_WIDTH, WINDOW_HEIGHT, 3, true, false);

	//our object
	Cube cube;
	Teapot teapot;

	Rect plane;
	Rect screenFillingQuad;
	screenFillingQuad.loadBufferData();

	//our textures
	Texture bricks((char*)RESOURCES_PATH "/brick.bmp");
	Texture marble((char*)RESOURCES_PATH "/seamless_marble.jpg");
	Texture chrome((char*)RESOURCES_PATH "/chrome.jpg");

	//Scene creation 
	Level testLevel("testLevel");
	Scene testScene("testScene");
	testLevel.addScene(&testScene);
	testLevel.changeScene("testScene");

	//Add Camera to scenegraph
	testScene.getScenegraph()->addCamera(&cam);
	testScene.getScenegraph()->getCamera("PilotviewCam");
	testScene.getScenegraph()->setActiveCamera("PilotviewCam");

	Node cube1("cube1");
	cube1.addGeometry(&cube);
	cube1.addTexture(&bricks);
	cube1.setModelMatrix(glm::translate(cube1.getModelMatrix(), glm::vec3(-0.3, 0.25, 0.2)));
	cube1.setModelMatrix(glm::scale(cube1.getModelMatrix(), glm::vec3(0.3, 0.3, 0.3)));

	Node cube2("cube2");
	cube2.addGeometry(&cube);
	cube2.addTexture(&bricks);
	cube2.setModelMatrix(glm::translate(cube2.getModelMatrix(), glm::vec3(0.7, 0.25, 0.3)));
	cube2.setModelMatrix(glm::scale(cube2.getModelMatrix(), glm::vec3(0.3, 0.3, 0.3)));


	Node wallNode1("wall1");
	wallNode1.addGeometry(&plane);
	wallNode1.addTexture(&marble);
	wallNode1.setModelMatrix(glm::translate(wallNode1.getModelMatrix(), glm::vec3(0.0, 0.1, 0.2)));
	wallNode1.setModelMatrix(glm::rotate(wallNode1.getModelMatrix(), 90.0f, glm::vec3(1.0, 0.0, 0.0)));
	wallNode1.setModelMatrix(glm::scale(wallNode1.getModelMatrix(), glm::vec3(1.5, 1.5, 1.5)));

	Node wallNode2("wall2");
	wallNode2.addGeometry(&plane);
	wallNode2.addTexture(&marble);
	wallNode2.setModelMatrix(glm::translate(wallNode2.getModelMatrix(), glm::vec3(0.0, 1.0, -0.2)));
	wallNode2.setModelMatrix(glm::scale(wallNode2.getModelMatrix(), glm::vec3(1.5, 1.5, 1.5)));


	Node teaNode("teaNode");
	teaNode.addGeometry(&teapot);
	teaNode.addTexture(&chrome);
	teaNode.setModelMatrix(glm::translate(teaNode.getModelMatrix(), glm::vec3(0.2, 0.3, 0.7)));
	teaNode.setModelMatrix(glm::scale(teaNode.getModelMatrix(), glm::vec3(0.3, 0.3, 0.3)));


	//Creating a scenegraph
	testScene.getScenegraph()->getRootNode()->addChildrenNode(&wallNode1);
	testScene.getScenegraph()->getRootNode()->addChildrenNode(&wallNode2);
	testScene.getScenegraph()->getRootNode()->addChildrenNode(&cube1);
	testScene.getScenegraph()->getRootNode()->addChildrenNode(&cube2);
	testScene.getScenegraph()->getRootNode()->addChildrenNode(&teaNode);

	glm::mat4 prevViewMatrix;
	glm::mat4 prevProjectionMatrix;

	double startTime = glfwGetTime();
	//Renderloop
	while (!glfwWindowShouldClose(testWindow.getWindow()))
	{
		// You have to compute the delta time
		cam.setSensitivity(glfwGetTime() - startTime);

		startTime = glfwGetTime();


		//Bind Shadow Map FBO
		fboDepth.bind();

		//Clear Shadow Map
		glClear(GL_DEPTH_BUFFER_BIT);

		//Set the shader for the light pass. This shader is highly optimized because the scene depth is the only thing that matters here!
		shadowmapShader.bind();
		shadowmapShader.sendMat4("viewMatrix", cam.getViewMatrix());
		shadowmapShader.sendMat4("projectionMatrix", cam.getProjectionMatrix());

		//Render the scene
		testScene.render(shadowmapShader);

		//Restore the default framebuffer
		shadowmapShader.unbind();
		fboDepth.unbind();

		fboGBuffer.bind();
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderGBuffer.bind();
		shaderGBuffer.sendInt("useMotionBlur", 1);
		shaderGBuffer.sendMat4("viewMatrix", cam.getViewMatrix());
		shaderGBuffer.sendMat4("projectionMatrix", cam.getProjectionMatrix());
		shaderGBuffer.sendInt("useTexture", 1);
		shaderGBuffer.sendInt("useMotionBlur", 1);
		shaderGBuffer.sendSampler2D("depthTexture", fboGBuffer.getDepthTexture());
		shaderGBuffer.sendMat4("viewMatrix", cam.getViewMatrix());
		shaderGBuffer.sendMat4("projectionMatrix", cam.getProjectionMatrix());
		shaderGBuffer.sendMat4("previousViewMatrix", prevViewMatrix);
		shaderGBuffer.sendMat4("previousProjectionMatrix", prevProjectionMatrix);
		shaderGBuffer.sendFloat("thresholdValue", 0.9);
		shaderGBuffer.sendFloat("fWindowHeight", WINDOW_HEIGHT);
		shaderGBuffer.sendFloat("fWindowWidth", WINDOW_WIDTH);
		teaNode.setModelMatrix(glm::rotate(teaNode.getModelMatrix(), 10.0f, glm::vec3(0.0, 1.0, 0.0)));
		cube1.setModelMatrix(glm::rotate(cube1.getModelMatrix(), 10.0f, glm::vec3(0.0, 1.0, 0.0)));
		cube2.setModelMatrix(glm::rotate(cube2.getModelMatrix(), 10.0f, glm::vec3(0.0, 1.0, 0.0)));
		testScene.render(shaderGBuffer);
		shaderGBuffer.unbind();
		fboGBuffer.unbind();

		//ScreenFillingQuad Render Pass
		shaderSFQ.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderSFQ.sendSampler2D("fboTexture", fboGBuffer.getColorTexture(2), 2);
		screenFillingQuad.renderGeometry();
		shaderSFQ.unbind();

		prevViewMatrix = cam.getViewMatrix();
		prevProjectionMatrix = cam.getProjectionMatrix();
				
		glfwSwapBuffers(testWindow.getWindow());
		glfwPollEvents();
	}

	glfwDestroyWindow(testWindow.getWindow());
	glfwTerminate();

	return 0;
}