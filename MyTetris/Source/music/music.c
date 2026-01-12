#include "LPC17xx.h"
#include "music.h"
#include "timer.h"

NOTE tetris_song[] = 
{
                // Main theme - Part A
        {e5, time_semiminima},
        {b4, time_croma},
        {c5, time_croma},
        {d5, time_semiminima},
        {c5, time_croma},
        {b4, time_croma},
        {a4, time_semiminima},
        {a4, time_croma},
        {c5, time_croma},
        {e5, time_semiminima},
        {d5, time_croma},
        {c5, time_croma},
        {b4, time_semiminima},
        {b4, time_croma},
        {c5, time_croma},
        {d5, time_semiminima},
        {e5, time_semiminima},
        {c5, time_semiminima},
        {a4, time_semiminima},
        {a4, time_semiminima},
        {pause, time_croma},
        
        // Part B
        {pause, time_croma},
        {d5, time_semiminima},
        {f5, time_croma},
        {a5, time_semiminima},
        {g5, time_croma},
        {f5, time_croma},
        {e5, time_semiminima + time_croma},
        {c5, time_croma},
        {e5, time_semiminima},
        {d5, time_croma},
        {c5, time_croma},
        {b4, time_semiminima},
        {b4, time_croma},
        {c5, time_croma},
        {d5, time_semiminima},
        {e5, time_semiminima},
        {c5, time_semiminima},
        {a4, time_semiminima},
        {a4, time_semiminima},
        {pause, time_semiminima},
        
        // Repeat Main theme - Part A
        {e5, time_semiminima},
        {b4, time_croma},
        {c5, time_croma},
        {d5, time_semiminima},
        {c5, time_croma},
        {b4, time_croma},
        {a4, time_semiminima},
        {a4, time_croma},
        {c5, time_croma},
        {e5, time_semiminima},
        {d5, time_croma},
        {c5, time_croma},
        {b4, time_semiminima},
        {b4, time_croma},
        {c5, time_croma},
        {d5, time_semiminima},
        {e5, time_semiminima},
        {c5, time_semiminima},
        {a4, time_semiminima},
        {a4, time_semiminima},
        {pause, time_croma},
};

const NOTE block_drop_effect[] = 
{
		{pause, time_croma},
		{pause, time_croma},
		{e5, time_croma},
		{g5, time_croma},
		{e5, time_croma},
		{c5, time_croma},
		{pause, time_croma},
		{pause, time_croma},
};

const NOTE line_clear_effect[] = 
{
		{pause, time_croma},
		{pause, time_croma},
		{c5, time_croma},
		{e5, time_croma},
		{g5, time_croma},
		{a5, time_semiminima},
		{pause, time_croma},
		{pause, time_croma},
};

const NOTE game_over_effect[] = 
{
		{pause, time_croma},
		{pause, time_croma},
		{e5, time_croma},
		{d5, time_croma},
		{c5, time_croma},
		{b4, time_croma},
		{a4, time_semiminima},
		{pause, time_croma},
		{pause, time_croma},
};

const SoundEffect NO_EFFECT = {0, 0};
const SoundEffect BLOCK_DROP_EFFECT = 
{
		block_drop_effect,
		sizeof(block_drop_effect) / sizeof(NOTE)
};
const SoundEffect LINE_CLEAR_EFFECT = 
{
		line_clear_effect,
		sizeof(line_clear_effect) / sizeof(NOTE)
};
const SoundEffect GAME_OVER_EFFECT = 
{
		game_over_effect,
		sizeof(game_over_effect) / sizeof(NOTE)
};


const SoundEffect TETRIS_MUSIC = 
{
	tetris_song,
	sizeof(tetris_song) / sizeof(NOTE)
};

void playNote(NOTE note)
{
	if(note.freq != pause)
	{
		reset_timer(0);
		init_timer(0, note.freq);
		enable_timer(0);
	}
	reset_timer(1);
	init_timer(1, note.duration);
	enable_timer(1);
}

BOOL isNotePlaying(void)
{
	return ((LPC_TIM0->TCR != 0) || (LPC_TIM1->TCR != 0));
}
SoundEffect playingEffect = NO_EFFECT;

void playEffect(SoundEffect effect) {
	playingEffect = effect;
}

