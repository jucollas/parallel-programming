#include "../include/cell.h"
using namespace std;

Cell::Cell(EntityType _type, Entity* _ent) 
  : type(_type), ent(_ent) {}

Entity* Cell::get_entity() const { return ent;}

EntityType Cell::get_type() const { return type; }

void Cell::set_entity(Entity* _ent) { ent = _ent; }

void Cell::set_type(EntityType _type) { type = _type; }
