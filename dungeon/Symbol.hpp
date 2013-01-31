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

	virtual bool isUnknown() const = 0;

	virtual std::string character() const = 0;

	virtual bool movable() const {
		return false;
	}
};

class Unseen : public Symbol
{
public:
	Unseen(Map * container) :
		Symbol(container)
	{
	};

	bool isUnknown() const {
		return true;
	}

	std::string character() const {
		return " ";
	}
};

class Unidentified : public Symbol
{
public:
	Unidentified(Map * container) :
		Symbol(container)
	{
	};

	bool isUnknown() const {
		return true;
	}

	std::string character() const {
		return "?";
	}
};

class Block : public Symbol
{
public:
	Block(Map * container) :
		Symbol(container)
	{
	};

	bool isVisited() const {
		return false;
	}

	bool isUnknown() const {
		return false;
	}

	std::string character() const {
		return "X";
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

	bool movable() const {
		return true;
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

	Map * getDestination() const {
		assert(destination);
		return destination;
	}

	std::string character() const {
		return "*";
	}

	bool movable() const {
		return true;
	}
};

enum DoorStatus {OPEN, CLOSED};

class Door : public Symbol
{
	DoorStatus status;

public:
	Door(Map * container, DoorStatus status) :
		Symbol(container),
		status(status)
	{
	}

	bool isUnknown() const {
		return false;
	}

	void open()
	{
		status = OPEN;
	}

	bool isOpen() const
	{
		return (status == OPEN);
	}

	void visit()
	{
		assert(false && "You cannot visit a door, replace it with Floor or Link");
	}

	bool movable() const {
		return true;
	}

	std::string character() const {
		if (status == OPEN)
			return "O";
		else
			return "C";
	}
};



}

#endif /* _SYMBOL_H_ */

#include <dungeon.hpp>
