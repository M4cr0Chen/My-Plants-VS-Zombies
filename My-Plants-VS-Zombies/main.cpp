/*
* Development Log
* 1. Create new project (empty project template) using any version of Visual Studio
* 2. Import materials
* 3. Implement primary game scene
* 4. Implement the tool bar at the top of the game panel
* 5. Implement the plant cards in the tool bar
*/

#include <stdio.h>
#include <graphics.h> // easyX graphics library
#include <time.h>
#include "tools.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum {Peashooter, Sunflower, Plant_Count}; 
// Set Plant_Count to the end of the enum, its array index, which is n+1 is 
// exactly how many plants in the enum.



IMAGE imgBg; // variable for background image
IMAGE imgBar; // variable for plant choose bar image
IMAGE imgCards[Plant_Count];
IMAGE* imgPlants[Plant_Count][20];

int curX, curY; // coordinate during dragging of the currently selected plant
int curPlant; // 0: For not selecting, 1: Selecting first plant.

struct Plant {
	int type, frameIndex;
};

struct Plant map[3][9];

struct sunshineBall {
	int x, y; // Coordinates for the X and Y Coord of sunshine when falling
	int frameIndex; // Index for the current displaying picture
	int destY; // Y-Coord for the destination of the falling sunshine
	bool used; // Using or not
	int timer; //
};

struct sunshineBall balls[10];

IMAGE imgSunshineBall[29];
int sunshine;


bool fileExist(const char* name) {
	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}

// gameInit function initialize the game
void gameInit() {
	// Loading Background Image
	// Change the charset from "Unicode Charset" to "Multi-Bytes Charsets"
	loadimage(&imgBg, "res/bg.jpg"); 
	loadimage(&imgBar, "res/bar5.png");

	memset(imgPlants, 0, sizeof(imgPlants));
	memset(map, 0, sizeof(map));

	// Initialize Plant Cards
	char name[64];
	for (int i = 0; i < Plant_Count; i++) {
		// Generate File names for plants
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name); // name, address

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/plants_animations/%d/%d.png", i, j + 1);
			// Justify if file exist
			if (fileExist(name)) {
				imgPlants[i][j] = new IMAGE;
				loadimage(imgPlants[i][j], name);
			}
			else {
				break;
			}
		}
	}
	curPlant = 1;
	sunshine = 50;

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	// random seed
	srand(time(NULL));

	// Create Game Graphical Window
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	// Setup font
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWidth = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY; // Font Anti-aliasing Effect

	settextstyle(&f); // Set the font
	setbkmode(TRANSPARENT); // Set Background Mode: Transparent
	setcolor(BLACK);
}

// render the background image when initializing
void updateWindow() {
	BeginBatchDraw(); // Start double buffering

	putimage(0, 0, &imgBg); // putimage(leftup_x, leftup_y, IMAGE);
	//putimage(250, 0, &imgBar);
	putimagePNG(250, 0, &imgBar); //putimagePNG removes the black edges of the PNG
	
	// iteratively render the plant cards one-by-one
	for (int i = 0; i < Plant_Count; i++) {
		int x = 338 + i * 65;
		// 338 -> x value of the first card | 65 is width of each card img

		int y = 6;
		putimage(x, y, &imgCards[i]);
	}

	
	// Render plants putted to the block
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				int x = 256 + j * 81;
				int y = 179 + i * 102 + 10;
				int PlantType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				putimagePNG(x, y, imgPlants[PlantType][index]);
			}
		}
	}

	// Render plants card during dragging
	if (curPlant > 0) {
		IMAGE* img = imgPlants[curPlant - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	// Render the falling Sunshine
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].x, balls[i].y, img);
		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(280, 67, scoreText); // Output Score

	EndBatchDraw(); // End double buffering
}

void collectSunshine(ExMessage* msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].x;
			int y = balls[i].y;
			if (msg->x > x && msg->x < x + w &&
				msg->y > y && msg->y < y + h) {
				balls[i].used = false;
				sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);
			}
		}
	}
}

void userClick() {
	ExMessage msg;
	static int status = 0; // plant selection status
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338 && msg.x < 388 + 65 * Plant_Count && msg.y < 96) {
				int index = (msg.x - 338) / 65;
				status = 1;
				curPlant = index + 1;
			}
			else {
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {
			if (msg.x > 255 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;

				if (map[row][col].type == 0) {
					map[row][col].type = curPlant;
					map[row][col].frameIndex = 0;
				}
			}

			curPlant = 0;
			status = 0;
		}
	}
}

void createSunshine() {
	static int count = 0;
	static int freq = 400;
	count++;
	if (count >= freq) {
		freq = 200 + rand() % 200;
		count = 0;

		// Get one available sunshine from the pool
		int ballMax = sizeof(balls) / sizeof(balls[0]);

		int i = 0;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax) return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].x = 260 + rand() % (900 - 260); // 260 <-> 900
		balls[i].y = 60;
		balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;
	}	
}

void updateSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29; // Update new animation frame

			if (balls[i].timer == 0) {
				balls[i].y += 2; // Update Y-coordinates
			}

			// Stop at destY
			if (balls[i].y >= balls[i].destY) {
				// balls[i].used = false; Disappear Sunshine Code
				balls[i].timer++;
				if (balls[i].timer > 300) {
					balls[i].used = false;
				}
			}
		}
	}
}

void updateGame() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				map[i][j].frameIndex++;
				int PlantType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgPlants[PlantType][index] == NULL) {
					map[i][j].frameIndex = 0;
				}
			}
		}
	}

	createSunshine(); // Generate Sunshine
	updateSunshine(); // Update Sunshine Status
}


void startUI() {
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");

	int flag = 0;
	int click = 0;

	while (1) {
		BeginBatchDraw();

		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);

		

		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN &&
					msg.x > 474 && msg.x < 474 + 300 && msg.y > 75 && msg.y < 75 + 140) {
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP && flag) {
				return;
			}
		}

		EndBatchDraw();
	}
}


// main function of the program
int main(void) {
	gameInit(); // initialize game

	startUI(); // intialize start UI

	int timer = 0;
	bool flag = true;
	while (1) {
		userClick(); // deal with user's inputs
		timer += getDelay();
		if (timer > 30) {
			flag = true;
			timer = 0;
		}

		if (flag) {
			flag = false;
			updateWindow(); // render the images using putimage
			updateGame();
		}
		
	}
	

	system("pause");
	return 0;
}
