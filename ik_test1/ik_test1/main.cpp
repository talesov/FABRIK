#define GLM_ENABLE_EXPERIMENTAL
#include <glad\glad.h>
#include <glfw\glfw3.h>
#include <iostream>
#include <Windows.h>
#include "image/stb_image.h"
// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Target.h"
#include "Chain.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"glad.lib")
#pragma comment(lib,"stb_image.lib")
#pragma comment(lib,"assimp.lib")
#pragma comment(lib,"winmm.lib") // 告诉连接器与这个库连接，因为我们要播放多媒体声音 
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void Do_Movement(Target* target);
// Window dimensions    
const GLuint WIDTH = 1200, HEIGHT = 720;
float screen_width = 1200.0f;          //窗口宽度
float screen_height = 720.0f;          //窗口高度
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;  // 深度贴图的分辨率
// Camera
Camera camera(glm::vec3(0.0f, 2.0f, 8.0f)); //摄像机位置初始化
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat near_plane = 1.0f, far_plane = 7.5f;  // 视锥的远近平面
glm::vec3 lightPos(-3.0f, 4.0f, -1.0f);

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
static unsigned int LoadTextureFromFile(const char* path)
{
	unsigned int texture_id;
	glGenTextures(1, &texture_id);

	int width, height, nr_channels;
	unsigned char* data = stbi_load(path, &width, &height, &nr_channels, 0);
	if (data)
	{
		GLenum format;
		if (nr_channels == 1)
		{
			format = GL_RED;
		}
		else if (nr_channels == 3)
		{
			format = GL_RGB;
		}
		else if (nr_channels == 4)
		{
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	return texture_id;

}
void minit()
{
	// Init GLFW    
	glfwInit();
	// Set all the required options for GLFW    
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
}

void DrawScene(Shader shader, float current_frame, unsigned int textureID1, unsigned int textureID2);//绘制整个场景

// The MAIN function, from here we start the application and run the game loop
bool flag = true;
unsigned int plane_vbo, plane_vao;
unsigned int cube_vbo, cube_vao;

int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	char desired_model[1000];
	
	int n=5;//关节数量
	float d = 0.2;//骨骼长度
	cout << "IK OpenGL please input 1 to start. ";
	cin >> desired_model;
	cout << "Input the joints number: " << endl;
	cin >> n;
	cout << "Input the bones length: " << endl;
	cin >> d;
	cout << desired_model << endl;
	minit();
	// Create a GLFWwindow object that we can use for GLFW's functions    
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "ink", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// Set the required callback functions    
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Initialize GLEW to setup the OpenGL Function pointers    
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLad" << std::endl;
		return -1;
	}
	// Define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// Load joints
	vector<glm::vec3> joints1;//结点
	//n为模型关节数量
	
	for (int i = 0; i < n; ++i) {
		//float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r = 0.0+d*i;//r是每一节bone的长度
		cout << r << endl;
		joints1.push_back(glm::vec3(0, r, 0));//定位节点位置
	}

	// Load our model object
	Target target(0.0f, 0.0f, 0);//初始化target model 位置

	Chain chain1(joints1, &target);//初始化 chain（链）把节点位置（vector数组）和target输入


	vector<Chain*> vec;
	vec.push_back(new Chain(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), &target));
	vec.push_back(new Chain(glm::vec3(0, 1, 0), glm::vec3(1, 1, 0), &target, 2));

	
	glm::mat4 projection = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);//投影
	glm::mat4 view = camera.GetViewMatrix();

	// 设置离屏渲染帧缓冲(生成深度贴图)
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//解决采样越界
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Shader cube_shader = Shader("res/shader/DrawScene.vs", "res/shader/DrawScene.fs");
	Shader shadowMap_shader = Shader("res/shader/ShadowMap.vs", "res/shader/ShadowMap.fs");
	Shader debugQuad = Shader("res/shader/debugDepthQuad.vs", "res/shader/debugDepthQuad.fs");

	//方块数据
	float cubeVertices[] = {
		//  ---- 位置 ----       ---- 法线 ----   - 纹理坐标 -
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f,
		1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.5f, 0.0f,
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f,
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.5f,

		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
		1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.5f, 0.0f,
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.5f, 0.5f,
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.5f, 0.5f,
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.5f,
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.5f, 0.0f,
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.5f, 0.5f,
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.5f,
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.5f,
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.5f, 0.0f,

		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.5f, 0.0f,
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.5f,
		1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.5f, 0.5f,
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.5f,
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.5f, 0.0f,
		1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.5f,
		1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.5f, 0.5f,
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.5f, 0.0f,
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.5f, 0.0f,
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.5f,

		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.5f,
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 0.5f, 0.0f,
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.5f, 0.5f,
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.5f, 0.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.5f,
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f
	};
	// 地板数据
	float planeVertices[] = {
		5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

		5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
		5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f
	};

	// ---------------------绑定顶点数组对象----------------------
	glGenVertexArrays(1, &cube_vao);
	glGenBuffers(1, &cube_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindVertexArray(cube_vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glGenVertexArrays(1, &plane_vao);
	glGenBuffers(1, &plane_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	glBindVertexArray(plane_vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// 纹理载入
	cube_shader.Use();
	unsigned int diffuse_map = LoadTextureFromFile("res/texture/container2.jpg");
	unsigned int floor = LoadTextureFromFile("res/texture/floor2.jpg");
	cube_shader.SetInt("diffuseTexture", 0);
	cube_shader.SetInt("depthMap", 1);


	// 光空间变换
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);        // 正交投影
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));   // 从光源的位置看向场景中央
	glm::mat4 lightPV = lightProjection * lightView;                                                    // 将世界空间变换到光空间

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	debugQuad.Use();
	debugQuad.SetInt("shadowMap", 0);

	//进入循环 Game loop    
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();

		// Clear the colorbuffer
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// 渲染阴影深度贴图
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);
		shadowMap_shader.Use();
		shadowMap_shader.SetMat4("lightPV", lightPV);
		DrawScene(shadowMap_shader, currentFrame, diffuse_map, floor);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 清除颜色和深度缓冲
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cube_shader.Use();
		cube_shader.SetVec3("viewPos", camera.Position);
		cube_shader.SetMat4("lightPV", lightPV);
		cube_shader.SetVec3("lightPos", lightPos);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection(1);//projection矩阵，投影矩阵
		projection = glm::perspective(glm::radians(camera.Zoom), screen_width / screen_height, 0.1f, 100.0f);
		cube_shader.SetMat4("projection", projection);
		cube_shader.SetMat4("view", view);

		// 绑定纹理贴图
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_map);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		DrawScene(cube_shader, currentFrame, diffuse_map, floor);

		Do_Movement(&target);
		// Transformation matrices
	
		target.Render(view, projection);

		if (strcmp(desired_model, "1") == 0) {
			if(flag==true){
			chain1.Solve();//IK计算得到最终位置
			chain1.Render(view, projection);//绘制造成内存泄漏!!!
			/*修改：把绘制的顶点数据放在loop之前，从segment中提取，否则每次循环都会往缓存中注入数据*/
		}
			else
			{
				chain1.Solve_f();
				chain1.Render(view, projection);
			}
		}
		
		else {
			cout << "Invalid chain model" << endl;
			break;
		}
		// Swap the screen buffers    
		glfwSwapBuffers(window);
		//Sleep(20);
	}

	//释放GLFW分配的内存 Terminate GLFW, clearing any resources allocated by GLFW.    
	glfwTerminate();
	return 0;
}

void DrawScene(Shader shader, float current_frame, unsigned int textureID1, unsigned int textureID2)
{
	// 画地板
	glBindVertexArray(plane_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID2);
	glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID1);
	// 画静止的方块
	glBindVertexArray(cube_vao);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f));
	shader.SetMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0));
	//model = glm::scale(model, glm::vec3(0.3f));
	//shader.SetMat4("model", model);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	//// 画旋转的方块
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::rotate(model, -current_frame, glm::vec3(0.0f, 0.3f, 0.5f));
	//model = glm::scale(model, glm::vec3(0.3f));
	//shader.SetMat4("model", model);
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0));
	//model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	//model = glm::scale(model, glm::vec3(0.4f));
	//model = glm::rotate(model, current_frame, glm::vec3(1.0f, 0.7f, -0.5f));
	//shader.SetMat4("model", model);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Do_Movement(Target* target)
{
	//不按shift的时候再xy平面进行上下，
	//按下shift后加入z轴的深度
	if (keys[GLFW_KEY_LEFT_SHIFT] && keys[GLFW_KEY_UP])
		target->ProcessTranslation(FORWARD, deltaTime);
	else if (keys[GLFW_KEY_UP])
		target->ProcessTranslation(UP, deltaTime);

	if (keys[GLFW_KEY_LEFT_SHIFT] && keys[GLFW_KEY_DOWN])
		target->ProcessTranslation(BACKWARD, deltaTime);
	else if (keys[GLFW_KEY_DOWN])
		target->ProcessTranslation(DOWN, deltaTime);

	if (keys[GLFW_KEY_LEFT])
		target->ProcessTranslation(LEFT, deltaTime);
	if (keys[GLFW_KEY_RIGHT])
		target->ProcessTranslation(RIGHT, deltaTime);
		
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

	if (flag == true && keys[GLFW_KEY_R])
		flag = false;
	if (flag == false && keys[GLFW_KEY_R])
		flag == true;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		fprintf(stderr, "Click\n");
	}
}
