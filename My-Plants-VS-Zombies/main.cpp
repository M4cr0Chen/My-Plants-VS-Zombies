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
#include <math.h>
#include "tools.h"
#include "vector2.h"

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
	int x, y;
	bool caught;
	int health;
	int shooting; // Status of shooting or not, 0: There's 1 zombie ahead; >0: More than 1 zombies ahead

	int timer;
	int x, y;
};

struct Plant map[3][9];

enum {SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCE};

struct sunshineBall {
	int x, y; // Coordinates for the X and Y Coord of sunshine when falling
	int frameIndex; // Index for the current displaying picture
	int destY; // Y-Coord for the destination of the falling sunshine
	bool used; // Using or not
	int timer; //

	float xoff;
	float yoff;

	float t; // Bezier curve timespan
	vector2 p1, p2, p3, p4;
	vector2 pCur; // Current sunshineball's location
	float speed;
	int status;
};

struct sunshineBall balls[10];

IMAGE imgSunshineBall[29];
int sunshine;


struct zm {
	float x, y;
	int frameIndex;
	bool used;
	float speed;
	int row;
	int health;
	bool dead;
	bool eating;
};
struct zm zms[10];
IMAGE imgZM[22]; // zombie walking frames
IMAGE imgZMDead[10]; // zombie dead frames
IMAGE imgZMEat[21]; // zommbie eating frames


// Data Structure for the bullet
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;
	int frameIndex;
};

struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];

int bulletsCount = sizeof(bullets) / sizeof(bullets[0]);
int zombiesCount = sizeof(zms) / sizeof(zms[0]);


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



	// Initialize zombies data
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i+1);
		loadimage(&imgZM[i], name); 
	}

	// Initialize bullets
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	// Initialize bullets' frame image array
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[3].getwidth() * k,
			imgBulletBlast[3].getheight() * k, true);
	}

	// Initialize zombie dead images
	for (int i = 0; i < 10; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead_fall/%d.png", i + 1);
		loadimage(&imgZMDead[i], name);
	}

	// Initialize zombie eat images
	for (int i = 0; i < 21; i++) {
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}
}

void drawZM() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			// IMAGE* img = &imgZM[zms[i].frameIndex];
			// IMAGE* img = (zms[i].dead) ? imgZMDead : imgZM;
			IMAGE* img = NULL;
			if (zms[i].dead) img = imgZMDead;
			else if (zms[i].eating) img = imgZMEat;
			else img = imgZM;

			img += zms[i].frameIndex;

			putimagePNG(
				zms[i].x, 
				zms[i].y-img->getheight(), 
				img);
		}
	}
}

void drawBullets() {
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				// Update blast frameindex
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
		
	}
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
				map[i][j].x = 256 + j * 81;
				map[i][j].y = 179 + i * 102 + 10;
				// map[i][j].health = 100; // Initialize health
				int PlantType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				putimagePNG(map[i][j].x, map[i][j].y, imgPlants[PlantType][index]);
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
		if (balls[i].used || balls[i].xoff) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].x, balls[i].y, img);
		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(280, 67, scoreText); // Output Score


	drawZM(); // Render zombie movement

	drawBullets(); // Render flying bullets

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
				// sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);

				// Setup sunshineBall's xoff and yoff
				float destY = 0;
				float destX = 262;
				float angle = atan((y - destY) / (x - destX));
				balls[i].xoff = 4 * cos(angle);
				balls[i].yoff = 4 * sin(angle);
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
		balls[i].xoff = 0;
		balls[i].yoff = 0;

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
		} // sunshineBall flying after being collected
		else if (balls[i].xoff) {
			// Setup sunshineBall's xoff and yoff
			float destY = 0;
			float destX = 262;
			float angle = atan((balls[i].y - destY) / (balls[i].x - destX));
			balls[i].xoff = 10 * cos(angle);
			balls[i].yoff = 10 * sin(angle);

			balls[i].x -= balls[i].xoff;
			balls[i].y -= balls[i].yoff;
			if (balls[i].y < 0 || balls[i].x < 262) {
				balls[i].xoff = 0;
				balls[i].yoff = 0;
				sunshine += 25;
			}
		}
	}
}

void createZM() {
	static int zmFreq = 10;
	static int count = 0;
	count++;
	if (count > zmFreq) {
		count = 0;
		zmFreq = rand() % 300 + 300;

		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);

		for (i = 0; i < zmMax && zms[i].used; i++);

		if (i < zmMax) {
			memset(&zms[i], 0, sizeof(zms[i]));
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 1;
			zms[i].health = 100;
			zms[i].dead = false;
		}
	}
}

void updateZM() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	static int count = 0;

	count++;

	// Zombie's frequency each step to the other
	if (count >= 3) {
		count = 0;

		// Update zombies' location
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed; // Zombie's horizontal movement per step

				if (zms[i].x < 170) {
					printf("GAME OVER \n");
					MessageBox(NULL, "over", "over", 0); // Need further optimization
					exit(0); // Need further optimization
				}
			}
		}
	}

	static int zombieCount = 0;
	zombieCount++;

	// Zombie's animation speed
	if (zombieCount > 4) {
		zombieCount = 0;

		// Update zombie frameIndex
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				// Dying frames
				if (zms[i].dead) {
					zms[i].frameIndex++;

					if (zms[i].frameIndex >= 10) {
						zms[i].used = false;
					}
				}
				// Eating frames
				else if (zms[i].eating) {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				// Walking frames
				else {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}	
}

void shoot() {
	int lines[3] = { 0 };
	int zombiesCount = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);

	int dangerX = WIN_WIDTH - imgZM[0].getwidth();

	
	// Traverse all plants stored in map[3][9]
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			
			// If the plant is a peashooter
			if (map[i][j].type == Peashooter + 1 /*&& lines[i]*/ ) {

				map[i][j].shooting = 0; // Peashooter only shoot to the first zombie
				// Refresh the shooting when zombie died

				// Bug fixed Log: To solve the bug that peashooter will shoot even if the zombies are at the back of the peashooter, I added another for loop that traverses each zombie and find their current x position,
				//   and if any's x-coord < any peashooter's x-coord, then that peashooter will not shoot.
				// However, I noticed a bug here which is how each peashooter's shoots at a same frequency, but the shooting peas got multiplied by the amount of zombies ahead of that peashooter. i.e. the more zombies on a same line the
				//   faster each peashooter shoots. This is because the peashooter treats each zombie evenly, or in another sense for each single zombie peashooter shoots just as much as how it should shoot on a regular line.
				//   This is not appropriate as we expected peashooter to shoot at a constant speed no matter how many zombies there are ahead of it.
				// Bug is fixed by introducing a new field: int shooting. The peashooter only shoot when int shooting = 0, and for any other zombies after it since shooting++, they'll not fell into the case of shooting.
				// This make the code to have a same logic as it is before adding the for k loop, which restrict the for k loop to only running once.

				// Dealing zombies as a queue: Shooting them one by one.

				// Justify if there's a zombie ahead to shoot
				// Traverse all zombies stored in zmks[10]
				for (int k = 0; k < zombiesCount; k++) {
					// Attribute Zombie's lines[]
					if (zms[k].used // This zombie is spawned
						&& zms[k].x > map[i][j].x // This zombie is ahead of the plant
						&& zms[k].x < WIN_WIDTH // This zombie is within the screen
						/* && zms[i].x < dangerX
						&& zms[i].x > imgZM[0].getwidth()*/) {
						lines[zms[k].row] = 1; // This zombie is on the row to be shooted
					}

					// Justify shoot or not
					if (lines[i] // If there's a zombie on the row
						&& zms[k].x > map[i][j].x // and if the zombie is ahead of the certain plant
						&& zms[k].x < WIN_WIDTH
						&& map[i][j].shooting == 0 // Peashooter only shoot to the first zombie
						) {

						static int count = 0;
						count++;

						map[i][j].shooting++;  // Peashooter not allowed to shoot any else zombies

						// Shoot frequency
						if (count > 40) {
							count = 0;

							// Shooting bullet function below
							int k;
							for (k = 0; k < bulletMax && bullets[k].used; k++);
							if (k < bulletMax) {
								bullets[k].used = true;
								bullets[k].row = i;
								bullets[k].speed = 5;

								bullets[k].blast = false;
								bullets[k].frameIndex = 0;

								int plantX = 256 + j * 81;
								int plantY = 179 + i * 102 + 14;
								// Calculate x y coord of bullet when first shooted
								bullets[k].x = plantX + imgPlants[map[i][j].type - 1][0]->getwidth() - 10;
								bullets[k].y = plantY + 5;
							}
						}
					}
				}
			}
		}
	}
}

void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			// Update flying bullet before any collision
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}

			// After bullet collision with a zombie
			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;
				}
			}
		}
	}
}

void checkBulletToZm() {
	for (int i = 0; i < bulletsCount; i++) {
		// passes over non-qualified bullets in the pool
		if (bullets[i].used == false || bullets[i].blast) continue;

		for (int k = 0; k < zombiesCount; k++) {
			// passes over non-qualified zombies in the pool
			if (zms[k].used == false) continue;

			int x1 = zms[k].x + 80; // left border
			int x2 = zms[k].x + 110; // right border
			int x = bullets[i].x; // bullet's x-coord

			if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2) {
				zms[k].health -= 10;
				bullets[i].blast = true;
				bullets[i].speed = 0;

				if (zms[k].health <= 0) {
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
				}
				break;
			}
		}
	}
}

void checkZmToPlant() {
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zCount; i++) {
		if (zms[i].dead) continue;

		int row = zms[i].row;
		for (int k = 0; k < 9; k++) {
			// map[row][k].health = 100;
			if (map[row][k].type == 0) continue;

			int plantX = 256 + k * 81;
			int x1 = plantX + 10;
			int x2 = plantX + 60;
			int x3 = zms[i].x + 80;
			
			if (x3 > x1 && x3 < x2) {
				if (map[row][k].caught) {
					//zms[i].frameIndex++;
					//map[row][k].health -= 10; // Plant health deduction when been eating
					map[row][k].health++;
					if (map[row][k].health > 110) {
						map[row][k].type = 0;
						map[row][k].health = 0;
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;
					}
				}
				else {
					map[row][k].caught = true;
					map[row][k].health = 0; // Initialize plant's health
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;
				}
			}
		}
	}
}

void collisionDetection() {
	checkBulletToZm();
	checkZmToPlant();
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
	
	createZM(); // Generate Zombies
	updateZM(); // Update Zombies Status

	shoot(); // Shoot peas bullet
	updateBullets(); // Update bullet status

	collisionDetection(); // Detect collision of bullets and zombies
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
