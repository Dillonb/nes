#pragma once
#include "ppu.h"
#include "controller.h"
#include "movie.h"

void movie_update_button_states();
void set_movie_mode(movie m_to_play);
void render_screen(ppu_memory* ppu_mem);
bool get_button(button btn, player p);
