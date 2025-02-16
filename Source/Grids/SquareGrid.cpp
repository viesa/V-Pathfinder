#include "Grids/SquareGrid.h"

namespace Se
{
SquareGrid::SquareGrid() :
	TraverseGrid("Square"),
	_noBoxes(50, 50)
{
	SquareGrid::GenerateGrid();
	SquareGrid::GenerateNodes();
	SquareGrid::CalculateNeighbors();
}

void SquareGrid::OnRender(Scene &scene)
{
	TraverseGrid::OnRender(scene);

	if ( _drawFlags & TraverseGridDrawFlag_Grid )
	{
		scene.Submit(_lineVA);
	}

	if ( _drawFlags & TraverseGridDrawFlag_Objects )
	{
		scene.Submit(_filledSquaresVA);
		scene.Submit(_filledEdgesVA);
	}
}

void SquareGrid::OnRenderTargetResize(const sf::Vector2f &size)
{
	const bool newSize = _renderTargetSize != size;
	if ( newSize )
	{
		const int boxesX = static_cast<int>(size.x) / 12;
		const int boxesY = std::round(size.y / size.x * static_cast<float>(boxesX));
		_noBoxes = { boxesX, boxesY };
		_filledSquares.clear();
		_filledSquaresVA.clear();
		_filledEdges.clear();
		_filledEdgesVA.clear();
	}

	TraverseGrid::OnRenderTargetResize(size);
}

void SquareGrid::ClearNodeColor(int uid)
{
	Debug::Assert(uid != -1, "Invalid uid");
	const auto findResult = _filledSquares.find(uid);
	if ( findResult != _filledSquares.end() )
	{
		const int VAIndex = findResult->second.VAIndex;
		_filledSquaresVA[VAIndex].color = sf::Color::Transparent;
		_filledSquaresVA[VAIndex + 1].color = sf::Color::Transparent;
		_filledSquaresVA[VAIndex + 2].color = sf::Color::Transparent;
		_filledSquaresVA[VAIndex + 3].color = sf::Color::Transparent;
	}
}

void SquareGrid::SetNodeColor(int uid, const sf::Color &color)
{
	Debug::Assert(uid != -1, "Invalid uid");
	if ( _filledSquares.find(uid) == _filledSquares.end() )
	{
		const auto &position = NodeByUid(uid).Position();
		const auto boxSize = BoxSize();
		const auto halfBoxSize = boxSize / 2.0f;

		const auto square = Square{ _filledSquaresVA.getVertexCount(), sf::FloatRect{ position, boxSize } };
		const auto &[index, shape] = square;

		_filledSquaresVA.append({ sf::Vector2f{ shape.left, shape.top } - halfBoxSize, color });
		_filledSquaresVA.append({ sf::Vector2f{ shape.left + boxSize.x, shape.top } - halfBoxSize, color });
		_filledSquaresVA.append({ sf::Vector2f{ shape.left + boxSize.x, shape.top + boxSize.y } - halfBoxSize, color });
		_filledSquaresVA.append({ sf::Vector2f{ shape.left, shape.top + boxSize.y } - halfBoxSize, color });

		_filledSquares.emplace(uid, square);
	}
	else
	{
		const int VAIndex = _filledSquares.at(uid).VAIndex;
		_filledSquaresVA[VAIndex].color = color;
		_filledSquaresVA[VAIndex + 1].color = color;
		_filledSquaresVA[VAIndex + 2].color = color;
		_filledSquaresVA[VAIndex + 3].color = color;
	}
}

void SquareGrid::ClearNodeEdgeColor(int fromUid, int toUid)
{
	Debug::Assert(fromUid != -1 && toUid != -1, "Invalid uid");
	const auto findResult = _filledEdges.find({ fromUid, toUid });
	if ( findResult != _filledEdges.end() )
	{
		const int VAIndex = findResult->second;
		_filledEdgesVA[VAIndex].color = sf::Color::Transparent;
		_filledEdgesVA[VAIndex + 1].color = sf::Color::Transparent;
		_filledEdgesVA[VAIndex + 2].color = sf::Color::Transparent;
		_filledEdgesVA[VAIndex + 3].color = sf::Color::Transparent;
	}
}

void SquareGrid::SetNodeEdgeColor(int fromUid, int toUid, const sf::Color &color)
{
	Debug::Assert(fromUid != -1 && toUid != -1, "Invalid uid");
	if ( _filledEdges.find({ fromUid, toUid }) == _filledEdges.end() )
	{
		const auto &firstPosition = NodeByUid(fromUid).Position();
		const auto &secondPosition = NodeByUid(toUid).Position();

		const auto boxSize = BoxSize();
		sf::Vector2f halfRectSize(boxSize.x / 2.0f, boxSize.y / 6.0f);
		halfRectSize.x += halfRectSize.y;

		const auto fromTo = secondPosition - firstPosition;
		const auto fromToNorm = VecUtils::Unit(fromTo);

		const auto firstLine = fromTo / 2.0f - fromToNorm * halfRectSize.y;
		const auto secondLine = fromTo / 2.0f + fromToNorm * halfRectSize.y;
		const auto perpendicularFromToNorm = VecUtils::Perpendicular(fromToNorm);

		const auto VAIndex = _filledEdgesVA.getVertexCount();

		_filledEdgesVA.append({ firstPosition + firstLine + perpendicularFromToNorm * halfRectSize.x,color });
		_filledEdgesVA.append({ firstPosition + firstLine - perpendicularFromToNorm * halfRectSize.x, color });
		_filledEdgesVA.append({ firstPosition + secondLine - perpendicularFromToNorm * halfRectSize.x , color });
		_filledEdgesVA.append({ firstPosition + secondLine + perpendicularFromToNorm * halfRectSize.x , color });

		_filledEdges.emplace(std::make_pair(fromUid, toUid), VAIndex);
	}
	else
	{
		const int VAIndex = _filledEdges.at({ fromUid, toUid });
		_filledEdgesVA[VAIndex].color = color;
		_filledEdgesVA[VAIndex + 1].color = color;
		_filledEdgesVA[VAIndex + 2].color = color;
		_filledEdgesVA[VAIndex + 3].color = color;
	}
}

void SquareGrid::GenerateGrid()
{
	const sf::Vector2f topLeft = _visRect.getPosition();
	const auto boxSize = BoxSize();
	const sf::Vector2f lineLength = _visRect.getSize();


	auto setupVertex = [this](int index, const sf::Vector2f &start, const sf::Vector2f &end)
	{
		_lineVA[index].position = start;
		_lineVA[index + 1].position = end;
		_lineVA[index].color = _gridColor;
		_lineVA[index + 1].color = _gridColor;
	};

	// Setup new line vertex array
	const sf::Vector2i noVertices(2 * _noBoxes.x + 2, 2 * _noBoxes.y + 2);
	_lineVA.clear();
	_lineVA.resize(noVertices.x + noVertices.y);

	for ( int i = 0; i < noVertices.x; i += 2 )
	{
		const sf::Vector2f start = topLeft + sf::Vector2f(static_cast<float>(i / 2) * boxSize.x, 0.0f);
		const sf::Vector2f end(start.x, start.y + lineLength.y);
		setupVertex(i, start, end);
	}

	for ( int i = 0; i < noVertices.y; i += 2 )
	{
		const sf::Vector2f start = topLeft + sf::Vector2f(0.0f, static_cast<float>(i / 2) * boxSize.y);
		const sf::Vector2f end(start.x + lineLength.x, start.y);
		setupVertex(i + noVertices.x, start, end);
	}
}

void SquareGrid::GenerateNodes()
{
	_nodes.clear();

	int uid = 0;
	const auto boxSize = BoxSize();
	const auto topLeft = sf::Vector2f(_visRect.left, _visRect.top) + sf::Vector2f(boxSize.x, boxSize.y) / 2.0f;

	for ( int i = 0; i < _noBoxes.y; i++ )
	{
		for ( int j = 0; j < _noBoxes.x; j++ )
		{
			const sf::Vector2f position(topLeft.x + j * boxSize.x, topLeft.y + i * boxSize.y);
			_nodes.emplace(uid++, Node(uid, position));
		}
	}
}

void SquareGrid::CalculateNeighbors()
{
	const auto boxSize = BoxSize();
	const float diagonalLength = VecUtils::Length(boxSize);
	for ( int i = 0; i < _noBoxes.x * _noBoxes.y; i++ )
	{
		for ( int j = 0; j < 8; j++ )
		{
			if ( ((i % _noBoxes.x == 0) && j == 0) ||
				((i % _noBoxes.x == 0 || (i >= 0 && i < _noBoxes.x)) && j == 1) ||
				((i >= 0 && i < _noBoxes.x) && j == 2) ||
				(((i >= 0 && i < _noBoxes.x) || (i + 1) % _noBoxes.x == 0) && j == 3) ||
				(((i + 1) % _noBoxes.x == 0) && j == 4) ||
				(((i + 1) % _noBoxes.x == 0 || i >= _noBoxes.x * (_noBoxes.y - 1)) && j == 5) ||
				((i >= _noBoxes.x * (_noBoxes.y - 1) && (i <= (_noBoxes.x * _noBoxes.y))) && j == 6) ||
				((i >= _noBoxes.x * (_noBoxes.y - 1) || i % _noBoxes.x == 0) && j == 7) )
			{
				continue;
			}
			switch ( j )
			{
			case 0:
				NodeByUid(i).AddNeighbor(i - 1, boxSize.x);
				break;
			case 1:
				//GetNode(i).AddNeighbor(i - 1 - _noBoxes.x, diagonalLength);
				break;
			case 2:
				NodeByUid(i).AddNeighbor(i - _noBoxes.x, boxSize.y);
				break;
			case 3:
				//GetNode(i).AddNeighbor(i + 1 - _noBoxes.x, diagonalLength);
				break;
			case 4:
				NodeByUid(i).AddNeighbor(i + 1, boxSize.x);
				break;
			case 5:
				//GetNode(i).AddNeighbor(i + 1 + _noBoxes.x, diagonalLength);
				break;
			case 6:
				NodeByUid(i).AddNeighbor(i + _noBoxes.x, boxSize.y);
				break;
			case 7:
				//GetNode(i).AddNeighbor(i - 1 + _noBoxes.x, diagonalLength);
				break;
			default:
				break;
			}
		}
	}
}

auto SquareGrid::BoxSize() const -> sf::Vector2f
{
	return { _visRect.width / static_cast<float>(_noBoxes.x), _visRect.height / static_cast<float>(_noBoxes.y) };
}
}
