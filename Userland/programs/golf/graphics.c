#include "golf.h"
#include "logic.h"
#include "graphics.h"
#include "framebuffer.h"
#include "font.h"




framebuffer_t fb;
int width, height;

void init_graphics(uint16_t w, uint16_t h, uint16_t pitch, uint8_t bpp) {
    width = w;
    height = h;
    fb = fb_init(width, height, pitch, bpp);
}


uint16_t get_width() {
    return width;
}

uint16_t get_height() {
    return height;
}


void draw_background(uint32_t color) {
    fb_fill(fb, color);
} 


void draw_circle(circle_t *c) {
    fb_fill_circle(fb, c->pos.x, c->pos.y, c->radius, c->color);
}

void draw_player(circle_t *p){
    // cuerpo
    draw_circle(p);

    float dx = p->dir.x;
    float dy = p->dir.y;
    float len2 = dx*dx + dy*dy;          
    if (len2 == 0.0f) {                  
        dx = 1.0f; dy = 0.0f; len2 = 1.0f;
    }
    float invLen = inv_sqrt(len2);       
    float nx = dx * invLen;              
    float ny = dy * invLen;

    float px = -ny;                      
    float py =  nx;

    // flecha
    int arrowLength = p->radius * 2 / 3;    
    int arrowHeadSize = p->radius / 4;      
    int baseX = (int)(p->pos.x - nx * arrowLength / 2);
    int baseY = (int)(p->pos.y - ny * arrowLength / 2);
    int tipX = (int)(p->pos.x + nx * arrowLength / 2);
    int tipY = (int)(p->pos.y + ny * arrowLength / 2);
    
    fb_draw_line(fb, baseX, baseY, tipX, tipY, 0x000000);

    // puntas de la flecha
    int wing1X = (int)(tipX - nx * arrowHeadSize - px * arrowHeadSize / 2);
    int wing1Y = (int)(tipY - ny * arrowHeadSize - py * arrowHeadSize / 2);
    int wing2X = (int)(tipX - nx * arrowHeadSize + px * arrowHeadSize / 2);
    int wing2Y = (int)(tipY - ny * arrowHeadSize + py * arrowHeadSize / 2);
    
    fb_draw_line(fb, tipX, tipY, wing1X, wing1Y, 0x000000);
    fb_draw_line(fb, tipX, tipY, wing2X, wing2Y, 0x000000);

    // ojos
    int eyeDist   = p->radius / 2;
    int pupilOff  = p->radius / 4;
    int eyeRad    = 3;

    int lx = (int)(p->pos.x +  px * eyeDist);
    int ly = (int)(p->pos.y +  py * eyeDist);
    int rx = (int)(p->pos.x -  px * eyeDist);
    int ry = (int)(p->pos.y -  py * eyeDist);

    int pupX = (int)(nx * pupilOff);
    int pupY = (int)(ny * pupilOff);

    fb_fill_circle(fb, lx + pupX, ly + pupY, eyeRad, 0x000000);
    fb_fill_circle(fb, rx + pupX, ry + pupY, eyeRad, 0x000000);
}








// SCOREBOARD

void draw_scoreboard(uint8_t two_players, uint16_t score1, uint16_t score2, uint64_t touches) {
    fb_fill_height(fb, 0, SCOREBOARD_HEIGHT, MENU_BKG);
    
    // Solo dibujar "Touches: "
    fb_draw_string(fb, "Touches: ", font, 10, 0, 2, MENU_CONTENT);
    // número de touches
    char touch_buffer[32];
    uint64_t touch_len = num_to_str(touches, touch_buffer, 10);
    touch_buffer[touch_len] = '\0';
    int touches_number_x = 10 + (9 * 8 * 2); // 9 chars de "Touches: "
    fb_draw_string(fb, touch_buffer, font, touches_number_x, 0, 2, MENU_CONTENT);

    if (two_players) {
        int text_width = 11 * 8 * 2; // 11 chars, 8 pixels por char, size 2
        int score_x = width - text_width - 10; // 10 pixeles de margen
        
        // Texto base
        fb_draw_string(fb, "P1   -   P2", font, score_x, 0, 2, MENU_CONTENT);

        // dígitos del score (posiciones 3 y 7 en "P1 x - y P2")
        char score1_buffer[2];
        score1_buffer[0] = '0' + (score1 % 10);
        score1_buffer[1] = '\0';

        char score2_buffer[2];
        score2_buffer[0] = '0' + (score2 % 10);
        score2_buffer[1] = '\0';

        int score1_x = score_x + (3 * 8 * 2); // después de "P1 "
        fb_draw_string(fb, score1_buffer, font, score1_x, 0, 2, MENU_CONTENT);

        int score2_x = score_x + (7 * 8 * 2); // después de "P1 x - "
        fb_draw_string(fb, score2_buffer, font, score2_x, 0, 2, MENU_CONTENT);
    }
}



// PANTALLAS
int draw_menu_screen(void) {
    int num_players;

    
    fb_fill(fb, MENU_BKG);
    
    int title_y = height / 3;
    int instruction_y = height / 2;
    
    fb_draw_string(fb, "GOLF GAME", font, width/2 - 144, title_y, 4, MENU_CONTENT);
    fb_draw_string(fb, "Select number of players:", font, width/2 - 200, instruction_y, 2, MENU_CONTENT);
    fb_draw_string(fb, "Press 1 for Single Player (WAD)", font, width/2 - (30 * 8 * 2 / 2), instruction_y + 40, 2, MENU_CONTENT);
    fb_draw_string(fb, "Press 2 for Two Players (WAD + IJL)", font, width/2 - (33 * 8 * 2 / 2), instruction_y + 70, 2, MENU_CONTENT);
    fb_draw_string(fb, "Press Enter to quit the game at any time", font, width/2 - 168, instruction_y + 120, 1, 0x888888); // Gray color for less emphasis
    
    show_frame();

    do {
        num_players = getchar();  // bloq: NO sale hasta que se presione una tecla 
    } while (num_players != '1' && num_players != '2' && num_players != '\n'); // ignora cualquier tecla que no sea 1 o 2
    
    if (num_players == '\n') {
        return 0;
    }

    return num_players - '0'; // devuelve 1 o 2 como enteros
}

void draw_level_end_screen(uint8_t winner) {

    fb_fill(fb, MENU_BKG);
    
    int title_y = height / 3;
    int subtitle_y = height / 2;
    int instruction_y = height / 2 + 60;
    
    fb_draw_string(fb, "GOAL!", font, width/2 - 80, title_y, 4, MENU_CONTENT);
    
    if (winner == 1) {
        fb_draw_string(fb, "Player 1 scored!", font, width/2 - 192, subtitle_y, 3, MENU_CONTENT);
    } else {
        fb_draw_string(fb, "Player 2 scored!", font, width/2 - 192, subtitle_y, 3, MENU_CONTENT);
    }

    fb_draw_string(fb, "Press SPACE to continue", font, width/2 - 184, instruction_y, 2, MENU_CONTENT);
    
    show_frame();

    WAIT_SPACE();
}



static void build_score_string(char* dest, const char* player_name, uint16_t score) {
    const char* prefix = player_name;
    int i = 0;
    while (*prefix) {
        dest[i++] = *prefix++;
    }
    dest[i++] = ':';
    dest[i++] = ' ';
    
    char score_buffer[16];
    uint64_t score_len = num_to_str(score, score_buffer, 10);
    for (int j = 0; j < score_len; j++) {
        dest[i++] = score_buffer[j];
    }
    dest[i] = '\0';
}

void draw_final_score_screen(uint8_t two_players, uint16_t score_p1, uint16_t score_p2, uint64_t touches) {

    
    fb_fill(fb, MENU_BKG);
    
    int title_y = height / 4;
    int content_start_y = height / 2 - 60;
    int instruction_y = height / 2 + 120;
    
    if (two_players) {
        fb_draw_string(fb, "FINAL SCORE", font, width/2 - 176, title_y, 4, MENU_CONTENT);
        
        // Player 1 score
        char score_text[32];
        build_score_string(score_text, "Player 1", score_p1);
        int p1_text_width = strlen(score_text) * 8 * 3;
        fb_draw_string(fb, score_text, font, width/2 - p1_text_width/2, content_start_y, 3, MENU_CONTENT);
        
        build_score_string(score_text, "Player 2", score_p2);
        int p2_text_width = strlen(score_text) * 8 * 3;
        fb_draw_string(fb, score_text,  font, width/2 - p2_text_width/2, content_start_y + 50, 3, MENU_CONTENT);
        
        int winner_y = content_start_y + 100;
        if (score_p1 > score_p2) {
            fb_draw_string(fb, "Player 1 Wins!", font, width/2 - 168, winner_y, 3, 0x00FF00); // Gerde para el ganadorreen for winner
        } else if (score_p2 > score_p1) {
            fb_draw_string(fb, "Player 2 Wins!", font, width/2 - 168, winner_y, 3, 0x00FF00); // Verde para el ganador
        } else {
            fb_draw_string(fb, "It's a Tie!", font, width/2 - 132, winner_y, 3, 0xFFFF00); // Amarillo para empate
        }
        
        // toques
        char touches_text[32];
        build_score_string(touches_text, "Total Touches", (uint16_t)touches);
        int touches_text_width = strlen(touches_text) * 8 * 2;
        fb_draw_string(fb, touches_text, font, width/2 - touches_text_width/2, winner_y + 60, 2, MENU_CONTENT);
        
        instruction_y = winner_y + 100;
    } else {
        // solo mostramos toques
        fb_draw_string(fb, "GAME COMPLETE", font, width/2 - 208, title_y, 4, MENU_CONTENT);
        
        char touches_text[32];
        build_score_string(touches_text, "Total Touches", (uint16_t)touches);
        int touches_text_width = strlen(touches_text) * 8 * 4;
        fb_draw_string(fb, touches_text, font, width/2 - touches_text_width/2, content_start_y, 4, MENU_CONTENT);
        
        instruction_y = content_start_y + 80;
    }
    
    fb_draw_string(fb, "Press SPACE to return to shell", font, width/2 - 248, instruction_y, 2, MENU_CONTENT);
    
    show_frame();

    WAIT_SPACE();
}

void draw_countdown_screen(uint64_t size) {
    uint64_t x = width/2 - 4 * size;
    uint64_t y = height/2 - 8 * size;
    region_t region = {
        .x = x,
        .y = y,
        .height = 16*size,
        .width = 8*size
    };
    fb_fill(fb, BACKGROUND_COLOR_GOLF);
    fb_draw_string(fb, "3", font, x, y, size, 0xffffff);
    show_frame();
    sys_sleep(1000);

    fb_fill_rectangle(fb, x, y, x + 8*size - 1, y + 16*size - 1, BACKGROUND_COLOR_GOLF);
    fb_draw_string(fb, "2", font,  x, y, size, 0xffffff);
    fb_present_region(fb, &region);
    sys_sleep(1000);

    fb_fill_rectangle(fb, x, y, x + 8*size - 1, y + 16*size - 1, BACKGROUND_COLOR_GOLF);
    fb_draw_string(fb, "1", font, x, y, size, 0xffffff);
    fb_present_region(fb, &region);
    sys_sleep(1000);

    fb_fill_rectangle(fb, x, y, x + 8*size - 1, y + 16*size - 1, BACKGROUND_COLOR_GOLF);
    show_frame();
}



void show_frame() {
    fb_present(fb);
}