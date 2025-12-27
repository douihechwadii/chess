#ifndef MOVE_H
#define MOVE_H

typedef enum
{
    MOVE_NONE        = 0,
    MOVE_CAPTURE     = 1 << 0,
    MOVE_PROMOTION   = 1 << 1,
    MOVE_EN_PASSANT  = 1 << 2,
    MOVE_CASTLING    = 1 << 3

} MoveFlag;

typedef struct 
{
    int from;
    int to;

    int flags;

    // what changed in the moved piece
    int promotion;
    
    // what got captured during the move
    int captured;
} Move;


#endif