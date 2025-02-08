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

std::string getCurrentTimeAsString() {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // Convert to local time
    std::tm local_tm = *std::localtime(&time_t_now);
    
    // Format the time as a string
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S"); // Format: YYYY-MM-DD_HH-MM-SS
    return oss.str();
}

std::string formatNumber(int number) {
    std::ostringstream oss;
    oss << std::setw(6) << std::setfill('0') << number; // Set width to 6 and fill with '0'
    return oss.str();
}

class CameraStreamRecorder 
{
    public:
        CameraStreamRecorder()
        {
            colorCount = 0;
            depthCount = 0;

            currentPath = std::filesystem::current_path();
            rootPath = currentPath.parent_path();

            if (readConfig() == -1) {
                std::cerr << "[ERROR] capture reading config failed!" << std::endl;
            }

            std::string dirName = getCurrentTimeAsString();
            dirPath = motionPath / dirName;
            if (std::filesystem::create_directories(dirPath)) {
                std::cout << "Directory created: " << dirPath << std::endl;
                colorPath = dirPath / "images";
                depthPath = dirPath / "depth";
                std::filesystem::create_directory(colorPath);
                std::filesystem::create_directory(depthPath);
            } else {
                std::cerr << "Directory already exists or could not be created." << std::endl;
            }

            camera::Camera cam(0, "record.yaml");
                    
            while(cam.windowOK()) {
                if (keyEventProcess() == false) {
                    break;
                }

                auto cameraFuture = cam.asyncGetImage(images);
                cameraFuture.get();

                if (!images[0].empty()) {
                    std::string filename = colorPath / ("color_" + formatNumber(colorCount) + ".jpg");
                    cv::imwrite(filename, images[0]);
                    colorCount++;
                }
                if (!images[1].empty()) {
                    std::string filename = depthPath / ("depth_" + formatNumber(depthCount) + ".png");
                    cv::imwrite(filename, images[1]);
                    depthCount++;
                }
            }
        }
    
    private:

        int readConfig()
        {
            try {
                // Load the YAML file
                YAML::Node config = YAML::LoadFile(rootPath / "config" / "record.yaml");
                if (config["scene_name"]) {
                    motionPath = rootPath / "data" / config["scene_name"].as<std::string>() / "motion";
                } 
            } catch (const YAML::Exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return -1;
            }
            return 0;
        }

        bool keyEventProcess() {
            int key = cv::waitKey(20);
            if (key == 'e' || key == 'E') {
                return false;
            }
            return true;
        }

        std::vector<cv::Mat> images{std::vector<cv::Mat>(2)};
        std::filesystem::path rootPath, currentPath, motionPath, dirPath, colorPath, depthPath;
        int colorCount, depthCount;
};

int main()
{
    CameraStreamRecorder recorder;
    return 0;
}