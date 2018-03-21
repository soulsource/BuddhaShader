#version 430 core

in vec2 uv;

out vec3 color;

layout(std430, binding=2) restrict readonly buffer renderedDataRed
{
        restrict readonly uint counts_SSBO[];
};
layout(std430, binding=3) restrict readonly buffer brightnessData
{
    restrict readonly uvec3 brightness;
};

uniform uint width;
uniform uint height;

uvec3 getColorAt(vec2 fragCoord)
{
    uint xIndex = uint(max(0.0,(fragCoord.x+1.0)*0.5*width));
    uint yIndex = uint(max(0.0,abs(fragCoord.y)*height));
    uint firstIndex = 3*(xIndex + yIndex * width);
    return uvec3(counts_SSBO[firstIndex],counts_SSBO[firstIndex+1],counts_SSBO[firstIndex+2]);
}

void main(){
    uvec3 totalCount = getColorAt(uv);

    vec3 scaled = vec3(totalCount)/max(length(vec3(brightness)),1.0);
    color = scaled;
}
