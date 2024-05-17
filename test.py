import cv2
import time

def show_camera(index=0):
    cap = cv2.VideoCapture(index)
    
    if not cap.isOpened():
        print(f"Cannot open camera with index {index}")
        return False
    
    print(f"Camera {index} opened successfully")
    
    # 몇 초 동안 대기하여 카메라 초기화 시간 확보
    time.sleep(2)
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Can't receive frame from camera. Exiting ...")
            break
        
        cv2.imshow(f'Camera {index}', frame)
        
        if cv2.waitKey(1) == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
    return True

for i in range(10, 19):
    print(f"Trying camera index {i}")
    if show_camera(i):
        print(f"Camera index {i} is working")
    else:
        print(f"Camera index {i} is not working")
