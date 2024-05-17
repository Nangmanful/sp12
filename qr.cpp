#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

int main() {
    // 카메라 디바이스 오픈 (/dev/video10을 예로 사용)
    cv::VideoCapture cap(14);
    if (!cap.isOpened()) {
        std::cerr << "카메라를 열 수 없습니다." << std::endl;
        return -1;
    }

    cv::Mat frame;
    cv::namedWindow("Camera Feed", cv::WINDOW_AUTOSIZE);

    while (true) {
        // 카메라로부터 새 프레임 읽기
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "비디오 캡처가 종료되었습니다." << std::endl;
            break;
        }

        // 화면에 표시
        cv::imshow("Camera Feed", frame);

        // ESC 키(아스키 코드 27)를 누르면 종료
        if (cv::waitKey(30) >= 0) break;
    }

    // 종료 시 모든 자원 해제
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
