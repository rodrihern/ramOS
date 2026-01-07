
// A W D JUGADOR 1 
//J I L JUGADOR 2

#ifndef GOLF_H
#define GOLF_H

#include <stdint.h>
#include "usrlib.h"
#include "scancodes.h"

// -------------------------------------------------------------------------
// CONSTANTES DEL JUEGO


// #define SLOWDOWN_RATE 350000 // ralentizamos en un jugador para que el juego ande mejor
// Colores
#define BACKGROUND_COLOR_GOLF 0x135f3c  // Rich dark green
#define PLAYER1_COLOR 0x2196F3         // Bright blue - better visibility
#define PLAYER2_COLOR 0xFFC107         // Amber/gold - good contrast          
#define BALL_COLOR 0xFFFFFF            // White - like a real golf ball
#define HOLE_COLOR 0x000000            // black
#define PLAYER_OUTLINE_COLOR 0xFFFFFF  // White outline for players

// Radios
#define PLAYER_RADIUS 30
#define BALL_RADIUS 10
#define HOLE_RADIUS 20
#define HOLE_RADIUS_L2 15
#define HOLE_RADIUS_L3 11

// Velocidades
#define PLAYER_SPEED 0.015
#define BALL_INITIAL_SPEED 0.0
#define HOLE_SPEED 0.0

// aceleraciones (pixels per millisecond squared)
#define PLAYER_ACCELERATION 0.002  
#define BALL_MAX_SPEED 1
#define PLAYER_MAX_SPEED 0.35
#define BALL_FRICTION_DECELERATION 0.0004  // Friction coefficient (pixels per millisecond squared)
#define BALL_MIN_SPEED_THRESHOLD 0.001     // Minimum speed threshold before stopping completely

#define MASS_RATIO 2.25

// Direcciones iniciales
#define PLAYER1_INITIAL_DIR_X -1.0
#define PLAYER1_INITIAL_DIR_Y 0.0

#define INITIAL_DIR_X_ZERO 0.0
#define INITIAL_DIR_Y_ZERO 0.0

#define PLAYER2_INITIAL_DIR_X 1.0
#define PLAYER2_INITIAL_DIR_Y 0.0

//Ángulo de giro (radians per millisecond)
#define ROTATION_SPEED 0.00375
#define INITIALIZER 0



typedef struct {
    uint8_t left;
    uint8_t forward;
    uint8_t right;
} controls_t;

static const controls_t P1_KEYS = { KEY_A, KEY_W, KEY_D };   // jugador 1
static const controls_t P2_KEYS = { KEY_J, KEY_I, KEY_L  };   // jugador 2


// -------------------------------------------------------------------------
// DEFINICIONES Y ESTRUCTURAS
// -------------------------------------------------------------------------

// Estructura para posición (pixeles)
typedef struct {
    int x;
    int y;
} position_t;

// Estructura para dirección (versor)
typedef struct {
    double x;
    double y;
} direction_t;


// Estructura para círculos genéricos
typedef struct circle {
    position_t prev; // cordenadas anteriores para redibujar
    position_t pos; // coordenadas del centro del círculo
    uint32_t radius;
    double speed;  // modulo de velocidad
    direction_t dir; // versor de direccion
    uint32_t color;
    float rx; // acumulador de error fraccional del eje X
    float ry; // acumulador de error fraccional del eje Y
} circle_t;

// Bit-mask de bordes tocados
enum {
    EDGE_NONE   = 0,
    EDGE_LEFT   = 1 << 0, // 0001b
    EDGE_RIGHT  = 1 << 1, // 0010b
    EDGE_TOP    = 1 << 2, // 0100b
    EDGE_BOTTOM = 1 << 3  // 1000b
};

#define LEVELS          3

static const uint8_t HOLE_RADII[LEVELS] = {
        HOLE_RADIUS,
        HOLE_RADIUS_L2,    
        HOLE_RADIUS_L3     
};

/* --------------------------------------------------------- */


#endif