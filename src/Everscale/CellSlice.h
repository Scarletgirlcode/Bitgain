#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Cell.h"

namespace TW::Everscale {

class CellSlice {
public:
    const Cell* _Nonnull cell;
    uint16_t dataOffset = 0;

    explicit CellSlice(const Cell* _Nonnull cell) noexcept
        : cell(cell) {}

    uint32_t getNextU32();
    Data getNextBytes(uint8_t bytes);

private:
    void require(uint16_t bits) const;
};

} // namespace TW::Everscale
