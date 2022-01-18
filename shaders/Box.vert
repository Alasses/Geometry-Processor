#version 440 core

layout(location = 0) in vec3 vertexPosition_modelspace;
//layout(location = 1) in vec3 vertexColor;

//out vec3 fragColor;

uniform mat4 MVP;
uniform mat4 trans;

void main(void)
{
    gl_Position =  MVP * trans * vec4(vertexPosition_modelspace,1);

    //fragColor = vertexColor;
}
