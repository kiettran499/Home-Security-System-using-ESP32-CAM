import cv2
import urllib.request
import numpy as np
import cvlib as cv
from cvlib.object_detection import draw_bbox
import requests  # Thêm thư viện requests để gửi HTTP requests

# URL của ESP32-CAM
url = 'http://192.168.190.26/cam-hi.jpg'
led_on_url = 'http://192.168.190.26/led/on'
led_off_url = 'http://192.168.190.26/led/off'

def get_frame():
    img_resp = urllib.request.urlopen(url)
    imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
    return cv2.imdecode(imgnp, -1)

def run2():
    cv2.namedWindow("detection", cv2.WINDOW_AUTOSIZE)
    person_detected = False  # Biến để lưu trạng thái phát hiện người

    while True:
        im = get_frame()
        bbox, label, conf = cv.detect_common_objects(im)

        # Lọc chỉ các bounding box chứa "person"
        filtered_bbox = []
        filtered_label = []
        filtered_conf = []

        for i in range(len(label)):
            if label[i] == "person":
                filtered_bbox.append(bbox[i])
                filtered_label.append(label[i])
                filtered_conf.append(conf[i])

        # Bật LED nếu phát hiện "person", nếu không thì tắt
        if filtered_label:
            if not person_detected:
                requests.get(led_on_url)  # Gửi yêu cầu bật LED
                person_detected = True
        else:
            if person_detected:
                requests.get(led_off_url)  # Gửi yêu cầu tắt LED
                person_detected = False

        # Vẽ các bounding boxes chỉ cho người
        im = draw_bbox(im, filtered_bbox, filtered_label, filtered_conf)
        
        cv2.imshow('detection', im)
        key = cv2.waitKey(5)
        if key == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == '__main__':
    print("started")
    run2()
