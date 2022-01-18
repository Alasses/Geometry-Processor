#version 440 core

layout(location = 0) in vec3 vertexPosition;

out vec3 ObjectColor;

uniform mat4 MVP;
uniform mat4 trans;

uniform vec3 objectColor;

void main(void)
{
    gl_Position =  MVP * trans * vec4(vertexPosition,1);

    ObjectColor = objectColor;
}

