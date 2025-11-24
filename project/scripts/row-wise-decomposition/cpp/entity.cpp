#include "../include/entity.h"

Entity::Entity(int _x, int _y, int _local_proc, int _local_food)
    : local_proc(_local_proc),
      local_food(_local_food),
      x(_x),
      y(_y)
{}

int Entity::get_x() const { return x; }
int Entity::get_y() const { return y; }
int Entity::get_local_proc() const { return local_proc; }
int Entity::get_local_food() const { return local_food; }


void Entity::print() const {
    cout << "x : " << x << "  y : " << y
         << "  gen=" << local_gen
         << "  proc=" << local_proc
         << "  food=" << local_food
         << "\n";
}
