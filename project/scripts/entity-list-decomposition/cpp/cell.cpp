#include "../include/cell.h"
using namespace std;

Cell::Cell(EntityType _type, int _index) 
  : type(_type), index(_index) {}

int Cell::get_index() const { return index; }

EntityType Cell::get_type() const { return type; }

void Cell::set_index(int _index) { index = _index; }

void Cell::set_type(EntityType _type) { type = _type; }
