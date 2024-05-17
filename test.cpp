import cv2

def show_camera(index=0):
    cap = cv2.VideoCapture(index)
    
    if not cap.isOpened():
        print(f"Cannot open camera with index {index}")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Can't receive frame (stream end?). Exiting ...")
            break
        
        cv2.imshow('Camera', frame)
        
        if cv2.waitKey(1) == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

show_camera()
