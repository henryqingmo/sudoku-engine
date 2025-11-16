#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>

#include "serialization.h"

using sudoku_engine::serialization::Puzzle;
using sudoku_engine::serialization::PuzzleLoader;

PuzzleLoader::PuzzleLoader(std::istream& file) : file(file) {
    read_header();
    read_index();
}

void PuzzleLoader::read_header() {
    static_assert(sizeof(uint32_t) == 4);

    uint8_t header[HEADER_SIZE];
    this->file.read(reinterpret_cast<char*>(header), HEADER_SIZE);
    if (!this->file)
        throw std::runtime_error("Failed to read header");

    // check magic
    uint32_t magic;
    std::memcpy(&magic, header, 4);
    if (magic != MAGIC)
        throw std::runtime_error("Invalid KS file magic");

    // check version
    uint8_t ver = header[4];
    if (ver != VERSION)
        throw std::runtime_error("Unsupported KS file version");

    // read count (uint32_t little-endian)
    uint32_t puzzle_count;
    std::memcpy(&puzzle_count, header + 8, 4);

    this->index_offsets = utils::ArrayVector<size_t>(puzzle_count);
}

void PuzzleLoader::read_index() {
    this->file.seekg(HEADER_SIZE, std::ios::beg);
    for (size_t i = 0; i < this->index_offsets.capacity(); ++i) {
        uint64_t offset = 0;
        this->file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        if (!this->file)
            throw std::runtime_error("Failed to read index entry");
        this->index_offsets.append(offset);
    }
}

std::unique_ptr<Puzzle> PuzzleLoader::load_puzzle(size_t index) {
    if (index >= this->puzzle_count())
        throw std::out_of_range("Puzzle index out of range");

    this->file.seekg(this->index_offsets.data()[index], std::ios::beg);

    uint32_t payload_len = 0;
    this->file.read(reinterpret_cast<char*>(&payload_len), sizeof(payload_len));
    if (!this->file)
        throw std::runtime_error("Failed to read puzzle payload length");

    if (payload_len < SOLUTION_SIZE + 1)
        throw std::runtime_error("Payload too short for solution + cages");

    std::vector<uint8_t> payload(payload_len);
    this->file.read(reinterpret_cast<char*>(payload.data()), payload.size());
    if (!this->file)
        throw std::runtime_error("Failed to read puzzle payload");

    const auto solution_span = std::span(payload.begin(), SOLUTION_SIZE);
    const auto cages_span = std::span(payload).subspan(SOLUTION_SIZE);

    size_t pos = 0;
    const uint8_t num_cages = cages_span[pos++];

    Puzzle* const puzzle = new Puzzle{
        .cages = utils::ArrayVector<BoardCage>(num_cages),
        .solution = BoardState<BoardCell>(
            BOARD_SIZE,
            std::vector<BoardCell>(solution_span.begin(), solution_span.end())
        )
    };

    for (uint8_t i = 0; i < num_cages; ++i) {
        if (pos + 2 > cages_span.size())
            throw std::runtime_error(
                "Unexpected end of payload while reading cage header"
            );

        const uint8_t cage_sum = cages_span[pos++];
        uint8_t cage_size = cages_span[pos++];

        std::vector<BoardPosition> cage_cells;
        cage_cells.reserve(cage_size);

        if (pos + cage_size > cages_span.size())
            throw std::runtime_error(
                "Unexpected end of payload while reading cage coords"
            );

        for (uint8_t j = 0; j < cage_size; ++j) {
            const uint8_t pair = cages_span[pos++];
            const uint8_t row = (pair >> 4) & 0x0F;
            const uint8_t col = pair & 0x0F;
            cage_cells.emplace_back(row, col);
        }

        puzzle->cages.append(BoardCage(cage_sum, std::move(cage_cells)));
    }

    return std::unique_ptr<Puzzle>(puzzle);
}
