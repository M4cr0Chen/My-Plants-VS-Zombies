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
#include "tools.h"

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
	// Create Game Graphical Window
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);
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

	EndBatchDraw(); // End double buffering
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
