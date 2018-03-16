//commented out, added by c-code that loads this shader.
//#version 430
//layout (local_size_x = 1024) in; //to be safe, we limit our local work group size to 1024. That's the minimum a GL 4.3 capable driver must support.

layout(std430, binding=2) buffer renderedDataRed
{
        uint counts_SSBORed[];
};
layout(std430, binding=3) buffer renderedDataGreen
{
        uint counts_SSBOGreen[];
};
layout(std430, binding=4) buffer renderedDataBlue
{
        uint counts_SSBOBlue[];
};

uniform uint width;
uniform uint height;

uniform uint iterationCount;
uniform uvec3 orbitLength;

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

float isInMainCardioid(vec2 v)
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
    return step(rhsSqrt*rhsSqrt,zNormSqr);
}

float isInMainBulb(vec2 v)
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
    return step(sqrRadius,0.062499999);
}

vec2 getStartValue(uint seed, uint yDecoupler)
{
    uint hash = seed;

    vec2 retval = vec2(0);
    for(int i = 0; i < 5; ++i)
    {
        float x = hash1(hash,hash);
        hash = (hash ^ intHash(yDecoupler));
        float y = hash1(hash,hash);
        vec2 random = vec2(x,y);
        vec2 point = vec2(random.x * 3.5-2.5,random.y*1.55);
        float useThisPoint =1.0-(isInMainBulb(point) + isInMainCardioid(point));
        retval = mix(retval,point,useThisPoint);
    }
    return retval;
}

bool isGoingToBeDrawn(vec2 offset)
{
    vec2 val = vec2(0);
    uint limit = orbitLength.x > orbitLength.y ? orbitLength.x : orbitLength.y;
    limit = limit > orbitLength.z ? limit : orbitLength.z;
    for(uint i = 0; i < limit;++i)
    {
        val = compSqr(val) + offset;
        if(dot(val,val) > 4.0)
        {
            return true;
        }
    }
    return false;
}

void drawOrbit(vec2 offset)
{
    vec2 val = vec2(0);
    uint limit = orbitLength.x > orbitLength.y ? orbitLength.x : orbitLength.y;
    limit = limit > orbitLength.z ? limit : orbitLength.z;
    for(uint i = 0; i < limit;++i)
    {
        val = compSqr(val) + offset;
        if(dot(val,val) > 20.0)
        {
            return;
        }
        if(val.x > -2.5 && val.x < 1.0 && val.y > -1.0 && val.y < 1.0)
        {
            addToColorAt(val,uvec3(i < orbitLength.r,i < orbitLength.g,i < orbitLength.b));
        }
    }
}

void main() {
    //we need to know how many total work groups are running this iteration

    uvec3 totalWorkersPerDimension = gl_WorkGroupSize * gl_NumWorkGroups;
    uint totalWorkers = totalWorkersPerDimension.x*totalWorkersPerDimension.y*totalWorkersPerDimension.z;

    uint seed = iterationCount * totalWorkers + gl_GlobalInvocationID.x + gl_GlobalInvocationID.y*totalWorkersPerDimension.x + gl_GlobalInvocationID.z*(totalWorkersPerDimension.x * totalWorkersPerDimension.y);
    uint yDecoupler = iterationCount;
    vec2 offset = getStartValue(seed, yDecoupler);

    if(!isGoingToBeDrawn(offset))
        return;

    drawOrbit(offset);
}
