#pragma once
#include "util/config.h"
#include "util/debug.h"

struct shellstate_t{
    uint32_t num_keypresses;
    char heading[20]; 
    // Stores the contents of the shell -- atmost 500 lines and 80 chars
    char contents[500][80];
    // Points below the line to be printed
    int content_ptr;
    char command_text[80];
    // The pointer to the command text buffer
    int command_ptr;
};

struct renderstate_t{
    uint32_t num_keypresses;
    char heading[20];
    // Store the contents to be rendered
    char contents[24][80];
    char command_text[79];
};

void shell_init(shellstate_t& state);
void shell_update(uint8_t scankey, shellstate_t& stateinout);
void shell_step(shellstate_t& stateinout);
void shell_render(const shellstate_t& shell, renderstate_t& render);

bool render_eq(const renderstate_t& a, const renderstate_t& b);
void render(const renderstate_t& state, int w, int h, addr_t display_base);

