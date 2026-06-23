import sensor, image, time, lcd

from modules import ybserial
import time

serial = ybserial()

lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 100)


msg =""
num=0
n=0
clock = time.clock()
while(True):
    clock.tick()
    img = sensor.snapshot()
    for code in img.find_qrcodes():
        img.draw_rectangle(code.rect(), color = 127, thickness=3)
        img.draw_string(code.x(),code.y()-20,code.payload(),color=(255,0,0),scale=2)
        msg = code.payload()
        print(code)


    if msg !="":
        send_data = "$"+msg+"#"
        msg = ""
        serial.send(send_data)
    else :
        msg = ""
        send_data = "$"+msg+"#"
        serial.send(send_data)


    time.sleep_ms(100)
    lcd.display(img)
    #print(clock.fps())
