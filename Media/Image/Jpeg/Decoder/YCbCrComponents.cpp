#pragma once
#include "YCbCrComponents.h"
#include <cassert>
#include <vector>
#include "ComponentInfo.h"
#include "Image/Jpeg/Syntax/Segment.h"

namespace RagiMagick2::Image::Jpeg
{
    YCbCrComponents::YCbCrComponents(const Syntax::SOF0& sof0)
    {
        sampleWidth = sof0.width;
        sampleHeight = sof0.height;

        auto componentSelector = [&](Syntax::ComponentID id) -> ComponentInfo& {
            switch (id) {
            case Syntax::ComponentID::Y:
                return y;
            case Syntax::ComponentID::Cb:
                return cb;
            case Syntax::ComponentID::Cr:
                return cr;
            default:
                assert(false);
                return y; // dummy
            }
        };

        for (const auto& component : sof0.components) {
            assert(
                component.id == Syntax::ComponentID::Y  ||
                component.id == Syntax::ComponentID::Cb ||
                component.id == Syntax::ComponentID::Cr
            );
            auto& info = componentSelector(component.id);
            info.horizontalSamplingFactor = component.horizonalSamplingFactor;
            info.verticalSamplingFactor = component.verticalSamplingFactor;
        }

        for (const auto& component : sof0.components) {
            auto& info = componentSelector(component.id);
            const auto hMaxFactor = getMaxHorizontalSamplingFactor();
            const auto vMaxFactor = getMaxVerticalSamplingFactor();
            info.width = (sof0.width * info.horizontalSamplingFactor + hMaxFactor - 1) / hMaxFactor;
            info.height = (sof0.height * info.verticalSamplingFactor + vMaxFactor - 1) / vMaxFactor;
            info.buffer.resize(static_cast<size_t>(info.width * info.height), 0);
        }
    }
} // namespace RagiMagick2::Image::Jpeg
