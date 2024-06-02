#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <ctime>
#include <string>

extern "C" {
    const char* qrrecognition(cv::VideoCapture* cap) {
        static std::string result;

        if (!cap->isOpened()) {
            result = "카메라를 열 수 없습니다.";
            return result.c_str();
        }

        cv::QRCodeDetector qrDecoder;
        std::clock_t start_time = std::clock();

        while (true) {
            cv::Mat frame;
            std::cout << "카메라 시작" << std::endl;
            std::clock_t current_time = std::clock();
            double elapsed_time = static_cast<double>(current_time - start_time) / CLOCKS_PER_SEC;

            if (elapsed_time >= 10) {
                std::cout << "2분 지남" << std::endl;
                break;
            }

            (*cap) >> frame; // 프레임 캡처
            if (frame.empty()) break;

            std::vector<cv::Point> points;
            std::string data = qrDecoder.detectAndDecode(frame, points);

            if (!data.empty()) {
                std::cout << "QR 인식!" << std::endl;
                result = data;
                frame.release();
                return result.c_str();
            }

            std::cout << "카메라 작동 중" << std::endl;
            frame.release();
        }

        std::cout << "이제 나감" << std::endl;
        result = "77";
        return result.c_str();
    }

    cv::VideoCapture* create_capture() {
        return new cv::VideoCapture(0, cv::CAP_V4L2);
    }

    void release_capture(cv::VideoCapture* cap) {
        cap->release();
        delete cap;
    }
}
