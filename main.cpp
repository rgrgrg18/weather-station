#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Wire.h>
#include <DHT11.h>
#include <Adafruit_BMP085.h>

// Пины для дисплея
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Сенсоры
DHT11 dht11(2); // DHT11 подключён к пину 2
Adafruit_BMP085 bmp; // BMP180 использует I2C (SDA, SCL)

// Истории значений
float tempHistory[60];
float humHistory[60];
float pressHistory[60];
int dataIndex = 0;

// Переменная для переключения графиков
int graphMode = 0; // 0 - давление, 1 - температура, 2 - влажность
unsigned long lastSwitchTime = 0;

void setup() {
  Serial.begin(9600);
  tft.init(240, 240, SPI_MODE2);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_RED);
  tft.setCursor(20, 40);
  tft.print("Weather Station");
  delay(1000);

  if (!bmp.begin()) {
    tft.setCursor(20, 80);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(1);
    tft.println("BMP180 error!");
    while (1);
  }
}

void loop() {
  int temp = 0;
  int hum = 0;
  int result = dht11.readTemperatureHumidity(temp, hum);
  float pressure = bmp.readPressure() / 133.322; // в мм рт. ст.

  tft.fillScreen(ST77XX_BLACK);

  if (result == 0) {
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(5, 10);
    tft.print("Temp: ");
    tft.print(temp);
    tft.println(" C");

    tft.setCursor(5, 40);
    tft.setTextColor(ST77XX_CYAN);
    tft.print("Hum: ");
    tft.print(hum);
    tft.println(" %");

    tft.setCursor(5, 70);
    tft.setTextColor(ST77XX_GREEN);
    tft.print("Pres: ");
    tft.print(pressure, 1);
    tft.println(" mmHg");

    // Сохраняем в историю
    tempHistory[dataIndex] = temp;
    humHistory[dataIndex] = hum;
    pressHistory[dataIndex] = pressure;
    dataIndex = (dataIndex + 1) % 60;
  } else {
    tft.setCursor(5, 10);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.print("Sensor error!");
  }

  // Отрисовка графика
  drawGraphArea();

  delay(2000);

  // Переключение графика каждые 2 секунды
  graphMode = (graphMode + 1) % 3;
}

void drawGraphArea() {
  int graphX = 10, graphY = 120, graphW = 220, graphH = 100;

  float* data;
  uint16_t color;
  const char* label;

  switch (graphMode) {
    case 0:
      data = pressHistory;
      color = ST77XX_GREEN;
      label = "Pressure";
      break;
    case 1:
      data = tempHistory;
      color = ST77XX_YELLOW;
      label = "Temp";
      break;
    case 2:
      data = humHistory;
      color = ST77XX_CYAN;
      label = "Humidity";
      break;
  }

  // Заголовок
  tft.setTextColor(color);
  tft.setCursor(graphX, graphY - 10);
  tft.setTextSize(2);
  tft.print(label);

  // График
  float maxVal = -9999, minVal = 9999;
  for (int i = 0; i < 60; i++) {
    if (!isnan(data[i])) {
      if (data[i] > maxVal) maxVal = data[i];
      if (data[i] < minVal) minVal = data[i];
    }
  }
  if (maxVal == minVal) maxVal += 1.0;

  for (int i = 1; i < 60; i++) {
    int x0 = graphX + map(i - 1, 0, 59, 0, graphW);
    int y0 = graphY + graphH - map(data[i - 1], minVal, maxVal, 0, graphH);
    int x1 = graphX + map(i, 0, 59, 0, graphW);
    int y1 = graphY + graphH - map(data[i], minVal, maxVal, 0, graphH);
    tft.drawLine(x0, y0, x1, y1, color);
  }
}
