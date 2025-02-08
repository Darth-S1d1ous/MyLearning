#include <atomic>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "opencv2/core/mat.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <vector>
#include <future>
#include "orbbec_camera.hpp"
#include <thread>
#include <functional>


namespace camera {
    int Camera::frameCount = 0;
    int Camera::lastFrameCount = 0;
    int Camera::saveCount = 0;
    int Camera::saveCountIMU = 0;
}

pthread_mutex_t lock;
pthread_mutex_t lockIMU;
bool save = false;
bool saveIMU = false;

std::string formatNumber(int number) {
    std::ostringstream oss;
    oss << std::setw(6) << std::setfill('0') << number; // Set width to 6 and fill with '0'
    return oss.str();
}

bool copyFile(const std::filesystem::path& source, const std::filesystem::path& destination) {
    try {
        // Check if the source file exists
        if (!std::filesystem::exists(source)) {
            std::cerr << "Error: Source file does not exist.\n";
            return false;
        }

        // Check if the destination directory exists; if not, create it
        if (!std::filesystem::exists(destination.parent_path())) {
            std::filesystem::create_directories(destination.parent_path());
        }

        // Copy the file
        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
        std::cout << "File copied successfully from " << source << " to " << destination << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }
}

class CameraStreamCapture 
{
    public:
        CameraStreamCapture()
        {
            printf("Start capture");

            currentPath = std::filesystem::current_path();
            rootPath = currentPath.parent_path();
            printf("Current Path: %s\n", currentPath.c_str());
            configPath = rootPath / "config" / "scan.yaml";

            camera::Camera cam(0, "scan.yaml");
            
            if (readConfig() == -1) {
                std::cerr << "[ERROR] capture reading config failed!" << std::endl;
            }

            saveCount = 0;

            std::cerr << "=== capture ok1 ===" << std::endl;
            while(cam.windowOK()) {
                if (keyEventProcess() == false) {
                    break;
                }
                std::cerr << "=== capture ok2 ===" << std::endl;

                pthread_mutex_lock(&lock);
                bool saveImage = false;
                saveImage = save;
                save = false;
                pthread_mutex_unlock(&lock);

                pthread_mutex_lock(&lockIMU);
                bool saveIMU_local = false;
                saveIMU_local = saveIMU;
                saveIMU = false;
                pthread_mutex_unlock(&lockIMU);

                auto cameraFuture = cam.asyncGetImage(images);
                std::cerr << "=== capture ok3 ===" << std::endl;
                auto imuFuture = cam.asyncGetIMU(imuData);
                cameraFuture.get();
                imuFuture.get();

                std::cerr << "=== capture ok4 ===" << std::endl;
                if (!images[0].empty()) {
                    if (saveImage) {
                        std::string filename = saveColorPath / ("color_" + formatNumber(saveCount) + ".jpg");
                        cv::imwrite(filename, images[0]);
                        printf("%d color image saved to %s", saveCount, filename.c_str());
                    }
                }
                if (!images[1].empty()) {
                    if (saveImage) {
                        std::string filename = saveDepthPath / ("depth_" + formatNumber(saveCount) + ".png");
                        cv::imwrite(filename, images[1]);
                        printf("%d depth image saved to %s", saveCount, filename.c_str());
                    }
                }
                
                if (saveIMU_local) {
                    std::string filename = saveIMUPath / ("imu_" + formatNumber(saveCountIMU) + ".txt");
                    
                    std::ofstream imuFile(filename, std::ios::app);
                    if (imuFile.is_open()) {
                        imuFile << imuData[0] << " " << imuData[1] << " " << imuData[2] << "\n";
                        imuFile << imuData[3] << " " << imuData[4] << " " << imuData[5] << "\n";
                        imuFile.close();
                        printf("%d imu data saved to %s", saveCountIMU, filename.c_str());
                    } else {
                        std::cerr << "[Error]: Unable to open imu file " << filename << "\n";
                    }          
                    
                    saveCountIMU++;
                }

                saveCount += saveImage;
            }
        }

        ~CameraStreamCapture()
        {
            printf("End capture");
        }

    private:

        int readConfig()
        {
            try {
                // Load the YAML file
                YAML::Node config = YAML::LoadFile(configPath);
                if (config["scene_name"]) {
                    savePath = rootPath / "data" / config["scene_name"].as<std::string>() / "scene";
                    if (copyFile(configPath, rootPath.parent_path() / "data" / config["scene_name"].as<std::string>() / "config" / "scan.yaml")) {
                        std::cout << "File copy operation succeeded.\n";
                    } else {
                        std::cout << "File copy operation failed.\n";
                    }

                    saveColorPath = savePath / "color";
                    saveDepthPath = savePath / "depth";
                    saveIMUPath = savePath / "imu";
                    std::filesystem::create_directory(rootPath / "data" / config["scene_name"].as<std::string>());
                    std::filesystem::create_directory(savePath);
                    std::filesystem::create_directory(saveColorPath);
                    std::filesystem::create_directory(saveDepthPath);
                    std::filesystem::create_directory(saveIMUPath);
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
            } catch (const YAML::Exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return -1;
            }
            return 0;
        }

        bool keyEventProcess() {
            int key = cv::waitKey(20);
            if (key == 'c' || key == 'C') {
                pthread_mutex_lock(&lock);
                save = true;
                pthread_mutex_unlock(&lock);

                pthread_mutex_lock(&lockIMU);
                saveIMU = true;
                pthread_mutex_unlock(&lockIMU);
            }
            else if (key == 'e' || key == 'E')
            {
                return false;
            }
            return true;
        }

        std::vector<cv::Mat> images{std::vector<cv::Mat>(2)};
        std::vector<float> imuData{std::vector<float>(6)};
        static int saveCount, saveCountIMU;
        std::filesystem::path currentPath, rootPath, configPath, savePath, saveColorPath, saveDepthPath, saveIMUPath;
        int width = 1280, height = 960, fps = 30;
};

int CameraStreamCapture::saveCount = 0;
int CameraStreamCapture::saveCountIMU = 0;

void signal_handler(int signum) 
{
    printf("Interrupt signal (%d) received.\n", signum);
    std::exit(signum);
}

int main()
{
    signal(SIGINT, signal_handler);
    CameraStreamCapture();
    return 0;
}