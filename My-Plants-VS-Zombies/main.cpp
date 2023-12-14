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

	// Render plants card during dragging
	if (curPlant > 0) {
		IMAGE* img = imgPlants[curPlant - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY -img->getheight() / 2, img);
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

		}
	}
}


// main function of the program
int main(void) {
	gameInit(); // initialize game

	while (1) {
		userClick();

		updateWindow(); // render the images using putimage
	}
	

	system("pause");
	return 0;
}
