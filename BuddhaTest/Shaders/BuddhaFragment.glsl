#version 430 core

in vec2 uv;

out vec3 color;

layout(std430, binding=2) restrict readonly buffer renderedDataRed
{
        restrict readonly uint counts_SSBORed[];
};
layout(std430, binding=3) restrict readonly buffer renderedDataGreen
{
        restrict readonly uint counts_SSBOGreen[];
};
layout(std430, binding=4) restrict readonly buffer renderedDataBlue
{
        restrict readonly uint counts_SSBOBlue[];
};

uniform uint width;
uniform uint height;

uvec3 getColorAt(vec2 fragCoord)
{
    uint xIndex = uint(max(0.0,(fragCoord.x+1.0)*0.5*width));
    uint yIndex = uint(max(0.0,abs(fragCoord.y)*height));
    uint firstIndex = (xIndex + yIndex * width);
    return uvec3(counts_SSBORed[firstIndex],counts_SSBOGreen[firstIndex],counts_SSBOBlue[firstIndex]);
}

void main(){
    uvec3 totalCount = getColorAt(uv);
    uvec3 brightness = getColorAt(vec2(-0.2390625,0));

    vec3 scaled = vec3(totalCount)/max(length(vec3(brightness)),1.0);
    color = scaled;
}
