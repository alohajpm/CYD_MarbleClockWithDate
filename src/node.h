#ifndef NODE_H
#define NODE_H

#include "Point2D.h"

struct Node {
    Point2D coord;
    char content; // ' ' = empty, 'X'=foreground ball, '.'=background ball, '!'=Valve (stops the balls from continuing along the path)
};

// class Path {
//     public:
//     static const int MAX_SIZE = 100; // Adjust as needed

//     Path() {
//         _length = 0;
//     }

//     int legth();
//     int append(Node node);

// private:
//     Node nodes[MAX_SIZE];
//     int _length;

//     int Path::legth() {return _length;};

//     int Path::append(Node node)
//     {
//         nodes[_length++] = node;
//     }
// };

#endif