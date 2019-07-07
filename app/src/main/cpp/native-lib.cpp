//#include <jni.h>
//#include <dlfcn.h>
//#include <GLES2/gl2.h>
//
//#include <android/log.h>
//#include <android/sensor.h>
//#include <android/asset_manager_jni.h>
//
//#include <cstdint>
//#include <cassert>
//#include <string>
//#define TAG "System->C++"
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
//
//const int LOOPER_ID_USED = 3;
//const int SENSOR_HISTORY_LENGTH = 100;
//const int SENSOR_REFRESH_RATE_HZ = 100;
//constexpr int32_t SENSOR_REFRESH_PERIOD_US = int32_t (1000 / SENSOR_REFRESH_RATE_HZ);
//const float SENSOR_FILTER_ALPHA = 0.1f;
//
//const char* kPackageName = "com.example.sensor";
//
//ASensorManager *ASensorManagerInstance(){
//  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
//  void* androidHandle =dlopen("libandroid.so",RTLD_NOW);
//    PF_GETINSTANCEFORPACKAGE getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
//          dlsym(androidHandle,"ASensorManager_getInstanceForPackage");
//  if (getInstanceForPackageFunc){
//      return getInstanceForPackageFunc(kPackageName);
//  }
//  typedef ASensorManager* (*PF_GETINSTANCE)();
//  PF_GETINSTANCE getInstance = (PF_GETINSTANCE)dlsym(androidHandle,"ASensorManager_getInstance");
//  assert(getInstance);
//  return getInstance();
//}
//class sensorgraph{
//    std::string vertexShaderSource;
//    std::string fragShaderSource;
//    ASensorManager *sensorManager;
//    const ASensor *accelerometer;
//    ASensorEventQueue *sensorEventQueue;
//    ALooper *looper;
//
//    GLuint shaderProgram;
//    GLuint vPosition;
//    GLuint vSensorValue;
//    GLuint uFragColor;
//    GLfloat xPos[SENSOR_HISTORY_LENGTH];
//
//    struct AccelerometerDate{
//      GLfloat x;
//      GLfloat y;
//      GLfloat z;
//    };
//    AccelerometerDate sensorData[SENSOR_HISTORY_LENGTH*2];
//    AccelerometerDate sensorDataFilter;
//    int sensorDataIndex;
//
//public:
//     sensorgraph() : sensorDataIndex(0){}
//     void init(AAssetManager *assetManager){
//     AAsset *vertexShaderAsset= AAssetManager_open(assetManager,"shader.glslv",AASSET_MODE_BUFFER);
//     assert(vertexShaderAsset != NULL);
//     const void *vertexShaderBuf = AAsset_getBuffer(vertexShaderAsset);
//     assert(vertexShaderBuf != NULL);
//     off_t vertexShaderLength = AAsset_getLength(vertexShaderAsset);
//     vertexShaderSource = std::string((const char*)vertexShaderBuf,(size_t)vertexShaderLength);
//     AAsset_close(vertexShaderAsset);
//
//     AAsset *fragShaderAssets = AAssetManager_open(assetManager,"shader.glslf",AASSET_MODE_BUFFER);
//     assert(fragShaderAssets != NULL);
//     const void *fragShaderBuf = AAsset_getBuffer(fragShaderAssets);
//     assert(fragShaderBuf != NULL);
//     off_t fragShaderLength = AAsset_getLength(fragShaderAssets);
//     fragShaderSource = std::string((const char*)fragShaderBuf,(size_t)fragShaderLength);
//     AAsset_close(fragShaderAssets);
//
//     sensorManager = ASensorManagerInstance();
//     assert(sensorManager != NULL);
//     accelerometer = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
//     assert(accelerometer != NULL);
//     looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
//     assert(looper != NULL);
//     sensorEventQueue = ASensorManager_createEventQueue(sensorManager,looper,LOOPER_ID_USED,NULL,NULL);
//     assert(sensorEventQueue != NULL);
//     auto status = ASensorEventQueue_enableSensor(sensorEventQueue,accelerometer);
//     assert(status >= 0);
//     status = ASensorEventQueue_setEventRate(sensorEventQueue,accelerometer,SENSOR_REFRESH_PERIOD_US);
//     assert(status >= 0);
//     (void) status;
//     generateXPos();
//    }
//    void surfaceChange(int w,int h){
//         glViewport(0,0,w,h);
//     }
//     void surfaceCreate(){
//        shaderProgram = createProgram(vertexShaderSource,fragShaderSource);
//        GLint getPositionLocalResult = glGetAttribLocation(shaderProgram,"vPosition");
//        assert(getPositionLocalResult != -1);
//        vPosition = (GLuint)getPositionLocalResult;
//        GLint getSensorLocalResult = glGetAttribLocation(shaderProgram,"vSensorValue");
//        assert(getSensorLocalResult != -1);
//        vSensorValue = (GLuint)getSensorLocalResult;
//         GLint getFragColorLocationResult = glGetUniformLocation(shaderProgram, "uFragColor");
//         assert(getFragColorLocationResult != -1);
//         uFragColor = (GLuint)getFragColorLocationResult;
//     }
//    GLuint createProgram(const std::string pVertexSource, const std::string pFragSource){
//         GLuint vertexShader = loadShader(GL_VERTEX_SHADER,pVertexSource);
//         GLuint fragShader = loadShader(GL_FRAGMENT_SHADER,pFragSource);
//         GLuint program = glCreateProgram();
//         assert(program != 0);
//         glAttachShader(program,vertexShader);
//         glAttachShader(program,fragShader);
//         glLinkProgram(program);
//         int linked = 0;
//         glGetProgramiv(program,GL_LINK_STATUS,&linked);
//         assert(linked != 0);
//         glDeleteShader(vertexShader);
//         glDeleteShader(fragShader);
//         return program;
//     }
//     GLuint loadShader(GLenum type,std::string pSource){
//        GLuint shader = glCreateShader(type);
//        assert(shader != 0);
//        const char *sourceBuf = pSource.c_str();
//        glShaderSource(shader,1,&sourceBuf,NULL);
//        glCompileShader(shader);
//        GLint compileShader = 0;
//        glGetShaderiv(shader,GL_COMPILE_STATUS,&compileShader);
//        assert(compileShader != 0);
//        return shader;
//     }
//    void generateXPos(){
//        for (auto i = 0; i <SENSOR_HISTORY_LENGTH ; ++i) {
//            float t = static_cast<float>(i)/ static_cast<float> (SENSOR_HISTORY_LENGTH-1);
//            xPos[i] = (-1.f)*(1.f-t)+1.f*t;
//        }
//     }
//     void update(){
//       ALooper_pollAll(0,NULL,NULL,NULL);
//       ASensorEvent event;
//       while (ASensorEventQueue_getEvents(sensorEventQueue,&event,1)>0){
//           sensorDataFilter.x = SENSOR_FILTER_ALPHA * event.acceleration.x;
//           sensorDataFilter.y = SENSOR_FILTER_ALPHA * event.acceleration.y;
//           sensorDataFilter.z = SENSOR_FILTER_ALPHA * event.acceleration.z;
//       }
//       sensorData[sensorDataIndex] = sensorDataFilter;
//       sensorData[SENSOR_HISTORY_LENGTH+sensorDataIndex] = sensorDataFilter;
//       sensorDataIndex = (sensorDataIndex+1)%SENSOR_HISTORY_LENGTH;
//     }
//     void render(){
//       glClearColor(0.f,0.f,0.f,0.f);
//       glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//       glUseProgram(shaderProgram);
//       glEnableVertexAttribArray(vPosition);
//       glVertexAttribPointer(vPosition,1,GL_FLOAT,GL_FALSE,0,xPos);
//
//       glEnableVertexAttribArray(vSensorValue);
//       glVertexAttribPointer(vSensorValue,1,GL_FLOAT,GL_FALSE, sizeof(AccelerometerDate),&sensorData[sensorDataIndex].x);
//       glUniform4f(uFragColor,1.0f,1.0f,0.0f,1.0f);
//       glDrawArrays(GL_LINE_STRIP,0,SENSOR_HISTORY_LENGTH);
//       glVertexAttribPointer(vSensorValue,1,GL_FLOAT,GL_FALSE, sizeof(AccelerometerDate),&sensorData[sensorDataIndex].y);
//       glUniform4f(uFragColor,1.0f,0.0f,1.0f,1.0f);
//       glDrawArrays(GL_LINE_STRIP,0,SENSOR_HISTORY_LENGTH);
//       glVertexAttribPointer(vSensorValue,1,GL_FLOAT,GL_FALSE, sizeof(AccelerometerDate),&sensorData[sensorDataIndex].z);
//       glUniform4f(uFragColor,0.0f,1.0f,1.0f,1.0f);
//       glDrawArrays(GL_LINE_STRIP,0,SENSOR_HISTORY_LENGTH);
//     }
//     void onPause(){
//       ASensorEventQueue_disableSensor(sensorEventQueue,accelerometer);
//     }
//     void onResume(){
//      ASensorEventQueue_enableSensor(sensorEventQueue,accelerometer);
//      int status = ASensorEventQueue_setEventRate(sensorEventQueue,accelerometer,SENSOR_REFRESH_PERIOD_US);
//      assert(status >= 0);
//     }
//};
//sensorgraph sensorgraph;
//extern "C"{
//JNIEXPORT void JNICALL
//Java_com_example_sensor_Jni_surfaceCreate(JNIEnv *env,jclass type){
//    sensorgraph.surfaceCreate();
//}
//JNIEXPORT void JNICALL
//Java_com_example_sensor_Jni_init(JNIEnv *env,jclass type,jobject assetManager){
//    AAssetManager *nativeAAssetManager = AAssetManager_fromJava(env,assetManager);
//    sensorgraph.init(nativeAAssetManager);
//}
//JNIEXPORT void JNICALL
//Java_com_example_sensor_Jni_change(JNIEnv *env,jclass type,jint w,jint h){
//    sensorgraph.surfaceChange(w,h);
//}
//JNIEXPORT void JNICALL
//Java_com_example_sensor_Jni_update(JNIEnv *env,jclass type){
//    sensorgraph.update();
//    sensorgraph.render();
//}
//JNIEXPORT void JNICALL
//Java_com_example_sensor_Jni_pause(JNIEnv *env,jclass type){
//sensorgraph.onPause();
//}
//JNIEXPORT void JNICALL
//Java_com_example_sensor_Jni_resume(JNIEnv *env,jclass type){
//sensorgraph.onResume();
//}
//}
