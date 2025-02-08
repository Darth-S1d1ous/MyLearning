#include <atomic>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <filesystem>
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

bool save = false;

namespace camera {
    int Camera::frameCount = 0;
    int Camera::lastFrameCount = 0;
    int Camera::saveCount = 0;
    int Camera::saveCountIMU = 0;
}

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
            rootPath = currentPath;

            camera::Camera cam(0, "scan.yaml");
            
            if (readConfig() == -1) {
                std::cerr << "[ERROR] Reading config failed!" << std::endl;
            }

            saveCount = 0;

            while(cam.windowOK()) {
                if (keyEventProcess() == false) {
                    break;
                }

                bool saveImage = false;
                saveImage = save;
                save = false;

                auto cameraFuture = cam.asyncGetImage(images);
                cameraFuture.get();

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
                    savePath = rootPath.parent_path() / "data" / config["scene_name"].as<std::string>() / "scene";
                    if (copyFile(configPath, rootPath.parent_path() / "data" / config["scene_name"].as<std::string>() / "config" / "scan.yaml")) {
                        std::cout << "File copy operation succeeded.\n";
                    } else {
                        std::cout << "File copy operation failed.\n";
                    }

                    saveColorPath = savePath / "color";
                    saveDepthPath = savePath / "depth";
                    saveIMUPath = savePath / "imu";
                    std::filesystem::create_directory(rootPath.parent_path() / "data" / config["scene_name"].as<std::string>());
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
                save = true;
            }
            else if (key == 'e' || key == 'E')
            {
                return false;
            }
            return true;
        }

        std::vector<cv::Mat> images{std::vector<cv::Mat>(2)};
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