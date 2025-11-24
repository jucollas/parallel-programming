#ifndef CELL_H
#define CELL_H

#include <bits/stdc++.h>
#include "entity.h"
using namespace std;

enum class EntityType : uint8_t {
    EMPTY = 0,
    ROCK  = 1,
    RABBIT = 2,
    FOX = 3
};

class Cell {
private:
    EntityType type;
    Entity* ent;
    //int index;

public: 
    Cell(EntityType _type = EntityType::EMPTY, Entity* _ent = nullptr);

    Entity* get_entity() const ;
    EntityType get_type() const ;

    void set_entity(Entity* _ent);
    void set_type(EntityType _type);
};

#endif // CELL_H