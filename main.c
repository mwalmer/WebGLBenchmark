#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "star.c"

typedef struct
{
    float r;
    float g;
    float b;
} vec3;

typedef struct
{
    float x;
    float y;
} vec2;

typedef struct
{
    vec2 pos;
    vec3 color;
} iData;

static int width;
static int height;

static int entitiesSize = 100000;
static iData *instanceData;
static vec2 *velocities;
static float spriteSize = 32.0f;

static int numEntities = 0;

#include "jsbindings.c"
#include "mymath.c"

void benchmarkInit()
{
    initCanvas();
    emscripten_get_canvas_element_size("#glCanvas", &width, &height);

    instanceData = (iData*)malloc(sizeof(iData) * entitiesSize);
    velocities = (vec2*)malloc(sizeof(vec2) * entitiesSize);
}

vec2 getRandomPointCircle(float radius)
{
    float r = radius * radius;
    while(1)
    {
        float x = ((float)rand() / (float)RAND_MAX) * 2 - 1.0;
        float y = ((float)rand() / (float)RAND_MAX) * 2 - 1.0;
        float hyp = x * x + y * y;
        if(hyp <= r)
        {
            vec2 result = {x, y};
            return result;
        }
    }
}

void addEntities(int num)
{
    for(int i = 0; i < num; i++)
    {
        if(numEntities >= entitiesSize)
            return;
        
        vec2 pos = {mouseX - 16.0f, mouseY - 16.0f};
        instanceData[numEntities].pos = pos;
        instanceData[numEntities].color = color;

        vec2 vel = getRandomPointCircle(1);
        velocities[numEntities] = vel;
        
        numEntities++;
    }
}

void update(float dt)
{
    if(mouseDownFlag)
    {
        addEntities(300);
        if(numEntities >= entitiesSize)
        {
            int newSize = entitiesSize * 2;
            iData* newInstanceData = (iData*)malloc(sizeof(iData) * newSize);
            vec2* newVelocities = (vec2*)malloc(sizeof(vec2) * newSize);

            memcpy(newInstanceData, instanceData, sizeof(iData) * entitiesSize);
            memcpy(newVelocities, velocities, sizeof(vec2) * entitiesSize);

            free(instanceData);
            free(velocities);

            instanceData = newInstanceData;
            velocities = newVelocities;
            entitiesSize = newSize;

            glEnableVertexAttribArray(2);
            glBufferData(GL_ARRAY_BUFFER, sizeof(iData) * entitiesSize, instanceData, GL_DYNAMIC_DRAW);
        }
    }

    for(int i = 0; i < numEntities; i++)
    {
        float speed = 144;
        instanceData[i].pos.x += velocities[i].x * dt * speed;
        if(instanceData[i].pos.x <= 0)
        {
            velocities[i].x *= -1;
            instanceData[i].pos.x = 0;
        }
        else if(instanceData[i].pos.x >= width - spriteSize)
        {
            velocities[i].x *= -1;
            instanceData[i].pos.x = width - spriteSize;
        }
        
        instanceData[i].pos.y += velocities[i].y * dt * speed;
        if(instanceData[i].pos.y <= 0)
        {
            velocities[i].y *= -1;
            instanceData[i].pos.y = 0;
        }
        else if(instanceData[i].pos.y >= height - spriteSize)
        {
            velocities[i].y *= -1;
            instanceData[i].pos.y = height - spriteSize;
        }
    }
}

void render()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    // update instance buffer data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(iData) * numEntities, instanceData);
    
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numEntities);
    emscripten_webgl_commit_frame();
}

static double currentTime = 0;
EM_BOOL GameLoop(double newTime, void* userData)
{
    static int fps = 0;
    static int lastFps = 0;
    static int lastEnCount = 0;
    static double clock = 0;

    double frameTime = newTime - currentTime;
    currentTime = newTime;

    if(lastEnCount != numEntities)
    {
        updateMetrics(lastFps, numEntities);
        lastEnCount = numEntities;
    }

    if((currentTime - clock) / 1000 >= 1)
    {
        updateMetrics(fps, numEntities);
        lastFps = fps;
        fps = 0;
        clock = currentTime;
    }


    update(frameTime / 1000);
    render();

    fps++;

    return EM_TRUE;
}

int main()
{
    // allocates memory
    benchmarkInit();

    EmscriptenWebGLContextAttributes contextAttribute;
    emscripten_webgl_init_context_attributes(&contextAttribute);
    contextAttribute.antialias = 0;
    // contextAttribute.premultipliedAlpha = 0;
    contextAttribute.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
    contextAttribute.majorVersion = 2;
    contextAttribute.minorVersion = 0;
    contextAttribute.enableExtensionsByDefault = 0;
    contextAttribute.explicitSwapControl = 1; // need specific linker flags
    contextAttribute.renderViaOffscreenBackBuffer = 1;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#glCanvas", &contextAttribute);
    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(context);
    if(result != EMSCRIPTEN_RESULT_SUCCESS)
        printError("Context creation failed!");

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const char *vsSource = 
        "attribute vec2 vPos;\n"
        "attribute vec2 vTexCoord;\n"
        "attribute vec2 iOffset;\n"
        "attribute vec3 iColor;\n"
        "uniform mat4 projection;\n"
        "varying vec2 fTexCoord;\n"
        "varying vec3 fColor;\n"
        "void main() {\n"
        "fTexCoord = vTexCoord;\n"
        "fColor = iColor;\n"
        "gl_Position = vec4(vPos + iOffset, 0, 1) * projection;\n"
        "}";
    glShaderSource(vs, 1, &vsSource, 0);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fsSource = 
        "precision lowp float;\n"
        "varying vec2 fTexCoord;\n"
        "varying vec3 fColor;\n"
        "uniform sampler2D texSample;\n"
        "void main() {\n"
        "vec4 sampledTex = texture2D(texSample, fTexCoord);\n"
        "if(all(lessThanEqual(sampledTex.xyz, vec3(0.1)))){gl_FragColor = sampledTex;}\n"
        "else{gl_FragColor = sampledTex + vec4(fColor.xyz, sampledTex.w);}\n"
        "}";

    glShaderSource(fs, 1, &fsSource, 0);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glBindAttribLocation(program, 0, "vPos");
    glBindAttribLocation(program, 1, "vTexCoord");
    glBindAttribLocation(program, 2, "iOffset");
    glBindAttribLocation(program, 3, "iColor");
    glLinkProgram(program);
    glUseProgram(program);

    GLuint vertexArray, vertexBuffer, elementBuffer, instanceBuffer, texture;
    glGenVertexArrays(1, &vertexArray);

    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &elementBuffer);
    glGenBuffers(1, &instanceBuffer);
    glGenTextures(1, &texture);
    
    float size = spriteSize;
    float vertices[] = {
        /*  POS          TEX  */
        0.0f, size,   0.0f, 1.0f,
        size, size,   1.0f, 1.0f,
        size, 0.0f,   1.0f, 0.0f,
        0.0f, 0.0f,   0.0f, 0.0f,
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3 
    };

    memset(instanceData, 0, sizeof(iData) * entitiesSize);

    glBindVertexArray(vertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(iData) * entitiesSize, instanceData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glVertexAttribDivisor(3, 1);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, &imageData[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(program, "sampleTex"), 0);

    float projection[16];
    setOrthographicOffCenterRH(projection, width, height);

    GLint id = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(id, 1, GL_FALSE, projection);

    // adding event listeners
    EMSCRIPTEN_RESULT ret;
    ret = emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mousedown_callback);
    if(ret != EMSCRIPTEN_RESULT_SUCCESS)
        printError("mouse down callback failed!");

    ret = emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseup_callback);
    if(ret != EMSCRIPTEN_RESULT_SUCCESS)
        printError("mouse up callback failed!");

    ret = emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mousemove_callback);
    if(ret != EMSCRIPTEN_RESULT_SUCCESS)
        printError("mouse move callback failed!");

    ret = emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, resize_callback);
    if(ret != EMSCRIPTEN_RESULT_SUCCESS)
        printError("resize callback failed!");

    ret = emscripten_set_touchstart_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touchstart_callback);
    if(ret != EMSCRIPTEN_RESULT_SUCCESS)
        printError("touch start callback failed!");

    ret = emscripten_set_touchend_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touchend_callback);
    if(ret != EMSCRIPTEN_RESULT_SUCCESS)
        printError("touch end callback failed!");

    ret = emscripten_set_touchmove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touchmove_callback);
    if(ret != EMSCRIPTEN_RESULT_SUCCESS)
        printError("touch move callback failed!");


    srand(0);
    
    // set gameloop callback
    emscripten_request_animation_frame_loop(GameLoop, 0);

    return 0;
}