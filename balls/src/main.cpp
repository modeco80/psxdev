/* $PSLibId: Run-time Library Release 4.4$ */
/*                  Fast balls:
 *
 *        Render multiple balls bouncing on the screen
 *
 *        Copyright  (C)  1993 by Sony Corporation
 *             All rights Reserved */

#include <sys/types.h>
#include <stdlib.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#include "balltex.h" // Ball texture data.

// Optimization toggles
#define OTENV				// do PutDrawEnv and DrawOTag at the same time


//#define DEBUG

#define OTSIZE		1		// ordering table size
#define MAXOBJ		4000	// max sprite count

// Display size
#define	FRAME_X		320
#define	FRAME_Y		240

// Wall defs
#define WALL_X		(FRAME_X-16)
#define WALL_Y		(FRAME_Y-16)

/** Drawing/Primitive Buffer */
struct DrawBuffer {
	DRAWENV	draw; ///< Drawing environment
	DISPENV	disp; ///< Display environment.
	
	u_long ot[OTSIZE];
	SPRT_16 sprt[MAXOBJ]; ///< Sprite data
	
	// Initalize this draw buffer.
	void Init();
};

/*
 * Position Buffer*/
struct POS {
	u_short x, y;			/* current point*/
	u_short dx, dy;			/* verocity*/
	
	POS() {
		x  = rand();		/* starting coordinate X*/
		y  = rand();		/* starting coordinate Y */
		dx = (rand() % 4) + 1;	/* displacement distance X (1<=x<=4)  */
		dy = (rand() % 4) + 1;	/* displacement distance Y (1<=y<=4)  */
	}
};

void DrawBuffer::Init() {
	u_short	clut[32]; // texture CLUT entries

	// set BG color
	draw.isbg = 1;
	setRGB0(&draw, 60, 120, 120);

	// load ball texture
	draw.tpage = LoadTPage(ball16x16, 0, 0, 640, 0, 16, 16);
#ifdef DEBUG
	DumpTPage(draw.tpage);
#endif

	// Load texture CLUT
	for (int i = 0; i < 32; i++) {
		clut[i] = LoadClut(ballcolor[i], 640, 480+i);
#ifdef DEBUG
		DumpClut(clut[i]);
#endif
	}

	// Initialize things that never change for each sprite
	for (int i = 0; i < MAXOBJ; i++) {
		SetSprt16(&sprt[i]); 		// set SPRT_16
		SetSemiTrans(&sprt[i], 1);	// semi-ambient is ON
		SetShadeTex(&sprt[i], 1);	// shaded texture is OFF
		setUV0(&sprt[i], 0, 0); 	// texture uv is (0,0)
		(&sprt[i])->clut = clut[i % 32];
	}
}

class BallsDemo {
	DrawBuffer db[2]; ///< The draw buffers
	DrawBuffer* cdb; ///< Pointer to a given draw buffer
	int nobj;
	POS objpos[MAXOBJ];
	bool shouldExit;

	static void OnVSync() {
		/* print absolute VSync count */
		FntPrint("V-BLNK(%d)\n", VSync(-1));
	}

public:

	BallsDemo() {
		// Do some rudimentary initalization.
		cdb = &db[0];
		shouldExit = false;
		nobj = 1;
	}

	void Init() {
		PadInit(0);
		ResetGraph(0);
		SetGraphDebug(0);
		
		VSyncCallback(BallsDemo::OnVSync);
		
		// Set up rendering/display environment for double buffering:
		// - when rendering (0, 0)  - (320,240), display (0,240) - (320,240) (db[0])
		// - when rendering (0,240) - (320,480), display (0,  0) - (320,240) (db[1])
		SetDefDrawEnv(&db[0].draw, 0, 0, FRAME_X, FRAME_Y);
		SetDefDispEnv(&db[0].disp, 0, FRAME_Y, FRAME_X, FRAME_Y);
		
		SetDefDrawEnv(&db[1].draw, 0, FRAME_Y, FRAME_X, FRAME_Y);
		SetDefDispEnv(&db[1].disp, 0, 0, FRAME_X, FRAME_Y);

		// Load basic font pattern into frame buffer and set its display locaiton
		FntLoad(960, 256);
		SetDumpFnt(FntOpen(16, 16, 256, 200, 0, 512));	
		

		// initialize drawing buffers
		db[0].Init();
		db[1].Init();

		SetDispMask(1);
	}
	
	void Exit() {
		PadStop();
		StopCallback();
	}
	
	bool ShouldExit() const { return shouldExit; }
	
	void Frame() {
		this->ReadPad();
		
		// Swap the current draw buffer
		cdb = (cdb == &db[0]) ? &db[1] : &db[0];
		
#ifdef DEBUG
		// dump draw buffer stuff
		DumpDrawEnv(&cdb->draw);
		DumpDispEnv(&cdb->disp);
		DumpTPage(cdb->draw.tpage);
#endif

		// Clear the current ordering table
		ClearOTag(&cdb->ot[0], OTSIZE);

		// update sprites
		for (int i = 0; i < nobj; i++) {
			int x;
			int y;
		
			if ((x = ((&objpos[i])->x += (&objpos[i])->dx) % WALL_X*2) >= WALL_X)
				x = WALL_X * 2 - x;
			if ((y = ((&objpos[i])->y += (&objpos[i])->dy) % WALL_Y*2) >= WALL_Y)
				y = WALL_Y * 2 - y;

			setXY0(&cdb->sprt[i], x, y);	
			
			// Append to current ordering table
			AddPrim(&cdb->ot[0], &cdb->sprt[i]);	
		}
		
		// Wait for end of drawing
		DrawSync(0);		
		PutDispEnv(&cdb->disp); 
		
		// update drawing environment
#ifdef OTENV
		DrawOTagEnv(cdb->ot, &cdb->draw); 
#else
		PutDrawEnv(&cdb->draw); 
		// render primitives entered in the OT
		DrawOTag(cdb->ot);	
#endif

#ifdef DEBUG
		DumpOTag(cdb->ot);
#endif

		// print the number of balls and the elapsed time
		FntPrint("sprite = %d\n", nobj);
		FntPrint("total time = %d\n", VSync(0));
		FntFlush(-1);
	}
		
	void ReadPad() {
		u_long padd = PadRead(1);

		if(padd & PADLup)
			nobj += 10;
		
		if(padd & PADLdown)
			nobj -= 5;

		if(padd & PADL1)
			while (PadRead(1) & PADL1);

		if(padd & PADselect) {
			shouldExit = true;
			return;
		}

		limitRange(nobj, 1, MAXOBJ-1);	// set n to 1<=n<= (MAXOBJ-1). see libgpu.h
	}
};

int main() {
	BallsDemo demo;
	demo.Init();
	
	// Run the main loop
	while(!demo.ShouldExit())
		demo.Frame();
	
	demo.Exit();
	return 0;
}

