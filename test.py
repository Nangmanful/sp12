import cv2

def show_camera(index):
    cap = cv2.VideoCapture(index)
    if not cap.isOpened():
        print(f"Cannot open camera with index {index}")
        return False
    
    print(f"Camera {index} opened successfully")
    while True:
        ret, frame = cap.read()
        if not ret:
            print(f"Can't receive frame from camera {index}. Exiting ...")
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
