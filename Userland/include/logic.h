#ifndef GOLF_LOGIC_H
#define GOLF_LOGIC_H

#include "golf.h"

int collide_circles(const circle_t *a, const circle_t *b);
void push_ball(const circle_t *pusher, circle_t * ball);
void resolve_overlap(const circle_t *fix, circle_t *mov);
void update_ball_motion(circle_t * ball, circle_t * player1, circle_t * player2, uint8_t two_players);
uint8_t update_position(circle_t *c);
void rotate_player(circle_t* player, float angle);
uint8_t clamp_circle_in_screen(circle_t *c);
void handle_player_collision(circle_t *player1, circle_t *player2, uint8_t player1_moving, uint8_t player2_moving);
// devuelve true si a esta dentro de b, 0 sino
uint8_t is_inside(const circle_t * a, const circle_t * b);
#endif