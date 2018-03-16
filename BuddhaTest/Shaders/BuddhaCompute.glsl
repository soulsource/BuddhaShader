//commented out, added by c-code that loads this shader.
//#version 430
//layout (local_size_x = 1024) in; //to be safe, we limit our local work group size to 1024. That's the minimum a GL 4.3 capable driver must support.

layout(std430, binding=2) restrict buffer renderedDataRed
{
    restrict uint counts_SSBORed[];
};
layout(std430, binding=3) restrict buffer renderedDataGreen
{
    restrict uint counts_SSBOGreen[];
};
layout(std430, binding=4) restrict buffer renderedDataBlue
{
    restrict uint counts_SSBOBlue[];
};
layout(std430, binding=5) restrict buffer statusBuffer
{
    restrict uint individualState[];
};

uniform uint width;
uniform uint height;

uniform uvec3 orbitLength;

uniform uint iterationsPerDispatch;

void getIndividualState(in uint CellID, out vec2 offset, out vec2 coordinates, out uint phase, out uint orbitNumber, out uint doneIterations)
{
    uint startIndex = 7*CellID;
    uint x = individualState[startIndex];
    uint y = individualState[startIndex+1];
    phase = individualState[startIndex+2];
    orbitNumber = individualState[startIndex+3];
    doneIterations = individualState[startIndex+4];
    uint offx =  individualState[startIndex+5];
    uint offy =  individualState[startIndex+6];
    coordinates = vec2(uintBitsToFloat(x),uintBitsToFloat(y));
    offset = vec2(uintBitsToFloat(offx),uintBitsToFloat(offy));
}

void setIndividualState(in uint CellID, in vec2 offset, in vec2 coordinates, in uint phase, in uint orbitNumber, in uint doneIterations)
{
    uint startIndex = 7*CellID;
    uint x=floatBitsToUint(coordinates.x);
    uint y=floatBitsToUint(coordinates.y);
    uint offx = floatBitsToUint(offset.x);
    uint offy = floatBitsToUint(offset.y);
    atomicExchange(individualState[startIndex],x);
    atomicExchange(individualState[startIndex+1],y);
    atomicExchange(individualState[startIndex+2],phase);
    atomicExchange(individualState[startIndex+3],orbitNumber);
    atomicExchange(individualState[startIndex+4],doneIterations);
    atomicExchange(individualState[startIndex+5],offx);
    atomicExchange(individualState[startIndex+6],offy);
}

void addToColorOfCell(uvec2 cell, uvec3 toAdd)
{
    uint firstIndex = (cell.x + cell.y * width);
    atomicAdd(counts_SSBORed[firstIndex],toAdd.x);
    atomicAdd(counts_SSBOGreen[firstIndex],toAdd.y);
    atomicAdd(counts_SSBOBlue[firstIndex],toAdd.z);
}

uvec2 getCell(vec2 complex)
{
    vec2 uv = clamp(vec2((complex.x+2.5)/3.5, (abs(complex.y))),vec2(0.0),vec2(1.0));
    return uvec2(width * uv.x, height * uv.y);
}

void addToColorAt(vec2 complex, uvec3 toAdd)
{
    uvec2 cell = getCell(complex);
    addToColorOfCell(cell,toAdd);
}

uint intHash(uint x) {
    x = ((x >> 16) ^ x) * 0x45d9f3bU;
    x = ((x >> 16) ^ x) * 0x45d9f3bU;
    x = (x >> 16) ^ x;
    return x;
}

float hash1(uint seed, out uint hash)
{
    hash = intHash(seed);
    return float(hash)/float(0xffffffffU);
}

vec2 compSqr(in vec2 v)
{
        return vec2(v.x*v.x-v.y*v.y, 2.0*v.x*v.y);
}

bool isInMainCardioid(vec2 v)
{
    /*
    The condition that a point c is in the main cardioid is that its orbit has
    an attracting fixed point. In other words, it must fulfill
    z**2 -z + v = 0 (z**2+v has a fixed point at z)
    and
    d/dz(z**2+v) < 1 (fixed point at z is attractive)
    Solving these equations yields
    v = u/2*(1-u/2), where u is a complex number inside the unit circle

    Sadly we only know v, not u, and inverting this formula leads to a complex square root.
    This is the old code that uses the compSqrt method, which is rather slow...

    vec2 toRoot = (vec2(1.0,0.0)-vec2(4.0)*v);
    vec2 root = compSqrt(toRoot);
    vec2 t1,t2;
    t1=vec2(1,0)-root;
    t2=vec2(1,0)+root;
    float retval = step(dot(t1,t1),0.99999);
    retval += step(dot(t2,t2),0.99999);
    retval = min(retval,1.0);
    return retval;

    On several websites (and several mandelbrot-related shaders on ShaderToy) one can
    find various faster formulas that check the same inequality.
    What however is hard to find is the actual derivation of those formulas,
    and they are not as trivial as one might think at first.
    That's why I'm writing this lengthy comment, to preserve the scrap notes I made
    for future reuse by myself and others...

    Now the actual derivation looks like this:
    We start with the following line
    u = 1 +- sqrt(1-4*v)
    don't mind the +-, that's just me being too lazy to check which solution is the right one
    We know that |u| < 1, and we immediately square the whole beast to get rid of the root
    Some definitions: r := sqrt(1-4*v), z = 1-4*v

    1 > Re(u)**2+Im(u)**2 = (1 +- Re(r))**2+Im(r)**2
    1 > 1 +- 2*Re(r) + Re(r)**2+Im(r)**2
    1 > 1 +- 2*Re(r) + |r|**2
    For complex values the square root of the norm is the same as the norm of the square root
    1 > 1 +- 2*Re(r) + |z|
    +-2*Re(r) > |z|
    4*Re(r)**2 > |z|**2
    This step is now a bit arcane. If one solves the two coupled equations
    (a+i*b) = (x+i*y)*(x+i*y) component-wise for x and y,
    one can see that x**2 = (|a+i*b|+a)/2
    With this follows
    2*(|z|+Re(z)) > |z|**2
    |z| > 1/2*|z|**2-Re(z)
    |z|**2 > (1/2*|z|**2-Re(z))**2

    And long story short, the result is the following few operations.
    */
    vec2 z = vec2(1.0,0.0)-4.0*v;
    float zNormSqr = dot(z,z);
    float rhsSqrt = 0.5*zNormSqr - z.x;
    return rhsSqrt*rhsSqrt<zNormSqr;
}

bool isInMainBulb(vec2 v)
{
    //The condition for this is that f(f(z)) = z
    //where f(z) = z*z+v
    //This is an equation of fourth order, but two solutions are the same as
    //for f(z) = z, which has been solved for the cardioid above.
    //Factoring those out, you end up with a second order equation.
    //Again, the solutions need to be attractive (|d/dz f(f(z))| < 1)
    //Well, after a bit of magic one finds out it's a circle around -1, radius 1/4.
    vec2 shifted = v + vec2(1,0);
    float sqrRadius = dot(shifted,shifted);
    return sqrRadius<0.062499999;
}

vec2 getStartValue(uint seed, uint yDecoupler)
{
    uint hash = seed;

    for(uint i = 0; i < 5;++i)
    {
        float x = hash1(hash,hash);
        hash = (hash ^ intHash(yDecoupler));
        float y = hash1(hash,hash);
        vec2 random = vec2(x,y);
        vec2 point = vec2(random.x * 3.5-2.5,random.y*1.55);
        if(!(isInMainBulb(point) || isInMainCardioid(point)))
            return point;
    }
    return vec2(0);
}

bool isGoingToBeDrawn(in vec2 offset, in uint totalIterations, inout vec2 lastVal, inout uint iterationsLeftThisFrame, inout uint doneIterations, out bool result)
{
    uint endCount = doneIterations + iterationsLeftThisFrame > totalIterations ? totalIterations : doneIterations + iterationsLeftThisFrame;
    for(uint i = doneIterations; i < endCount;++i)
    {
        lastVal = compSqr(lastVal) + offset;
        if(dot(lastVal,lastVal) > 4.0)
        {
            result = true;
            iterationsLeftThisFrame -= ((i+1)-doneIterations);
            doneIterations = i+1;
            return true;
        }
    }
    iterationsLeftThisFrame -= (endCount - doneIterations);
    doneIterations = endCount;
    result = false;
    return endCount == totalIterations;
}

bool drawOrbit(in vec2 offset, in uint totalIterations, inout vec2 lastVal, inout uint iterationsLeftThisFrame, inout uint doneIterations)
{
    uint endCount = doneIterations + iterationsLeftThisFrame > totalIterations ? totalIterations : doneIterations + iterationsLeftThisFrame;
    for(uint i = doneIterations; i < endCount;++i)
    {
        lastVal = compSqr(lastVal) + offset;
        if(dot(lastVal,lastVal) > 20.0)
        {
            iterationsLeftThisFrame -= ((i+1)-doneIterations);
            doneIterations = i+1;
            return true; //done.
        }
        if(lastVal.x > -2.5 && lastVal.x < 1.0 && lastVal.y > -1.0 && lastVal.y < 1.0)
        {
            addToColorAt(lastVal,uvec3(i < orbitLength.r,i < orbitLength.g,i < orbitLength.b));
        }
    }
    iterationsLeftThisFrame -= (endCount - doneIterations);
    doneIterations = endCount;
    return endCount == totalIterations;
}

void main() {
    //we need to know how many total work groups are running this iteration

    const uvec3 totalWorkersPerDimension = gl_WorkGroupSize * gl_NumWorkGroups;
    const uint totalWorkers = totalWorkersPerDimension.x*totalWorkersPerDimension.y*totalWorkersPerDimension.z;

    const uint uniqueWorkerID = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y*totalWorkersPerDimension.x + gl_GlobalInvocationID.z*(totalWorkersPerDimension.x * totalWorkersPerDimension.y);

    const uint _totalIterations = orbitLength.x > orbitLength.y ? orbitLength.x : orbitLength.y;
    const uint totalIterations = _totalIterations > orbitLength.z ? _totalIterations : orbitLength.z;

    //getIndividualState(in uint CellID, out vec2 coordinates, out uint phase, out uint remainingIterations)
    vec2 lastPosition;
    uint phase;
    uint doneIterations;
    uint orbitNumber;
    vec2 offset;
    //getIndividualState(in uint CellID, out vec2 offset, out vec2 coordinates, out uint phase, out uint orbitNumber, out uint doneIterations)
    getIndividualState(uniqueWorkerID, offset, lastPosition, phase, orbitNumber, doneIterations);
    uint iterationsLeftToDo = iterationsPerDispatch;
    while(iterationsLeftToDo != 0)
    {
        if(phase == 0)
        {
            //new orbit:
            uint seed = orbitNumber * totalWorkers + uniqueWorkerID;
            uint yDecoupler = orbitNumber;
            offset = getStartValue(seed, yDecoupler);
            lastPosition = vec2(0);
            phase = 1;
            doneIterations = 0;
        }
        if(phase == 1)
        {
            //check if this orbit is going to be drawn
            bool result;
            if(isGoingToBeDrawn(offset,totalIterations, lastPosition, iterationsLeftToDo, doneIterations , result))
            {
                if(result)
                {
                    //on to step 2: drawing
                    phase = 2;
                    lastPosition = vec2(0);
                    doneIterations = 0;
                }
                else
                {
                    //back to step 0
                    ++orbitNumber;
                    phase = 0;
                }
            }
        }
        if(phase == 2)
        {
            if(drawOrbit(offset, totalIterations, lastPosition, iterationsLeftToDo, doneIterations))
            {
                ++orbitNumber;
                phase = 0;
            }
        }
    }


    setIndividualState(uniqueWorkerID, offset, lastPosition, phase, orbitNumber, doneIterations);
}
