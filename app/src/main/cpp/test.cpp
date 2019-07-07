//
// Created by ASUS on 2019/7/6.
//

#include <jni.h>
#include <string>
#include <dlfcn.h>
#include <GLES2/gl2.h>
#include <cassert>
#include <cstdint>
#include <android/log.h>
#include <android/asset_manager_jni.h>
#include <android/sensor.h>

#define TAG "System---->C++"
#define LOGD(...)__android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

const int SENSOR_HISTORY_LENGTH = 100;
const int32_t SENSOR_REFRESH_PERIOD_HZ = (int32_t)(1000000/SENSOR_HISTORY_LENGTH);
const int LOOPER_USED_ID = 3;
const char *kPackageName = "com.example.sensor";

ASensorManager *ASensorManageInstance(void){
    typedef ASensorManager* (*GETINSTANCEFORPACKAGE)(const char *name);
    void *androidHandle = dlopen("libandroid.so",RTLD_NOW);
    GETINSTANCEFORPACKAGE getInstanceForPackage =
            (GETINSTANCEFORPACKAGE)dlsym(androidHandle,"ASensorManager_getInstanceForPackage");
    if (getInstanceForPackage){
        return getInstanceForPackage(kPackageName);
    }
    typedef ASensorManager* (*GETINSTANCE)();
    GETINSTANCE getInstance = (GETINSTANCE)dlsym(androidHandle,"ASensorManager_getInstance");
    assert(getInstance);
    return getInstance();
}
class sensorGraph{
    std::string vertexShaderSource;
    std::string fragShaderSource;
    ASensorManager *sensorManager;
    const ASensor *sensor;
    ALooper *looper;
    ASensorEventQueue *eventQueue;

    GLuint position;
    GLuint sensorValue;
    GLuint fragColor;
    GLuint program;
    GLfloat xPos[SENSOR_HISTORY_LENGTH];

    struct AccelerateData{
        GLfloat x;
        GLfloat y;
        GLfloat z;
    };
   AccelerateData sensorData[SENSOR_HISTORY_LENGTH*2];
   AccelerateData sensorFilter;
   int sensorDateIndex;
public:
    sensorGraph() : sensorDateIndex(0){}
    void init(AAssetManager *assetManager){
        AAsset *vertexShaderAsset = AAssetManager_open(assetManager,"shader.glslv",AASSET_MODE_BUFFER);
        assert(vertexShaderAsset != NULL);
        const void *vertexShaderBuf = AAsset_getBuffer(vertexShaderAsset);
        assert(vertexShaderAsset != NULL);
        off_t vertexShaderLength =AAsset_getLength(vertexShaderAsset);
        vertexShaderSource = std::string((const char*)vertexShaderBuf,(size_t)vertexShaderLength);
        AAsset_close(vertexShaderAsset);

        AAsset *fragShaderAsset = AAssetManager_open(assetManager,"shader.glslf",RTLD_NOW);
        assert(fragShaderAsset != NULL);
        const void *fragShaderBuf = AAsset_getBuffer(fragShaderAsset);
        assert(fragShaderAsset != NULL);
        off_t fragShaderLength = AAsset_getLength(fragShaderAsset);
        fragShaderSource = std::string((const char*)fragShaderBuf,(size_t)fragShaderLength);
        AAsset_close(fragShaderAsset);

        sensorManager = ASensorManageInstance();
        assert(sensorManager != NULL);
        sensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
        assert(sensor != NULL);
        looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        assert(looper != NULL);
        eventQueue = ASensorManager_createEventQueue(sensorManager,looper,LOOPER_USED_ID,NULL,NULL);
        assert(eventQueue != NULL);
        int status = ASensorEventQueue_enableSensor(eventQueue,sensor);
        assert(status >= 0);
        status = ASensorEventQueue_setEventRate(eventQueue,sensor,SENSOR_REFRESH_PERIOD_HZ);
        assert(status >= 0);
        (void)status;
        generateGraph();
    }
    void generateGraph(){
        for (auto i = 0; i <SENSOR_HISTORY_LENGTH ; ++i) {
            float t = static_cast<float >(i)/ static_cast<float>(SENSOR_HISTORY_LENGTH-1);
            xPos[i] = (-1.0f)*(1.0f-t)+1.0f*t;
        }
    }
    void surfaceChange(int w, int h){
        glViewport(0,0,w,h);
    }
    void surfaceCreate(){
        program = createProgram(vertexShaderSource,fragShaderSource);

        GLint getLocalPositionResult = glGetAttribLocation(program,"vPosition");
        assert(getLocalPositionResult != -1);
        position = (GLuint)getLocalPositionResult;

        GLint getLocalSensorValueResult = glGetAttribLocation(program,"vSensorValue");
        assert(getLocalSensorValueResult != -1);
        sensorValue =(GLuint) getLocalSensorValueResult;

        GLint getLocalFragColorResult = glGetUniformLocation(program,"uFragColor");
        assert(getLocalFragColorResult != -1);
        fragColor =(GLuint) getLocalFragColorResult;

    }

    void update(){
        ALooper_pollAll(0,NULL,NULL,NULL);
        ASensorEvent event;
        while (ASensorEventQueue_getEvents(eventQueue,&event,1) >0){
            sensorFilter.x = event.acceleration.x *0.1f;
            sensorFilter.y = event.acceleration.y *0.1f;
            sensorFilter.z = event.acceleration.z *0.1f;
        }
        sensorData[sensorDateIndex] = sensorFilter;
        sensorData[SENSOR_HISTORY_LENGTH + sensorDateIndex] = sensorFilter;
        sensorDateIndex = (sensorDateIndex + 1)% SENSOR_HISTORY_LENGTH;
    }
    void render(){
        glClearColor(0.f,0.f,0.f,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glUseProgram(program);

        glEnableVertexAttribArray(position);
        glVertexAttribPointer(position,1,GL_FLOAT,GL_FALSE,0,xPos);

        glEnableVertexAttribArray(sensorValue);
        glVertexAttribPointer(sensorValue,1,GL_FLOAT,GL_FALSE, sizeof(AccelerateData),&sensorData[sensorDateIndex].x);
        glUniform4f(fragColor,1.f,0.f,1.f,1.f);
        glDrawArrays(GL_LINE_STRIP,0,SENSOR_HISTORY_LENGTH);

        glVertexAttribPointer(sensorValue,1,GL_FLOAT,GL_FALSE, sizeof(AccelerateData),&sensorData[sensorDateIndex].z);
        glUniform4f(fragColor,0.f,1.f,1.f,1.f);
        glDrawArrays(GL_LINE_STRIP,0,SENSOR_HISTORY_LENGTH);

        glVertexAttribPointer(sensorValue,1,GL_FLOAT,GL_FALSE, sizeof(AccelerateData),&sensorData[sensorDateIndex].y);
        glUniform4f(fragColor,1.f,1.f,0.f,1.f);
        glDrawArrays(GL_LINE_STRIP,0,SENSOR_HISTORY_LENGTH);

    }
    GLuint createProgram(const std::string& vs, const std::string& fs){
        GLuint vShader = createShader(GL_VERTEX_SHADER,vs);
        GLuint fShader = createShader(GL_FRAGMENT_SHADER,fs);
        GLuint program = glCreateProgram();
        assert(program != 0);
        glAttachShader(program,vShader);
        glAttachShader(program,fShader);
        glLinkProgram(program);
        int linkStatus = 0;
        glGetProgramiv(program,GL_LINK_STATUS,&linkStatus);
        assert(linkStatus != 0);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        return program;
    }
    GLuint createShader(GLenum type,const std::string& s){
       GLuint shader = glCreateShader(type);
       assert(shader != 0);
       const char *sBuf = s.c_str();
       glShaderSource(shader,1,&sBuf,NULL);
       glCompileShader(shader);
       int compileStatus = 0;
       glGetShaderiv(shader,GL_COMPILE_STATUS,&compileStatus);
       assert(compileStatus != 0);
       return shader;
    }

    void pause(){
        ASensorEventQueue_disableSensor(eventQueue,sensor);
    }
    void resume(){
        int status = ASensorEventQueue_enableSensor(eventQueue,sensor);
        assert(status >= 0);
        status = ASensorEventQueue_setEventRate(eventQueue,sensor,SENSOR_REFRESH_PERIOD_HZ);
        assert(status >= 0);
        (void)status;
    }
};
sensorGraph graph;
extern "C"{
JNIEXPORT void JNICALL
Java_com_example_sensor_Jni_pause(JNIEnv *env, jclass type) {
    graph.pause();
}
JNIEXPORT void JNICALL
Java_com_example_sensor_Jni_resume(JNIEnv *env, jclass type) {
    graph.resume();
}
JNIEXPORT void JNICALL
Java_com_example_sensor_Jni_update(JNIEnv *env, jclass type) {
    graph.update();
    graph.render();
}

JNIEXPORT void JNICALL
Java_com_example_sensor_Jni_init(JNIEnv *env, jclass type,jobject manager) {
   AAssetManager *assetManager = AAssetManager_fromJava(env,manager);
   graph.init(assetManager);
}

JNIEXPORT void JNICALL
Java_com_example_sensor_Jni_change(JNIEnv *env, jclass type, jint w, jint h) {
    graph.surfaceChange(w,h);
}

JNIEXPORT void JNICALL
Java_com_example_sensor_Jni_surfaceCreate(JNIEnv *env, jclass type) {
    graph.surfaceCreate();
}
}
