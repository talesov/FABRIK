#version 330 core
layout (location = 0) in vec3 aPos;   //����
layout (location = 1) in vec3 aNormal;   //N
layout (location = 2) in vec2 aTexCoords;   //��������
layout (location = 3) in vec3 aTangent;     //T

out VS_OUT {               //��ƬԪ����ṹ��
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0f));
    // Normal = mat3(model) * aNormal;
    vs_out.TexCoords = aTexCoords;
	//�������߿ռ������TBN����
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0f)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0f)));
    vec3 B = normalize(cross(T, N));
	
    vs_out.TBN = mat3(T, B, N); //����TBN����
}