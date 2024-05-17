#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>

int main() {
    VideoCapture cap(0, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "카메라를 열 수 없습니다." << std::endl;
        return -1;
    }

    cv::Mat frame;
    cv::QRCodeDetector qrDecoder;

    while (true) {
        cap >> frame; // 프레임 캡처
        if (frame.empty()) break;

        std::vector<cv::Point> points;
        std::string data = qrDecoder.detectAndDecode(frame, points);
        
        if (!data.empty()) {
            std::cout << "디코드된 데이터: " << data << std::endl;
            // QR 코드를 프레임에 표시
            for (size_t i = 0; i < points.size(); i++)
                line(frame, points[i], points[(i+1) % points.size()], cv::Scalar(255,0,0), 4);
        }

        cv::imshow("QR 코드 스캐너", frame);
        char c = (char)cv::waitKey(25);
        if (c == 27) // ESC 키
            break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
