#ifndef ENTITY_H
#define ENTITY_H

#include <iostream>
using namespace std;

class Entity {
private:
    int local_proc;
    int local_food;
    int local_gen;
    int x, y;

public:
    // Constructor con valores por defecto (CORRECTO AQUÍ)
    Entity(int _x = -1, int _y = -1,
           int _gen = 0, int _local_proc = 0, int _local_food = 0);

    int get_x() const;
    int get_y() const;
    int get_local_gen() const;
    int get_local_proc() const;
    int get_local_food() const;

    void set_local_food(int _local_food);
    void print() const;
};

#endif
