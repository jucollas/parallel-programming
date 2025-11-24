#ifndef CELL_H
#define CELL_H

#include <bits/stdc++.h>
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
    int index;

public: 
    Cell(EntityType _type = EntityType::EMPTY, int _index = -1);

    int get_index() const ;
    EntityType get_type() const ;

    void set_index(int _index);
    void set_type(EntityType _type);
};

#endif // CELL_H