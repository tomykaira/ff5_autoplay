#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "dungeon.hpp"
#include "dungeon/Map.hpp"

namespace dungeon {

class Symbol
{
protected:
	bool visited;
	Map * container;

public:
	Symbol(Map * container);

	virtual bool isVisited() const {
		return visited;
	}

	virtual void visit() {
		visited = true;
	}

	virtual bool isUnknown() const {
		return true;
	}

	virtual std::string character() const {
		return " ";
	}
};

class Floor : public Symbol
{
public:
	Floor(Map * container) :
		Symbol(container)
	{
	}

	bool isUnknown() const {
		return false;
	}

	std::string character() const {
		return "_";
	}
};

class Link : public Symbol
{
	Map * destination;

public:
	Link(Map * container) :
		Symbol(container),
		destination(NULL)
	{
	}

	bool isUnknown() const {
		return false;
	}

	bool isVisited() const {
		assert((visited && destination) || ((!visited) && (!destination)));
		return visited;
	}

	void visit();

	Map getDestination() const {
		assert(destination);
		return *destination;
	}

	std::string character() const {
		return "*";
	}
};

}

#endif /* _SYMBOL_H_ */

#include <dungeon.hpp>
