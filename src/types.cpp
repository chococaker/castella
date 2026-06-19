#include "types.h"

#include <sstream>

namespace choco {
std::ostream& operator<<(std::ostream& os, const Color& color) {
    switch (color) {
    case Color::WHITE:
        os << "w";
        break;
    case Color::BLACK:
        os << "b";
        break;
    default:
        assert(0);
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    using enum Piece;

    switch (piece) {
    case KING:
        os << "k";
        break;
    case QUEEN:
        os << "q";
        break;
    case BISHOP:
        os << "b";
        break;
    case KNIGHT:
        os << "n";
        break;
    case ROOK:
        os << "r";
        break;
    case PAWN:
        os << "p";
        break;
    default:
        assert(0);
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const Position& pos) {
    assert(pos.isValid());

    os << static_cast<char>(pos.file() + 'a')
       << static_cast<char>(pos.rank() + '1');

    return os;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    os << move.from << move.to;
    if (move.promotionType != Piece::INVALID) os << move.promotionType;
    return os;
}

std::ostream& operator<<(std::ostream& os, const MoveList& moves) {
    std::stringstream ss;
    for (int i = 0; i < moves.size(); i++) {
        ss << moves[i] << " ";
    }

    std::string str = ss.str();
    if (!str.empty()) str.pop_back();

    os << str;

    return os;
}
} // namespace choco
