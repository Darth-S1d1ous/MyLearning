#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <pthread.h>
#include <opencv2/opencv.hpp>
#include <sys/types.h>
#include <thread>
#include <utility>
#include <yaml-cpp/yaml.h>
#include "libobsensor/ObSensor.hpp"
#include "window.hpp"

using namespace std::chrono_literals;

static bool  SYNC         = true;
static bool  started      = true;
static bool  hd2c         = false;
static bool  sd2c         = true;
static float alpha        = 0.5;


namespace camera
{
    struct ThreadArgs {
        void *handle;
        cv::Mat frame;
        cv::Mat colorMat;
        cv::Mat depthMat;
        std::vector<cv::Mat*> images;
        bool frame_empty;
        std::mutex mutex;

        std::shared_ptr<Window> app = nullptr;

        ThreadArgs() : images{&colorMat, &depthMat} {}
    };
//********** define ************************************/
#define MAX_IMAGE_DATA_SIZE (4 * 3270 * 2048)
    //********** frame ************************************/
    //cv::Mat frame;
    //********** frame_empty ******************************/
    //bool frame_empty = 0;
    //********** mutex ************************************/
    //pthread_mutex_t mutex;
    //********** CameraProperties config ************************************/
    enum CamerProperties
    {
        CAP_PROP_FRAMERATE_ENABLE,  //帧数可调
        CAP_PROP_FRAMERATE,         //帧数
        CAP_PROP_BURSTFRAMECOUNT,   //外部一次触发帧数
        CAP_PROP_HEIGHT,            //图像高度
        CAP_PROP_WIDTH,             //图像宽度
        CAP_PROP_EXPOSURE_TIME,     //曝光时间
        CAP_PROP_GAMMA_ENABLE,      //伽马因子可调
        CAP_PROP_GAMMA,             //伽马因子
        CAP_PROP_GAINAUTO,          //亮度
        CAP_PROP_SATURATION_ENABLE, //饱和度可调
        CAP_PROP_SATURATION,        //饱和度
        CAP_PROP_OFFSETX,           //X偏置
        CAP_PROP_OFFSETY,           //Y偏置
        CAP_PROP_TRIGGER_MODE,      //外部触发
        CAP_PROP_TRIGGER_SOURCE,    //触发源
        CAP_PROP_LINE_SELECTOR      //触发线

    };

    enum E_TriggerMode{
        TriggerMode_Off = 0,
        TriggerMode_On = 1
    };
    
    enum E_TiggerSource{
        TriggerSource_Line0 = 0,
        TriggerSource_Line1 = 1,
        TriggerSource_Line2 = 2,
        TriggerSource_Line3 = 3,
        TriggerSource_Counter0 = 4,
        TriggerSource_Software = 7,
        TriggerSource_FrequencyConverter = 8
    };

    //^ *********************************************************************************** //
    //^ ********************************** Camera Class************************************ //
    //^ *********************************************************************************** //
    class Camera
    {
    public:
        Camera(int ind, std::string yamlName);
        ~Camera();
        //********** 原始信息转换线程 **********************/
        void *HKWorkThread(void *p_handle);

        //********** 输出摄像头信息 ***********************/
        bool PrintDeviceInfo(ob::DeviceInfo &deviceInfo);

        int getHeight();
        int getWidth();
        bool windowOK();

        //********** 设置摄像头参数 ***********************/
        int readConfig();
        //********** 恢复默认参数 *************************/
        bool reset();
        //********** 读图10个相机的原始图像 ********************************/
        void ReadImg(std::vector<cv::Mat> &imagess);
        void TriggerCapture(std::vector<cv::Mat> &image);
        std::future<void> asyncGetImage(std::vector<cv::Mat> &image);

    private:
        bool keyEventProcess(Window &app, ob::Pipeline &pipe, std::shared_ptr<ob::Config> config);

        ThreadArgs* args;
        std::unique_ptr<std::thread> workthread;
        //********** frame ************************************/
        //cv::Mat frame;
        //********** frame_empty ******************************/
        //bool frame_empty = 0;
        //********** mutex ************************************/
        //pthread_mutex_t mutex;    
        //********** handle ******************************/
        //void *handle;
        
        std::shared_ptr<ob::Device> device;
        int width = 1280, height = 960, fps = 30;
        int Offset_x;
        int Offset_y;
        bool FrameRateEnable;
        
        static int frameCount, lastFrameCount, saveCount, saveCountIMU;
        std::filesystem::path currentPath, rootPath, configPath, savePath;
        std::shared_ptr<ob::Config> obConfig;
        ob::Pipeline pipeline;
        ob::FormatConvertFilter formatConvertFilter;

        int TriggerMode = TriggerMode_Off;

    };
    //^ *********************************************************************************** //

    int Camera::getHeight(){
        return height;
    }

    int Camera::getWidth(){
        return width;
    }

    bool Camera::windowOK(){
        if (args->app == nullptr)
            return false;
        return true;
    }

    //^ ********************************** Camera constructor************************************ //
    Camera::Camera(int ind, std::string yamlName)
    {
        printf("Camera %d is initializing...\n", ind);
        args = new ThreadArgs();
        args->handle = NULL;
        args->frame_empty = 0;

        device = pipeline.getDevice();

        currentPath = std::filesystem::current_path();
        rootPath = currentPath.parent_path();
        printf("Current Path: %s\n", currentPath.c_str());
        configPath = currentPath / "config" / yamlName;

        if(readConfig() == -1) {
            std::cerr << "Read Config Failed!" << std::endl;
        }
        obConfig = std::make_shared<ob::Config>();

        
        /************* color stream ***********/
        std::shared_ptr<ob::VideoStreamProfile> colorProfile = nullptr;
        try {
            auto colorProfiles = pipeline.getStreamProfileList(OB_SENSOR_COLOR);
            if(colorProfiles) {
                colorProfile = colorProfiles->getVideoStreamProfile(width, height, OB_FORMAT_RGB, fps);
            }
            obConfig->enableStream(colorProfile);
        }
        catch(ob::Error &e) {
            std::cerr << "Current device is not support color sensor!" << std::endl;
        }

        /************* depth stream ***********/
        auto depthProfiles = pipeline.getStreamProfileList(OB_SENSOR_DEPTH);
        std::shared_ptr<ob::VideoStreamProfile> depthProfile  = nullptr;
        if(depthProfiles) {
            depthProfile = depthProfiles->getVideoStreamProfile(640, 576, OB_FORMAT_Y16, fps);
        }

        obConfig->enableStream(depthProfile);
        obConfig->setDepthScaleRequire(false);
        obConfig->setAlignMode(ALIGN_D2C_SW_MODE);
        pipeline.enableFrameSync();

        pipeline.start(obConfig);
        printf("Pipeline started\n");

        //********** frame **********/
        args->app = std::make_shared<Window>("driver", colorProfile->width(), colorProfile->height(), RENDER_OVERLAY);

        if(TriggerMode == TriggerMode_Off){
            std::function<void*(ThreadArgs*)> func = std::bind(&Camera::HKWorkThread, this, std::placeholders::_1);
            workthread = std::make_unique<std::thread>(func, args);
        }

    }

    //^ ********************************** Camera constructor************************************ //
    Camera::~Camera()
    {
        //********** frame **********/

        if (TriggerMode == TriggerMode_Off)
            workthread->join();

        pipeline.stop();
        args->app->close();

        delete args;
    }

    //^ ********************************** Camera constructor************************************ //
    int Camera::readConfig()
    {
        try {
            // Load the YAML file
            YAML::Node config = YAML::LoadFile(configPath);
            if (config["scene_name"]) {
                printf("scene_name: %s\n", config["scene_name"].as<std::string>().c_str());
            }
            if (config["colorCamera.width"]) {
                width = config["colorCamera.width"].as<int>();
            }
            if (config["colorCamera.height"]) {
                height = config["colorCamera.height"].as<int>();
            }
            if (config["fps"]) {
                fps = config["fps"].as<int>();
            }
            if (config["auto_exposure"]) {

                bool autoExposure = config["auto_exposure"].as<bool>();
                device->setBoolProperty(OB_PROP_COLOR_AUTO_EXPOSURE_BOOL, autoExposure);
                if (autoExposure == false) {
                    if (config["exposure"]) {
                        int32_t exposure = config["exposure"].as<int32_t>();
                        if(device->isPropertySupported(OB_PROP_COLOR_EXPOSURE_INT, OB_PERMISSION_WRITE)) {
                            OBIntPropertyRange range =device->getIntPropertyRange(OB_PROP_COLOR_EXPOSURE_INT);
                            if(exposure >=range.min && exposure <= range.max && ((exposure -range.min)%range.step == 0)) {
                                device->setIntProperty(OB_PROP_COLOR_EXPOSURE_INT, exposure);
                            }
                            else {
                                printf("exposure: %d is not in the range of [%d, %d] or not a multiple of %d\n", exposure, range.min, range.max, range.step);
                                printf("set exposure error!!!!\n");
                                return -1;
                            }
                        }
                        printf("exposure: %d\n", exposure);
                    }

                    if (config["gain"]) {
                        int32_t gain = config["gain"].as<int32_t>();
                        if(device->isPropertySupported(OB_PROP_COLOR_GAIN_INT, OB_PERMISSION_READ)) {
                            // Obtain the Color camera benefit value.
                            if(device->isPropertySupported(OB_PROP_COLOR_GAIN_INT, OB_PERMISSION_WRITE)) {
                            // Set the Color value of the camera.
                                device->setIntProperty(OB_PROP_COLOR_GAIN_INT, gain);
                            }
                        }
                        printf("gain: %d\n", gain);
                    }
                }
            }
            if (config["min_depth"]) {
                if(device->isPropertySupported(OB_PROP_MIN_DEPTH_INT, OB_PERMISSION_WRITE)) {
                    // Set the minimum Depth in mm.
                    device->setIntProperty(OB_PROP_MIN_DEPTH_INT, config["min_depth"].as<int>());
                }
            }
            if (config["max_depth"]) {
                if(device->isPropertySupported(OB_PROP_MAX_DEPTH_INT, OB_PERMISSION_WRITE)) {
                    // Set the maximum Depth in mm.
                    device->setIntProperty(OB_PROP_MAX_DEPTH_INT, config["max_depth"].as<int>());
                }
            }

        } catch (const YAML::Exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;
        }
        return 0;
    }

    //^ ********************************** Camera constructor************************************ //
    bool Camera::reset()
    {
        return true;
    }

    //^ ********************************** PrintDeviceInfo ************************************ //
    bool Camera::PrintDeviceInfo(ob::DeviceInfo &deviceInfo)
    {
        printf("Device Name: %s\n", deviceInfo.name());
        printf("Device pid: %d\n", deviceInfo.pid());
        printf("Device Serial Number: %s\n", deviceInfo.serialNumber());
        printf("Device Firmware Version: %s\n", deviceInfo.firmwareVersion());
        printf("Device extension: %s\n", deviceInfo.extensionInfo());

        return true;
    }
    //^ ********************************** Camera constructor************************************ //
    void Camera::ReadImg(std::vector<cv::Mat> &images)
    {

        std::lock_guard<std::mutex> lock(args->mutex);
        if (args->frame_empty)
        {
            for (size_t i = 0; i < images.size(); ++i)
            {
                images[i] = cv::Mat();
            }
        }
        else
        {
            images.clear();
            images.resize(args->images.size());

            for (size_t i = 0; i < args->images.size(); ++i)
            {
                if (args->images[i] != nullptr)
                {
                    images[i] = *(args->images[i]);
                }
            }
            args->frame_empty = 1;
        }
    }

    void Camera::TriggerCapture(std::vector<cv::Mat> &images){
    }

    std::future<void> Camera::asyncGetImage(std::vector<cv::Mat> &images){
        if(TriggerMode == TriggerMode_Off){
            return std::async(std::launch::async, [&]{
                do{
                    ReadImg(images);
                }while(images.empty());
            });
        }
        else{
            return std::async(std::launch::async, [&]{
                do{
                    TriggerCapture(images);
                }while(images.empty());
            });
        }
    }

    //^ ********************************** HKWorkThread1 ************************************ //
    void *Camera::HKWorkThread(void *args)
    {
        ThreadArgs *thread_args = (ThreadArgs *)args;
        void *p_handle = thread_args->handle;
        cv::Mat &frame = thread_args->frame;
        cv::Mat &colorMat = thread_args->colorMat;
        cv::Mat &depthMat = thread_args->depthMat;
        bool &frame_empty = thread_args->frame_empty;
        std::mutex &mutex = thread_args->mutex;
        Window &app = *(thread_args->app);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        while(app) {
            if (keyEventProcess(app, pipeline, obConfig) == false) {
                break;
            }
            // Wait for up to 100ms for a frameset in blocking mode.
            auto frameSet = pipeline.waitForFrames(1000);
            if(frameSet == nullptr) {
                std::cout << "The frameset is null!" << std::endl;
                continue;
            }
            frameCount++;

            auto colorFrame = frameSet->colorFrame();
            auto depthFrame = frameSet->depthFrame();
            if(colorFrame != nullptr && depthFrame != nullptr) {
                app.addToRender({ colorFrame, depthFrame });
                if(colorFrame != nullptr) {
                    if(colorFrame->format() != OB_FORMAT_RGB) {
                        if(colorFrame->format() == OB_FORMAT_MJPG) {
                            formatConvertFilter.setFormatConvertType(FORMAT_MJPG_TO_RGB888);
                        }
                        else if(colorFrame->format() == OB_FORMAT_UYVY) {
                            formatConvertFilter.setFormatConvertType(FORMAT_UYVY_TO_RGB888);
                        }
                        else if(colorFrame->format() == OB_FORMAT_YUYV) {
                            formatConvertFilter.setFormatConvertType(FORMAT_YUYV_TO_RGB888);
                        }
                        else {
                            std::cout << "Color format is not support!" << std::endl;
                            continue;
                        }
                        colorFrame = formatConvertFilter.process(colorFrame)->as<ob::ColorFrame>();
                    }
                    formatConvertFilter.setFormatConvertType(FORMAT_RGB888_TO_BGR);
                    colorFrame = formatConvertFilter.process(colorFrame)->as<ob::ColorFrame>();

                    colorMat = cv::Mat(colorFrame->height(), colorFrame->width(), CV_8UC3, colorFrame->data());
                    
                    // uint32_t width = colorFrame->width();
                    // uint32_t height = colorFrame->height();
                    // uint8_t *data = (uint8_t *)colorFrame->data();
                    // uint32_t dataSize = colorFrame->dataSize();
                    // uint64_t timestamp = colorFrame-> timestamp();
                    
                }

                if(depthFrame != nullptr) {
                    depthMat = cv::Mat(depthFrame->height(), depthFrame->width(), CV_16UC1, depthFrame->data());
                }

                auto currentTime = std::chrono::high_resolution_clock::now();
                auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
                if (elapsedTime >= 1) {
                    float fps = (frameCount - lastFrameCount) / elapsedTime;
                    printf("Read Frame Rate: %.2f FPS\n", fps);
                    startTime = currentTime;
                    lastFrameCount = frameCount;
                }
                frame_empty = 0;
            }
            else if (colorFrame == nullptr) {
                std::cout << "Color frame is null!" << std::endl;
            }
            else {
                std::cout << "Depth frame is null!" << std::endl;
            }
        }

        return 0;
    }

    bool Camera::keyEventProcess(Window &app, ob::Pipeline &pipe, std::shared_ptr<ob::Config> config) 
    {
        ////Get the key value
        int key = app.waitKey(10);
        if(key == '+' || key == '=') {
            // Press the + key to increase alpha
            alpha += 0.1f;
            if(alpha >= 1.0f) {
                alpha = 1.0f;
            }
            app.setAlpha(alpha);
        }
        else if(key == '-' || key == '_') {
            // press - key to decrease alpha
            alpha -= 0.1f;
            if(alpha <= 0.0f) {
                alpha = 0.0f;
            }
            app.setAlpha(alpha);
        }
        else if(key == 'D' || key == 'd') {
            // Press the D key to switch the hardware D2C
            try {
                if(!hd2c) {
                    started = false;
                    pipe.stop();
                    hd2c = true;
                    sd2c = false;
                    config->setAlignMode(ALIGN_D2C_HW_MODE);
                    pipe.start(config);
                    started = true;
                }
                else {
                    started = false;
                    pipe.stop();
                    hd2c = false;
                    sd2c = false;
                    config->setAlignMode(ALIGN_DISABLE);
                    pipe.start(config);
                    started = true;
                }
            }
            catch(std::exception &e) {
                std::cerr << "Property not support" << std::endl;
            }
        }
        else if(key == 'S' || key == 's') {
            // Press the S key to switch the software D2C
            try {
                if(!sd2c) {
                    started = false;
                    pipe.stop();
                    sd2c = true;
                    hd2c = false;
                    config->setAlignMode(ALIGN_D2C_SW_MODE);
                    pipe.start(config);
                    started = true;
                }
                else {
                    started = false;
                    pipe.stop();
                    hd2c = false;
                    sd2c = false;
                    config->setAlignMode(ALIGN_DISABLE);
                    pipe.start(config);
                    started = true;
                }
            }
            catch(std::exception &e) {
                std::cerr << "Property not support" << std::endl;
            }
        }
        else if(key == 'F' || key == 'f') {
            // Press the F key to switch synchronization
            SYNC = !SYNC;
            if(SYNC) {
                try {
                    // enable SYNChronization
                    pipe.enableFrameSync();
                }
                catch(...) {
                    std::cerr << "sync not support" << std::endl;
                }
            }
            else {
                try {
                    // turn off SYNC
                    pipe.disableFrameSync();
                }
                catch(...) {
                    std::cerr << "sync not support" << std::endl;
                }
            }
        }
        else if(key == 'e' or key == 'E') {
            return false;
        }

        return true;
    }

} // namespace camera


// For python module
extern "C"{

    int getHeight(camera::Camera* cam){
        return cam->getHeight();
    }

    int getWidth(camera::Camera* cam){
        return cam->getWidth();
    }

    camera::Camera* createCamera(int idx, char *yamlName){
        return new camera::Camera(idx, yamlName);
    }

    void triggerCapture(camera::Camera* cam, char *data){
        std::vector<cv::Mat> src(2);

        do{
            cam->TriggerCapture(src);
        }while(src[0].empty() || src[1].empty());

        size_t colorSize = src[0].total() * src[0].elemSize();
        size_t depthSize = src[1].total() * src[1].elemSize();

        std::memcpy(data, src[0].data, colorSize);
        std::memcpy(data + colorSize, src[1].data, depthSize);
    }

    void getImage(camera::Camera* cam, char *data){
        std::vector<cv::Mat> src(2);

        do{
            cam->ReadImg(src);
        }while(src[0].empty() || src[1].empty());

        size_t colorSize = src[0].total() * src[0].elemSize();
        size_t depthSize = src[1].total() * src[1].elemSize();

        std::memcpy(data, src[0].data, colorSize);
        std::memcpy(data + colorSize, src[1].data, depthSize);
    }
}

#endif
