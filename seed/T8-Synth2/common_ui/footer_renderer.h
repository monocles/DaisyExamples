
#pragma once
#include "drivers/region.h"
#include "drivers/font.h"

namespace t8synth {

class FooterRenderer {
public:
    void DrawFooter(
        region& footer_region,
        const char* labelA,
        const char* labelB,
        const char* blockLabelA,
        const char* blockLabelB,
        int8_t value_a,
        int8_t value_b
    );

private:
    void DrawFooterSlider(region& footer_region);
    void DrawValuesAndBlocks(
        region& footer_region,
        const char* labelA,
        const char* labelB,
        const char* blockLabelA,
        const char* blockLabelB,
        int8_t value_a,
        int8_t value_b
    );
};

} // namespace t8synth