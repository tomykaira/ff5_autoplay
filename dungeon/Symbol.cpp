#include "dungeon.hpp"
#include "dungeon/Map.hpp"
#include "dungeon/Symbol.hpp"

namespace dungeon {

Symbol::Symbol(Map * container) :
  visited(false),
  container(container)
{
}

void Link::visit()
{
	destination = new Map(container->groundTemplate);
}

}
