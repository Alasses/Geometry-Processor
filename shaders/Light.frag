#version 440 core

in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;
in vec3 LightDirection_cameraspace2;
in vec3 EyeDirection_cameraspace;
in vec3 ObjectColor;
in int  IfEdge;

out vec3 color;

uniform vec3 lightColor;

float lightPower = 1;
float distance = 1;

void main()
{

    vec3 n = normalize( Normal_cameraspace );

    vec3 l = normalize( LightDirection_cameraspace );
    vec3 l2 = normalize( LightDirection_cameraspace2 );

    vec3 E = normalize(EyeDirection_cameraspace);

    vec3 R = reflect(-l, n);

    float cosTheta = clamp( dot( n,l ), 0,1 );
    float cosTheta2 = clamp( dot( n,l2 ), 0,1 );
    float cosAlpha = clamp( dot( E,R ), 0,1 );

    vec3 Ambient = 0.5 * ObjectColor;
    vec3 Diffuse = ObjectColor * lightColor * lightPower * cosTheta / distance ;
    vec3 Diffuse2 = ObjectColor * lightColor * lightPower * cosTheta2 / distance ;
    vec3 Specular = ObjectColor * lightColor * lightPower * pow(cosAlpha,5) / distance;

    color = Ambient + Diffuse + Diffuse2 + Specular;

    //if(dot(E, n) < 0.2)
        //color = vec3(0, 0, 0);
    //else
        //color = vec3(1, 1, 1);

    //color = vec3(1, 1, 1);
}
