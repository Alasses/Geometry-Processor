#version 440 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

out vec3 Normal_cameraspace;
out vec3 LightDirection_cameraspace;
out vec3 LightDirection_cameraspace2;
out vec3 EyeDirection_cameraspace;
out vec3 ObjectColor;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform mat4 trans;

uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform vec3 objectColor;

uniform int ifCheckBoard;

void main(void)
{
    gl_Position =  MVP * trans * vec4(vertexPosition,1);

    vec4 Pw = (M * trans * vec4(vertexPosition,1));
    vec3 Position_worldspace = Pw.xyz;

    vec4 Vc = ( V * M * trans * vec4(vertexPosition,1));
    vec3 vertexPosition_cameraspace = Vc.xyz;
    EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

    vec4 Lc = ( V * vec4(lightPos,1));
    vec4 Lc2 = ( V * vec4(lightPos2,1));
    vec3 LightPosition_cameraspace = Lc.xyz;
    vec3 LightPosition_cameraspace2 = Lc2.xyz;
    LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
    LightDirection_cameraspace2 = LightPosition_cameraspace2 + EyeDirection_cameraspace;

    vec4 Nc = (V * M * trans * vec4(vertexNormal ,0));
    Normal_cameraspace = Nc.xyz;

    if(ifCheckBoard == 1)
        ObjectColor = vertexColor;
    else
        ObjectColor = objectColor;
}
