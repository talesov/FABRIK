/////////////////////
// ModelObject.h
/////////////////////

#pragma once
#include "Shader.h"
#include "Model.h"
#include "Camera.h"

// Std includes
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
// GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad\glad.h>

class Target {
public:
  glm::vec3 position;
  float pitch;
  float yaw;
  glm::vec3 scale;
  
  // Functions
  Target(int x, int y, int z);
  void Render(glm::mat4 view, glm::mat4 proj);
  void ProcessTranslation(Camera_Movement direction, GLfloat deltaTime);
  
private:
  
  /* Data */
  Model objectModel;
  const GLchar* pathToModel      = "C:\\IK_workspace\\ik_test1\\ik_test1\\ik_test1\\sphere.off";//球体.off
  const GLchar* vertexShaderPath = "C:\\IK_workspace\\ik_test1\\ik_test1\\ik_test1\\shader.vs";//针对球体target的着色器vs
  const GLchar* fragShaderPath   = "C:\\IK_workspace\\ik_test1\\ik_test1\\ik_test1\\shader.fs";
  Shader objectShader;
  
  /* Functions */
  
  
};