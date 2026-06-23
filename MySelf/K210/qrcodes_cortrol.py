import sensor, image, time, lcd
from modules import ybserial

ser = ybserial()
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=100)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
sensor.run(1)
clock = time.clock()

COLORS = {
    'RED':    (30, 70, 40, 100, 20, 80),
    'YELLOW': (40, 100, -20, 20, 40, 100),
    'BLUE':   (10, 50, -40, 20, -70, -10),
}

def count_all_balls(img):
    cnt = {}
    for color_name, th in COLORS.items():
        blobs = img.find_blobs([th], pixels_threshold=50, area_threshold=50, merge=True, margin=10)
        cnt[color_name] = len(blobs)
    return cnt

msg = ""

while True:
    clock.tick()
    img = sensor.snapshot()
    fps = clock.fps()

    ball_counts = count_all_balls(img)
    y_offset = 5
    for color_name in ['RED', 'YELLOW', 'BLUE']:
        n = ball_counts.get(color_name, 0)
        if color_name == 'RED':    c = (255, 0, 0)
        elif color_name == 'YELLOW': c = (255, 255, 0)
        else:                      c = (0, 0, 255)
        img.draw_string(5, y_offset, color_name + ": " + str(n), color=c, scale=1.2)
        y_offset += 18

    for code in img.find_qrcodes():
        img.draw_rectangle(code.rect(), color=127, thickness=3)
        img.draw_string(code.x(), code.y()-20, code.payload(), color=(255, 0, 0), scale=2)
        msg = code.payload()
        print(code)

    if msg != "":
        send_data = "$" + msg + "#"
        msg = ""
        ser.send(send_data)
    else:
        ser.send("$#")

    img.draw_string(200, 5, "FPS: " + str(int(fps)), color=(255, 255, 255), scale=1.2)
    time.sleep_ms(100)
    lcd.display(img)
