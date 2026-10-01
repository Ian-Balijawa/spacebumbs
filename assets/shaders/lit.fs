#version 330

in vec3 fragPosition;
in vec3 fragNormal;

uniform vec4 colDiffuse;
uniform vec3 sunPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 ambientColor;

out vec4 finalColor;

void main()
{
    vec3 n = normalize(fragNormal);
    vec3 l = normalize(sunPos - fragPosition);
    vec3 v = normalize(viewPos - fragPosition);

    float diffuse = max(dot(n, l), 0.0);
    float specular = 0.0;
    if (diffuse > 0.0) {
        vec3 h = normalize(l + v);
        specular = pow(max(dot(n, h), 0.0), 48.0)*0.35;
    }
    float rim = pow(1.0 - max(dot(n, v), 0.0), 3.0)*0.12;

    vec3 color = colDiffuse.rgb*(ambientColor + lightColor*diffuse);
    color += lightColor*specular + vec3(0.30, 0.45, 0.80)*rim;
    finalColor = vec4(color, 1.0);
}
