import time
import Adafruit_DHT
import Adafruit_SSD1306
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
#Raspberry Pi DHT11 sensor setting:
sensor = Adafruit_DHT.DHT11
DHT11_pin = 4
count = ' '
name = '배준성'
# Raspberry Pi pin configuratoin:
RST = 24
#124x64 display with hardwareI2C:
disp = Adafruit_SSD1306.SSD1306_128_64(rst=RST)
#initialize Library
disp.begin()
width = disp.width
height = disp.height
top=10
font=ImageFont.truetype('/usr/share/fonts/truetype/nanum/NanumGothic.ttf',13)

start=0
try:
    while True:
        while start==0:
        
            #clr display
            disp.clear()
            disp.display()
            
            #logo upload
            image=Image.open('/home/pi/Desktop/logo.png').resize((width,height)).convert('1')
            disp.image(image)
            disp.display()
            time.sleep(1.75)
            start=1
        
        if count == '_':count = ' '
        else:count = '_'
        
        h, t = Adafruit_DHT.read_retry(sensor,DHT11_pin)
        if h is not None and t is not None:
            temp="{0}\n{1}\n온도: {2}˚C \n습도: {3}%".format(count,name,int(t),int(h))
        else:
            print('Read error')
        
        #화면크기 빈 이미지생성
        image=Image.new('1',(width,height))
        draw=ImageDraw.Draw(image)
        #이미지위에 텍스트 출력
        draw.text((0,0),temp,font=font,fill=255)
        #이미지 OLED로 출력
        disp.image(image)
        disp.display()
        time.sleep(0.5)

except KeyboardInterrupt:
    print("Terminated by Keyboard")
    
finally:
    print("End of Promgram")
    image=Image.open('/home/pi/Desktop/A.png').resize((width,height)).convert('1')
    draw=ImageDraw.Draw(image)
    draw.text((0,0),' 프로그램을 종료합니다',font=font,fill=255)
    disp.image(image)
    disp.display()
    time.sleep(5)
    #clr display
    disp.clear()
    disp.display()


