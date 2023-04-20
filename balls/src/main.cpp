/* $PSLibId: Run-time Library Release 4.4$ */
/*                  Fast balls:
 *
 *        Render multiple balls bouncing on the screen
 *
 *        Copyright  (C)  1993 by Sony Corporation
 *             All rights Reserved */

#include <sys/types.h>

#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <stdlib.h>


#include "balltex.h" // Ball texture data.

// Optimization toggles
#define OTENV // do PutDrawEnv and DrawOTag at the same time

// #define DEBUG

#define OTSIZE 1	// ordering table size
#define MAXOBJ 4000 // max sprite count

// Display size
#define FRAME_X 320
#define FRAME_Y 240

// Wall defs
#define WALL_X (FRAME_X - 16)
#define WALL_Y (FRAME_Y - 16)

// Types.
using u8 = unsigned char;
using s8 = char;
using u16 = unsigned short;
using s16 = short;
using u32 = unsigned int;
using s32 = int;
using u64 = unsigned long long;
using s64 = long long;
using usize = size_t;
using ssize = s32;

struct PrimitiveBuffer {
	/**
	 * Initalize the primitive buffer.
	 * \param[in] start Start address of buffer
	 * \param[in] size  Buffer size
	 */
	void Init(u8* start, usize size) {
		begin = start;
		end = start + size;
	}

	/**
	 * Allocates a set of primitive inside of this buffer.
	 * Returns a pointer to the allocated primitive, allowing it to be
	 * further modified after the fact.
	 */
	template<class T>
	constexpr T* AllocPrim(usize count = 1) {
		T* ptr = __builtin_bit_cast(T*, cur); // hacks are fun :)
		cur = cur + (sizeof(T) * count);
		return ptr;
	}

	/**
	 * Pull back the current pointer to the beginnign. Should be called after the frame is finished rendering
	 * (e.g: after DrawSync(), since that waits for the GPU to draw all primitives).
	 */
	void Reset() { cur = begin; }

   private:
	u8* begin;
	u8* end;
	u8* cur;
};

/** Drawing Buffer */
struct DrawBuffer {
	DRAWENV draw; ///< Drawing environment
	DISPENV disp; ///< Display environment.

	u_long ot[OTSIZE];
	//SPRT_16 sprt[MAXOBJ]; ///< Sprite data
	PrimitiveBuffer primBuf;

	u_short clut[32]; // texture CLUT entries

	template<class T>
	void AddPrimitive(T* primPtr) {
		::AddPrim(&ot[0], static_cast<void*>(primPtr));
	}

	// Initalize this draw buffer.
	void Init(u8* primBufAddr) {

		// set BG color
		draw.isbg = 1;
		setRGB0(&draw, 60, 120, 120);

		// load ball texture
		draw.tpage = LoadTPage(ball16x16, 0, 0, 640, 0, 16, 16);
#ifdef DEBUG
		DumpTPage(draw.tpage);
#endif

		// Load texture CLUT
		for(int i = 0; i < 32; i++) {
			clut[i] = LoadClut(ballcolor[i], 640, 480 + i);
#ifdef DEBUG
			DumpClut(clut[i]);
#endif
		}

		primBuf.Init(primBufAddr, 0x1ffff);

#if 0
		// Initialize things that never change for each sprite
		for(int i = 0; i < MAXOBJ; i++) {
			setSprt16(&sprt[i]);	   // set SPRT_16
			SetSemiTrans(&sprt[i], 1); // semi-ambient is ON
			SetShadeTex(&sprt[i], 1);  // shaded texture is OFF
			setUV0(&sprt[i], 0, 0);	   // texture uv is (0,0)
			(&sprt[i])->clut = clut[i % 32];
		}
#endif
	}
};

/*
 * Position Buffer*/
struct Ball {
	u8 clutIndex;
	u_short x, y;	/* current point*/
	u_short dx, dy; /* verocity*/

	SPRT_16* prim;

	inline Ball() {
		clutIndex = rand() % 32;
		x = rand();			   /* starting coordinate X*/
		y = rand();			   /* starting coordinate Y */
		dx = (rand() % 4) + 1; /* displacement distance X (1<=x<=4)  */
		dy = (rand() % 4) + 1; /* displacement distance Y (1<=y<=4)  */
	}
};

class BallsDemo {
	DrawBuffer db[2]; ///< The draw buffers
	DrawBuffer* cdb;  ///< Pointer to a given draw buffer
	int nobj;
	Ball objpos[MAXOBJ];
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
		db[0].Init(__builtin_bit_cast(u8*, 0x80020000));
		db[1].Init(__builtin_bit_cast(u8*, 0x80030000));

		// Allocate primitives
		AllocAndInitPrims();

		SetDispMask(1);
	}

	void Exit() {
		PadStop();
		StopCallback();
	}

	bool ShouldExit() const { return shouldExit; }

	void AllocAndInitPrimsOnDb(DrawBuffer& db) {
		db.primBuf.Reset();
		auto* prims = db.primBuf.AllocPrim<SPRT_16>(nobj);

		for(u32 i = 0; i < nobj; ++i) {
			setSprt16(&prims[i]);	   // set SPRT_16
			SetSemiTrans(&prims[i], 1); // semi-ambient is ON
			SetShadeTex(&prims[i], 1);  // shaded texture is OFF
			setUV0(&prims[i], 0, 0);	   // texture uv is (0,0)
			(&prims[i])->clut = cdb->clut[objpos[i].clutIndex];

			// Assign the primitive to the ball.
			objpos[i].prim = &prims[i];
		}
	}

	void AllocAndInitPrims() {
		AllocAndInitPrimsOnDb(db[0]);
		AllocAndInitPrimsOnDb(db[1]);
	}

	void Frame() {
		this->ReadPad();

		// Swap the current draw buffer
		cdb = (cdb == &db[0]) ? &db[1] : &db[0];

		// Reset the primitive buffer to the start again, so we don't accidentally overrun
		cdb->primBuf.Reset();

#ifdef DEBUG
		// dump draw buffer stuff
		DumpDrawEnv(&cdb->draw);
		DumpDispEnv(&cdb->disp);
		DumpTPage(cdb->draw.tpage);
#endif

		// Clear the current ordering table
		ClearOTag(&cdb->ot[0], OTSIZE);

		// update sprites
		for(int i = 0; i < nobj; i++) {
			int x;
			int y;

			if((x = ((&objpos[i])->x += (&objpos[i])->dx) % WALL_X * 2) >= WALL_X)
				x = WALL_X * 2 - x;
			if((y = ((&objpos[i])->y += (&objpos[i])->dy) % WALL_Y * 2) >= WALL_Y)
				y = WALL_Y * 2 - y;

			setXY0(objpos[i].prim, x, y);
			cdb->AddPrimitive(objpos[i].prim);
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
		auto old_nobj = nobj;

		if(padd & PADLup)
			nobj += 10;

		if(padd & PADLdown)
			nobj -= 5;

		if(padd & PADL1)
			while(PadRead(1) & PADL1)
				;

		if(padd & PADselect) {
			shouldExit = true;
			return;
		}

		limitRange(nobj, 1, MAXOBJ - 1); // set n to 1<=n<= (MAXOBJ-1). see libgpu.h

		// Re-allocate primitives.
		if(old_nobj != nobj) {
			AllocAndInitPrims();
		}
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
