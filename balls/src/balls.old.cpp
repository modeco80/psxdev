/* $PSLibId: Run-time Library Release 4.4$ */
/*                  Fast balls:
 *
 *        Render multiple balls bouncing on the screen
 *
 *        Copyright  (C)  1993 by Sony Corporation
 *             All rights Reserved */
 
// Partial C++ refactor

#include <sys/types.h>
#include <stdlib.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

/*
 * definition for speed up */
#define FASTVBLNK	/* shorten VBLNK */
#define SMALLDRAWAREA	/* decrease drawing*/
#define OTENV		/* do PutDrawEnv and DrawOTag at the same time */

/*
 * Kaji Printf*/
//#define KANJI

/*#define DEBUG */
/*
 * Primitive Buffer*/
#define OTSIZE		1		/* size of ordering table */
#define MAXOBJ		4000		/* max sprite number */

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

/*
 * display area*/
#define	FRAME_X		320		/* frame size*/
#define	FRAME_Y		240
#define WALL_X		(FRAME_X-16)	/* reflection point */
#define WALL_Y		(FRAME_Y-16)

/*
 * Initialize drawing Primitives*/
#include "balltex.h" // Ball texture data.

void DrawBuffer::Init() {
	u_short	clut[32];		/* texture CLUT entry*/

	/* set bg color*/
	draw.isbg = 1;
	setRGB0(&draw, 60, 120, 120);

	/* load texture pattern*/
	draw.tpage = LoadTPage(ball16x16, 0, 0, 640, 0, 16, 16);
#ifdef DEBUG
	DumpTPage(draw.tpage);
#endif
	/* load texture CLUT*/
	for (int i = 0; i < 32; i++) {
		clut[i] = LoadClut(ballcolor[i], 640, 480+i);
#ifdef DEBUG
		DumpClut(clut[i]);
#endif
	}

	/* initialize sprite*/
	SPRT_16* sp = sprt;
	for (int i = 0; i < MAXOBJ; i++, sp++) {
		/* set SPRT_16*/
		SetSprt16(sp);		
		
		/* semi-ambient is ON */
		SetSemiTrans(sp, 1);	
		
		/* shaded texture is OFF*/
		SetShadeTex(sp, 1);	
		
		/* texture point is (0,0)*/
		setUV0(sp, 0, 0);	
		
		/* set CLUT*/
		sp->clut = clut[rand() % 32];	
	}
}

/*
 * Read control-pad*/
/* int n; number of sprites*/
static int pad_read(int	n) {
	u_long	padd;

	/* read controller*/
	padd = PadRead(1);

	if(padd & PADLup)	n += 4;		/* up arrow key on the left of the controller*/
	if(padd & PADLdown)	n -= 4;		/* left arrow key on the left of the controller*/

	if (padd & PADL1) 			/* pause */
		while (PadRead(1)&PADL1);

	if(padd & PADselect) 	return(-1);	/* end of program*/

	limitRange(n, 1, MAXOBJ-1);	/* set n to 1<=n<= (MAXOBJ-1) */
					/* see libgpu.h*/
	return(n);
}

static void cbvsync() {
	/* print absolute VSync count */
	FntPrint("V-BLNK(%d)\n", VSync(-1));	
}


int main() {
	/* buffer storing ball coordinates and displacement distance */
	POS	pos[MAXOBJ];	
	
	/* double buffer*/
	DrawBuffer db[2];		
	
	/* current double buffer*/
	DrawBuffer* cdb;

	/* object count */
	int	nobj = 1;	
	
	/* current OT/work buffers*/
	u_long	*ot;		
	SPRT_16	*sp;		/* work */
	POS	*pp;		/* work */
	int	i, cnt, x, y;	/* work */

	/* reset controller */
	PadInit(0);		
	
	/* reset graphics system  (0: cold, 1: warm)  */
	ResetGraph(0);		
	
	/* set debug mode  (0: off, 1: monitor, 2: dump)  */
	SetGraphDebug(0);	
	
	/* set up V-sync callback function */
	VSyncCallback(cbvsync);	

	/* set up rendering/display environment for double buffering 
      when rendering (0, 0) - (320,240), display (0,240) - (320,480) (db[0]) 
      when rendering (0,240) - (320,480), display (0,  0) - (320,240) (db[1])  */
	SetDefDrawEnv(&db[0].draw, 0, 0, FRAME_X, FRAME_Y);
	SetDefDrawEnv(&db[1].draw, 0, FRAME_Y, FRAME_X, FRAME_Y);
	SetDefDispEnv(&db[0].disp, 0, FRAME_Y, FRAME_X, FRAME_Y);
	SetDefDispEnv(&db[1].disp, 0, 0, FRAME_X, FRAME_Y);

	/* init font environment */
	
	/* load basic font pattern into frame buffer */
	FntLoad(960, 256);	
	
	/* set font display location */
	SetDumpFnt(FntOpen(16, 16, 256, 200, 0, 512));	

	/* initialize primitive buffer */
	db[0].Init();
	db[1].Init();
	
	/* enable to display*/
	SetDispMask(1);		/* 0:inhibit,1:enable */

	/* main loop*/
	while ((nobj = pad_read(nobj)) > 0) {
		/* swap double buffer ID*/
		cdb  = (cdb==db)? db+1: db;	
#ifdef DEBUG
		/* dump DB environment */
		DumpDrawEnv(&cdb->draw);
		DumpDispEnv(&cdb->disp);
		DumpTPage(cdb->draw.tpage);
#endif

 		/* clear ordering table*/
		ClearOTag(cdb->ot, OTSIZE);

		/* update sprites */
		ot = cdb->ot;
		sp = cdb->sprt;
		pp = pos;
		for (i = 0; i < nobj; i++, sp++, pp++) {
			/* detect reflection*/
			if ((x = (pp->x += pp->dx) % WALL_X*2) >= WALL_X)
				x = WALL_X*2 - x;
			if ((y = (pp->y += pp->dy) % WALL_Y*2) >= WALL_Y)
				y = WALL_Y*2 - y;

			/* update vertex*/
			setXY0(sp, x, y);	
			
			/* apend to OT*/
			AddPrim(ot, sp);	
		}
		/* wait for end of drawing*/
		DrawSync(0);		
		
		/* cnt = VSync(1);	/* check for count */
		/* cnt = VSync(2);	/* wait for V-BLNK (1/30) */
		cnt = VSync(0);		/* wait for V-BLNK (1/60) */
		/* swap double buffers*/
		/* update display environment*/
		PutDispEnv(&cdb->disp); 
		
		/* update drawing environment*/
#ifdef OTENV
		DrawOTagEnv(cdb->ot, &cdb->draw); 
#else /* OTENV */
		PutDrawEnv(&cdb->draw); 
		
		/* render primitives entered in the OT*/
		DrawOTag(cdb->ot);	
#endif /* OTENV */
#ifdef DEBUG
		DumpOTag(cdb->ot);
#endif
		/* print the number of balls and the elapsed time*/
		FntPrint("sprite = %d\n", nobj);
		FntPrint("total time = %d\n", cnt);
		FntFlush(-1);
	}
	PadStop();	/* close controller*/
	StopCallback();
	return 0;
}
