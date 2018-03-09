#version 430

layout(std430, binding=2) buffer renderedData
{
        uint width;
        uint height;
        uint counts_SSBO[];
};

uniform uint iterationCount;

void addToColorAt(uvec2 cell, uvec3 toAdd)
{
    uint firstIndex = 3*(cell.x + cell.y * width);
    atomicAdd(counts_SSBO[firstIndex],toAdd.x);
    atomicAdd(counts_SSBO[firstIndex+1],toAdd.y);
    atomicAdd(counts_SSBO[firstIndex+2],toAdd.z);
}


layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    if(iterationCount < 1024)
        addToColorAt(uvec2(iterationCount%width,iterationCount/width),uvec3(1,0,0));
    else
        addToColorAt(uvec2(iterationCount%width,iterationCount/width),uvec3(0,1,0));
}
