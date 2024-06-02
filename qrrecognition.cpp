#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include "qrrecognition.h"

const char* qrrecognition() {
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "카메라를 열 수 없습니다." << std::endl;
        return "-1";
    }

    cv::QRCodeDetector qrDecoder;
    cv::Mat frame;
    std::clock_t start_time = std::clock();

    while (true) {
        // 현재 시간을 가져옵니다.
        std::clock_t current_time = std::clock();

        // 경과된 시간이 10초 이상인지 확인합니다.
        double elapsed_time = static_cast<double>(current_time - start_time) / CLOCKS_PER_SEC;
        if (elapsed_time >= 10) {
            std::cout << "10초 지남" << std::endl;
            break;
        }

        cap >> frame; // 프레임 캡처
        if (frame.empty()) break;

        std::vector<cv::Point> points;
        std::string data = qrDecoder.detectAndDecode(frame, points);
        
        if (!data.empty()) {
            std::cout << "qr 인식!" << std::endl;
            cap.release();
            const char* data2 = data.c_str();;
            return data2;
        }
    }

    cap.release();
    std::cout << "이제 나감" << std::endl;
    return "77";
}
