✅ 完整程式碼（ESP8266 + DHT11 + Chart.js + Responsive + Dark Mode）

請確認你已接好 DHT11（GPIO5），RGB LED（12/13/14），並安裝好 DHT sensor library 和 Adafruit Unified Sensor。

Getting Started with ESP8266 NodeMCU Development Board
https://randomnerdtutorials.com/getting-started-with-esp8266-wifi-transceiver-review/

* DiFi 是一塊整合感測元件在板子上的學習板，針對學生上課時可省去接線的困擾。
* ArduinoDroid可以開啟新的sketch ，複製貼上程式碼，按下compile，再上傳。
 
*Arduinodroid 伺服器位置IP如下圖，monitor or hotspot都看的到 一樣的，沒反應可以按開發版的reset，按下去板子的燈會亮
 
 


*電腦上NodeMCU 需要安裝驅動程式，才能讓電腦識別它並進行編程。不同版本的 NodeMCU 使用不同的 USB-to-Serial 轉換晶片

安裝
 


將NodeMCU用USB線和電腦連接，裝好驅動程式，就可以使用了。上傳檔案時晶片燈會亮。
 

  
 


晶片範例
 
Best Pins to Use – ESP8266
One important thing to notice about ESP8266 is that the GPIO number doesn’t match the label on the board silkscreen. For example, D0 corresponds to GPIO16 and D1 corresponds to GPIO5.
The following table shows the correspondence between the labels on the silkscreen and the GPIO number as well as what pins are the best to use in your projects, and which ones you need to be cautious.
The pins highlighted in green are OK to use. The ones highlighted in yellow are OK to use, but you need to pay attention because they may have unexpected behavior mainly at boot. The pins highlighted in red are not recommended to use as inputs or outputs.
Label	GPIO	Input	Output	Notes
D0	GPIO16	no interrupt	no PWM or I2C support	HIGH at boot
used to wake up from deep sleep
D1	GPIO5	OK	OK	often used as SCL (I2C)
D2	GPIO4	OK	OK	often used as SDA (I2C)
D3	GPIO0	pulled up	OK	connected to FLASH button, boot fails if pulled LOW
D4	GPIO2	pulled up	OK	HIGH at boot
connected to on-board LED, boot fails if pulled LOW
D5	GPIO14	OK	OK	SPI (SCLK)
D6	GPIO12	OK	OK	SPI (MISO)
D7	GPIO13	OK	OK	SPI (MOSI)
D8	GPIO15	pulled to GND	OK	SPI (CS)
Boot fails if pulled HIGH
RX	GPIO3	OK	RX pin	HIGH at boot
TX	GPIO1	TX pin	OK	HIGH at boot
debug output at boot, boot fails if pulled LOW
A0	ADC0	Analog Input	X	

SP8266 是以 wifi 晶片為基礎，也可以做遠端遙控喔！
 

 
	
	
	


Arduino的安裝說明： (ARDUINO 1.8.7下載)
打開ARDUINO軟體,檔案>>偏好設定,在下方額外的開發板管理員網址:輸入http://arduino.esp8266.com/stable/package_esp8266com_index.json
 
接著到工具>>開發板>>開發板管理員

 

	
	
	
	
拉到最下一欄會多一項ESP8266,需在欄內點一下才會出現安裝訊息

 

	
	
	


安裝好後會顯示INSTALLED

 

	
	
	
	接下來到工具>>開發板>>選 Generic ESP8266 Module


但我們這次採用DIFI板子，但選擇 NodeMCU 1.0
 
On-board LED
Most of the ESP8266 development boards have a built-in LED. This LED is usually connected to GPIO2.
 





