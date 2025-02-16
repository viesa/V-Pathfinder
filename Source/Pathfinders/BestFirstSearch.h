#pragma once

#include "Pathfinder.h"

namespace Se
{
class BestFirstSearch : public Pathfinder
{
public:
	BestFirstSearch();

	void FindPath(int startUID, int goalUID) override;

private:
	std::deque<int> _checkingQueue;
};
}