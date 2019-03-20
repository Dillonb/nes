#pragma once
#include "ppu.h"
#include "controller.h"

void render_screen(color (*screen)[256][240]);
bool get_button(button btn, player p);
