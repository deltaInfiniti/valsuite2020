#pragma once

#include<ValWindowManager.h>
class ValBreakout {
	/*
	quick and dirty breakout implimentation in ValSuite for giggles
	*/
public:
	unsigned short paddlex, paddley, paddlew;
    bool bricksb[255]; // is brick live
	unsigned short bricksx[255], bricksy[255];
	unsigned int bricksc[255]; // brick colors
	unsigned short brickslen = 0;
	unsigned short ballx, bally;
	short ballxmomentum, ballymomentum;
	std::mutex critical_sect;
	bool pause = false;
	int windowwidth = 0;
	int windowheight = 0;
	unsigned short ball_basespeed = 8;
	unsigned short blockwidth=25, blockheight = 17;

	void start(int numbricks,int rows, int width, int height) {
		windowwidth = width;
		windowheight = height;
		int rc = numbricks / rows;
		if (rc == 0) {
			cout << "can't start a game with < rows bricks\n";

			return;
		}
		int jump = width / rc;
		for (int n = 0; n < rows; n++) {
			for (int i = 0; i < rc;i++) {
				bricksx[(n * rc) + i] = i * jump + 10;
				bricksy[(n * rc) + i] = n * 22 + 30;
				//bricksc[(n * rc) + i] = VRGB_RAND;
				bricksc[(n * rc) + i] = G_VS_Rand3(16777215);
				bricksb[(n * rc) + i] = true;
			}
		}
		brickslen = numbricks;
		paddlew = 80;
		ballx = windowwidth / 2;
		paddlex = ballx - (paddlew / 2);
		paddley = (windowheight/5)*4;
		bally = (windowheight / 5)*3;
		ballxmomentum = ball_basespeed/2;
		ballymomentum = ball_basespeed / 2;
		pause = true;
	}


	void render(ValGFX* gfx) {
		gfx->makewhite();
		int b = 0;
		for (int i = 0; i < brickslen; i++) {
			if (!bricksb[i])continue;
			gfx->color_section(bricksx[i], bricksy[i], blockwidth, blockheight, bricksc[i]); //draw bricks
			gfx->draw_box3(bricksx[i], bricksy[i], bricksx[i] + blockwidth, bricksy[i] + blockheight, 4, 0, VRGB_BLACK); //draw bricks boarder
		}
		gfx->color_section(ballx, bally, 5, 5, VRGB_BLACK, 0); // draw ball
		gfx->color_section(paddlex, paddley, paddlew, 10, VRGB_RED); // draw paddle
	}
	
	unsigned short collision() {
		if (ballx < 10 || ballx > windowwidth - 10)return 2;
		if(bally < 10 || bally > windowheight - 10)return 1;
		if (dim_collision(paddlex, paddley, paddlew, 30, ballx, bally, 4, 4)) {
			ballymomentum = abs(ballymomentum)*-1;
			float wid = (paddlew / 2);
			float plc = paddlex + wid;
			float lst = abs(plc - ballx);
			lst /= wid;
			ballxmomentum = floor((float)ball_basespeed * lst);
			if (ballx < (paddlex + wid)) ballxmomentum *= -1;
			// take difference between middle of paddle and middle of ball
			// change mallxmomentum based on this
			return 0;
		}
		for (int i = 0; i < brickslen; i++) {
			if (!bricksb[i])continue;
			if (dim_collision(ballx, bally, 3, 3, bricksx[i], bricksy[i], blockwidth, blockheight)) {

				bricksb[i] = false;
				while (!bricksb[brickslen - 1])brickslen--; // attempt to slightly reduce the burden of future activities
				if (dim_collision(ballx - ballxmomentum, bally, 3, 3, bricksx[i], bricksy[i], blockwidth, blockheight))return 1;
				else return 2;

				
			}

		}
		return 0;
	}
};
void valbreakout_mouseprocess(void* vwind) {
	//cout << "in mouse processing breakout\n";
	ValWindow* wind = (ValWindow*)vwind;
	ValBreakout* brk = (ValBreakout*)wind->get_associated_data("breakout_");
	int x = wind->mouse_cap->x;
	int y = wind->mouse_cap->y;
	brk->critical_sect.lock();
	switch (wind->mouse_cap->type) {
	case VW_MOUSECAP_MOVE:
		if (x < 10)x = 10;
		if (x > brk->windowwidth - (brk->paddlew+10))x = brk->windowwidth - (brk->paddlew + 10);
		brk->paddlex = x;
		//brk->paddley = y;
		

		break;
	case VW_MOUSECAP_CLICK:
		brk->pause = !(brk->pause);
		break;



	}

	brk->critical_sect.unlock();
	// paddle follow mouseover
	// click pauses/unpauses

}

void valbreakout_renderthread(void* vwind) {
	ValWindow* wind = (ValWindow*)vwind;
	ValBreakout* brk = (ValBreakout*)wind->get_associated_data("breakout_");
	if (brk == NULL) {
		cout << "brk failed renderthread" <<  endl;
	}
	wind->mygfx.copy_buf();

	brk->render(&(wind->mygfx));
	wind->GFXdraw();
	int h = 0;
	while (1) {
		while (brk->pause)Sleep(50);
		brk->critical_sect.lock();
		brk->ballx += brk->ballxmomentum;
		brk->bally += brk->ballymomentum;

		h = brk->collision();
		if (h == 2)brk->ballxmomentum *= -1;
		else if (h == 1)brk->ballymomentum *= -1;
		
		brk->render(&(wind->mygfx));
		wind->GFXdraw();
		brk->critical_sect.unlock();

		Sleep(10);
	}

}
ValBreakout vbc;// make global?
mouse_capture_struct mscglob; // put in valbreakout
void ValBreakout_thread(void* a) {


	
	vbc.start(70,8,500,500);
	WindowMan.wait_for_splash();

	//ValWindow* wind = WindowMan.create_new_windowts("breakout test", 200, 200, 500, 500, 1);
	ValWindow* wind = WindowMan.new_window("breakout test", 200, 200, 500, 500, 1);
	wind->capture_mouse(&mscglob, valbreakout_mouseprocess);
	wind->associate_data(( void*)&vbc, "breakout_");
	Sleep(300); // wait for everything to finish getting ready
	
	_beginthread(valbreakout_renderthread, 0, (void*)wind);
}