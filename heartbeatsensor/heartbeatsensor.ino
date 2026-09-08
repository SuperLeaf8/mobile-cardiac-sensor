#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ST7796S_kbv.h"


#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ST7796S_kbv tft = Adafruit_ST7796S_kbv(TFT_CS, TFT_DC, TFT_RST);
int16_t center[2]; // to make easier for drawing targets
// 480 x 320

void drawArc(int16_t h, int16_t k, float theta0, float theta1, float radius, float res, uint16_t color) { // draw arc, 0 is towards up and is CW because flipped screen
	theta0 -= PI/2;
	theta1 -= PI/2;
	int16_t x0 = (int16_t)round(cos(theta0) * radius) + h;
	int16_t y0 = (int16_t)round(sin(theta0) * radius) + k;
	int16_t x;
	int16_t y;
	for (float i = theta0; i < theta1; i += res)
	{
		x = (int16_t)round(cos(i) * radius) + h;
		y = (int16_t)round(sin(i) * radius) + k;

		// tft.writePixel(x, y, ST7796S_GREEN); // center point is (50,100)
		tft.drawLine(x0, y0, x, y, color);
		x0 = x;
		y0 = y;
	}
	x = (int16_t)round(cos(theta1) * radius) + h;
	y = (int16_t)round(sin(theta1) * radius) + k;
	tft.drawLine(x0, y0, x, y, color);
}

#define BAUD_RATE 256000

// Target details
int16_t target1_x = 0;
uint16_t target1_y = 0;
uint16_t target1_res = 0;

int16_t target2_x = 0;
uint16_t target2_y = 0;
uint16_t target2_res = 0;

int16_t target3_x = 0;
uint16_t target3_y = 0;
uint16_t target3_res = 0;

// Multi-Target Detection Command
uint8_t MTD_CMD[12] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x90, 0x00, 0x04, 0x03, 0x02, 0x01};

#define AUTOFOR(i, n) for(auto i = 0; i < n; i++)     // Makes for loops neater in our code, feel free to argue with me about this

// Variables
#define HEADER_SIZE 4
#define TAIL_SIZE   2
#define DATA_SIZE   24
uint8_t FRAME_DATA[DATA_SIZE + TAIL_SIZE] = {0};
uint8_t HEADER_BUF[HEADER_SIZE] = {0};

bool READ_STATE = false;    // Are we currently reading data to FRAME_DATA ?
int HEADER_POINTER = 0; 	// Circular register implementation :) || We want this to be signed because we want backwards warping

int refresht = 800; // ms to wait before refresh
// Flattens our circular buff and compares it with more garbage
// Possible issues: endianness
// The pointer will point to the last recieved 
uint32_t compare_header(uint32_t src)
{
	uint32_t result = 0;
	AUTOFOR(i, 4) // Only works for 32 bit
	{
		HEADER_POINTER = (--HEADER_POINTER + HEADER_SIZE) % HEADER_SIZE;	// Step down header pointer
		result |= (uint32_t)HEADER_BUF[HEADER_POINTER] << i * 8;      		// Turn 1B -> 4B, shift it to correct place, "add" it to result since slot is always 0
	}
	return result == src;                                       // Feel free to change return type, but in this sketch we only care about comparing shit
}

// Helper function to insert into our circular array
void insert_into_header(uint8_t data)
{
	HEADER_BUF[HEADER_POINTER] = data;
	HEADER_POINTER = (++HEADER_POINTER + HEADER_SIZE) % HEADER_SIZE;
}

int sm_to_tc(uint16_t src)
{
  int m = src & 0x7FFF;
  return m * (src & 0x8000 ? 1 : -1);
}

float range = 5000; // in mm
int segs = range/1000; // lines to draw, try to keep it like range/1000
int pixelMax = 300;
float conv = pixelMax/range;
float angle = (50) * (PI/180.0); // angle of radar to draw

void setup() {
    tft.begin();
	tft.fillScreen(ST7796S_BLACK); // all this to write some information
	tft.setRotation(1);
	tft.setTextColor(ST7796S_RED);
	tft.setTextSize(3);
	tft.setCursor(140, 100);
	tft.print("Powered on.");
	tft.setCursor(120, 150);
	tft.print("Initializing...");
	delay(2000);
	refresh();
	// drawArc(100,100,0,PI,50,0.1,ST7796S_GREEN);
	center[0] = tft.width()/2;
	center[1] = tft.height();
	Serial.begin(BAUD_RATE);
	Serial.write(MTD_CMD, 12);
	Serial.flush();

}

void refresh() {
	tft.setTextColor(ST7796S_RED);
	tft.fillScreen(ST7796S_BLACK);
	tft.setTextSize(1);
	tft.setCursor(15, 230);
	tft.print("Mobile Realtime");
	tft.setCursor(15, 240);
	tft.print("Cardiac Sensor");
	tft.setCursor(15, 250);
	tft.print("v1.0");

	tft.setTextColor(ST7796S_GREEN);
	tft.setTextSize(2);

	tft.setCursor(15, 270);
	tft.print("Range: ");
	tft.setCursor(15, 290);
	tft.print(range/1000);
	tft.println(" meters");

	tft.setCursor(330, 270);
	tft.print("Meters/Line: ");
	tft.setCursor(330, 290);
	tft.print((range/1000)/segs);
	tft.println(" meters");
}

uint8_t b;
bool updated = false;
unsigned long tref = 0; // to create a non blocking halt
void loop() {

	// draw black screen
	// tft.fillCircle(center[0], center[1], pixelMax, ST7796S_BLACK);

	// draw radar background
	tft.drawLine(center[0],center[1],pixelMax*cos(-angle-PI/2)+center[0],pixelMax*sin(-angle-PI/2)+center[1], ST7796S_GREEN);
	tft.drawLine(center[0],center[1],pixelMax*cos(angle-PI/2)+center[0],pixelMax*sin(angle-PI/2)+center[1], ST7796S_GREEN);
	for (int i = 1; i <= segs; i++) {
		drawArc(center[0], center[1], (-angle), (angle), i*(pixelMax/segs), 0.1, ST7796S_GREEN);
	}

	// collect data
	while (!compare_header(0xAAFF0300))
	{
		if (Serial.available()) b = Serial.read();
		insert_into_header(b);
	}

	// Code to insert data into our data buffer after header is valid
	AUTOFOR(i, DATA_SIZE + TAIL_SIZE) // Just iterate through our fetched data including tail check
	{
		if (Serial.available()) FRAME_DATA[i] = Serial.read();
	}
	
	// collect data

	target1_x = -sm_to_tc(FRAME_DATA[0] | FRAME_DATA[1] << 8);
	target1_y = (FRAME_DATA[2] | FRAME_DATA[3] << 8) & 0x7FFF;
	target1_res = (FRAME_DATA[6] | FRAME_DATA[7] << 8);

	target2_x = -sm_to_tc(FRAME_DATA[8] | FRAME_DATA[9] << 8);
	target2_y = (FRAME_DATA[10] | FRAME_DATA[11] << 8) & 0x7FFF;
	target2_res = (FRAME_DATA[14] | FRAME_DATA[15] << 8);

	target3_x = -sm_to_tc(FRAME_DATA[16] | FRAME_DATA[17] << 8);
	target3_y = (FRAME_DATA[18] | FRAME_DATA[19] << 8) & 0x7FFF;
	target3_res = (FRAME_DATA[22] | FRAME_DATA[23] << 8);

	// draw
	if (!updated) { // update the screen
		// tft.fillTriangle(center[0], center[1], center[0]-ceil(320.0*tan(angle)), 0, center[0]+ceil(320.0*tan(angle)), 0, ST7796S_BLACK);
		refresh();
		if ((target1_x & 0xFFFF) | (target1_y & 0xFFFF) | (target1_res & 0xFFFF)) {
			tft.fillCircle(center[0] + (int16_t)(target1_x * conv), center[1] - (int16_t)(target1_y * conv), (int16_t)(target1_res * conv), ST7796S_CYAN);
		}
		if ((target2_x & 0xFFFF) | (target2_y & 0xFFFF) | (target2_res & 0xFFFF)) {
			tft.fillCircle(center[0] + (int16_t)(target2_x * conv), center[1] - (int16_t)(target2_y * conv), (int16_t)(target2_res * conv), ST7796S_ORANGE);
		}
		if ((target3_x & 0xFFFF) | (target3_y & 0xFFFF) | (target3_res & 0xFFFF)) {
			tft.fillCircle(center[0] + (int16_t)(target3_x * conv), center[1] - (int16_t)(target3_y * conv), (int16_t)(target3_res * conv), ST7796S_MAGENTA);
		}
		updated = true;
		tref = millis();
	}
	if ((millis()-tref) >= refresht) {
		updated = false;
	}

	// Reset header buffer after we allegedly capture frame
	// We used to check if the tail was valid, but we suspect some targets are omitted
	memset(HEADER_BUF, 0, HEADER_SIZE);
}
