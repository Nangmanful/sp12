#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include "qrrecognition.h"
#include <iostream>
#include <string>

#include <ctime>

const char* qrrecognition() {
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "카메라를 열 수 없습니다." << std::endl;
        return "-1";
    }

    cv::Mat frame;
    cv::QRCodeDetector qrDecoder;

    std::clock_t start_time = std::clock();

    while (true) {
        std::cout << "카메라 시작" << std::endl;
        // 현재 시간을 가져옵니다.
        std::clock_t current_time = std::clock();

        // 경과된 시간이 1초 이상인지 확인합니다.
        double elapsed_time = static_cast<double>(current_time - start_time) / CLOCKS_PER_SEC;
        if (elapsed_time >= 1.0) {
            std::cout << "1초 지남" << std::endl;
            break;
        }

        cap >> frame; // 프레임 캡처
        if (frame.empty()) break;

        std::vector<cv::Point> points;
        std::string data = qrDecoder.detectAndDecode(frame, points);
        
        if (!data.empty()) {
            std::cout << "qr 인식!" << std::endl;
            const char* data2 = data.c_str();;
            return data2;
        }

        char c = (char)cv::waitKey(25);
        if (c == 27) // ESC 키
            break;
        std::cout << "카메라 작동 중" << std::endl;
    }

    cap.release();
    cv::destroyAllWindows();
    std::cout << "이제 나감" << std::endl;
    return "77";
}



//     cv::VideoCapture cap(0, cv::CAP_V4L2);

//     if (!cap.isOpened()) {
//         std::cerr << "카메라를 열 수 없습니다." << std::endl;
//         return -1;
//     }

//     cv::Mat frame;
//     cv::QRCodeDetector qrDecoder;

//     while (true) {
//         cap >> frame; // 프레임 캡처
//         if (frame.empty()) break;

//         std::vector<cv::Point> points;
//         std::string data = qrDecoder.detectAndDecode(frame, points);
        
//         if (!data.empty()) {
//             std::cout << "디코드된 데이터: " << data << std::endl;
//             // QR 코드를 프레임에 표시
//             for (size_t i = 0; i < points.size(); i++)
//                 line(frame, points[i], points[(i+1) % points.size()], cv::Scalar(255,0,0), 4);
//         }

//         cv::imshow("QR 코드 스캐너", frame);
//         char c = (char)cv::waitKey(25);
//         if (c == 27) // ESC 키
//             break;
//     }

//     cap.release();
//     cv::destroyAllWindows();
