#version 430 core

in vec2 fragTexCoord;
in float textureLayer;
in vec4 fragColor;

in float temperature;
in float humidity;

in float behaviourFlag;

uniform sampler2DArray texture0;
uniform sampler2D grassColormap;

out vec4 finalColor;

void main() {
    vec4 tintColor;
    float adjustedHumid = humidity * temperature;
    float u = 1.0 - temperature;
    float v = 1.0 - adjustedHumid;

    tintColor = texture(grassColormap, vec2(u, v));


    vec4 texColor = texture(texture0, vec3(fragTexCoord.xy, textureLayer));
    if(behaviourFlag == 1){
        texColor *= tintColor;
    }
    else if (behaviourFlag == 2){
        vec4 overlayColor = texture(texture0, vec3(fragTexCoord.xy, 0.0));
        overlayColor.rgb *= tintColor.rgb;
        texColor = mix(texColor, overlayColor, overlayColor.a);
        texColor.a = 1.0;
    }
    finalColor = texColor * fragColor;
}