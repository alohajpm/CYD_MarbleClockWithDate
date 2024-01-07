#ifndef NODE_H
#define NODE_H

#include "Point2D.h"

struct Node {
    Point2D coord;
    char content; // ' ' = empty, 'X'=foreground ball, '.'=background ball, '!'=Valve (stops the balls from continuing along the path)
};

#endif