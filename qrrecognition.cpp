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
    static std::string data; // Use static variable to persist data after function returns

    while (true) {
        // 현재 시간을 가져옵니다.
        cap >> frame; // 프레임 캡처
        if (frame.empty()) break;

        std::vector<cv::Point> points;
        data = qrDecoder.detectAndDecode(frame, points);

        if (!data.empty()) {
            std::cout << "qr 인식!" << std::endl;
            cap.release();
            return data.c_str();
        }

        // Explicitly release the frame memory
        frame.release();
    }

    cap.release();
    std::cout << "이제 나감" << std::endl;
    return "77";
}
