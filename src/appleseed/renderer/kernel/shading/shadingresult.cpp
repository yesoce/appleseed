
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "shadingresult.h"

// appleseed.foundation headers.
#include "foundation/utility/otherwise.h"

// Standard headers.
#include <cassert>
#include <cstddef>

using namespace foundation;

namespace renderer
{

//
// ShadingResult class implementation.
//

namespace
{
    inline Color3f spectrum_as_color3f(const Spectrum& s)
    {
        return Color3f(s[0], s[1], s[2]);
    }

    template <typename T>
    inline bool is_valid_scalar(const T x)
    {
        return x >= T(0.0);
    }

    template <typename T, size_t N>
    inline bool is_valid_color(const Color<T, N>& c)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!is_valid_scalar(c[i]))
                return false;
        }

        return true;
    }
}

bool ShadingResult::is_valid_linear_rgb() const
{
    assert(m_color_space == ColorSpaceLinearRGB);

    if (!is_valid_color(spectrum_as_color3f(m_main.m_color)))
        return false;

    if (!is_valid_color(m_main.m_alpha))
        return false;

    const size_t aov_count = m_aovs.size();

    for (size_t i = 0; i < aov_count; ++i)
    {
        const ShadingFragment& aov = m_aovs[i];

        if (!is_valid_color(spectrum_as_color3f(aov.m_color)))
            return false;

        if (!is_valid_color(aov.m_alpha))
            return false;
    }

    return true;
}

void ShadingResult::set_aovs_to_transparent_black_linear_rgba()
{
    const size_t aov_count = m_aovs.size();

    for (size_t i = 0; i < aov_count; ++i)
    {
        ShadingFragment& aov = m_aovs[i];
        aov.m_color[0] = 0.0f;
        aov.m_color[1] = 0.0f;
        aov.m_color[2] = 0.0f;
        aov.m_alpha.set(0.0f);
    }
}

namespace
{
    inline void transform_srgb_to_linear_rgb(Spectrum& s)
    {
        Color3f& linear_rgb = *reinterpret_cast<Color3f*>(&s[0]);
        linear_rgb = srgb_to_linear_rgb(Color3f(s[0], s[1], s[2]));
    }

    inline void transform_ciexyz_to_linear_rgb(Spectrum& s)
    {
        Color3f& linear_rgb = *reinterpret_cast<Color3f*>(&s[0]);
        linear_rgb = ciexyz_to_linear_rgb(Color3f(s[0], s[1], s[2]));
    }

    inline void transform_spectrum_to_linear_rgb(const LightingConditions& lighting, Spectrum& s)
    {
        Color3f& linear_rgb = *reinterpret_cast<Color3f*>(&s[0]);
        linear_rgb = ciexyz_to_linear_rgb(spectrum_to_ciexyz<float>(lighting, s));
    }
}

void ShadingResult::transform_to_linear_rgb(const LightingConditions& lighting)
{
    const size_t aov_count = m_aovs.size();

    switch (m_color_space)
    {
      case ColorSpaceLinearRGB:
        break;

      case ColorSpaceSRGB:
        transform_srgb_to_linear_rgb(m_main.m_color);
        for (size_t i = 0; i < aov_count; ++i)
            transform_srgb_to_linear_rgb(m_aovs[i].m_color);
        break;

      case ColorSpaceCIEXYZ:
        transform_ciexyz_to_linear_rgb(m_main.m_color);
        for (size_t i = 0; i < aov_count; ++i)
            transform_ciexyz_to_linear_rgb(m_aovs[i].m_color);
        break;

      case ColorSpaceSpectral:
        transform_spectrum_to_linear_rgb(lighting, m_main.m_color);
        for (size_t i = 0; i < aov_count; ++i)
            transform_spectrum_to_linear_rgb(lighting, m_aovs[i].m_color);
        break;

      assert_otherwise;
    }

    m_color_space = ColorSpaceLinearRGB;
}

namespace
{
    inline void transform_linear_rgb_to_spectrum(Spectrum& s)
    {
        linear_rgb_illuminance_to_spectrum(Color3f(s[0], s[1], s[2]), s);
    }

    inline void transform_srgb_to_spectrum(Spectrum& s)
    {
        linear_rgb_illuminance_to_spectrum(srgb_to_linear_rgb(Color3f(s[0], s[1], s[2])), s);
    }

    inline void transform_ciexyz_to_spectrum(Spectrum& s)
    {
        ciexyz_illuminance_to_spectrum(Color3f(s[0], s[1], s[2]), s);
    }
}

void ShadingResult::transform_to_spectrum(const LightingConditions& lighting)
{
    const size_t aov_count = m_aovs.size();

    switch (m_color_space)
    {
      case ColorSpaceLinearRGB:
        transform_linear_rgb_to_spectrum(m_main.m_color);
        for (size_t i = 0; i < aov_count; ++i)
            transform_linear_rgb_to_spectrum(m_aovs[i].m_color);
        break;

      case ColorSpaceSRGB:
        transform_srgb_to_spectrum(m_main.m_color);
        for (size_t i = 0; i < aov_count; ++i)
            transform_srgb_to_spectrum(m_aovs[i].m_color);
        break;

      case ColorSpaceCIEXYZ:
        transform_ciexyz_to_spectrum(m_main.m_color);
        for (size_t i = 0; i < aov_count; ++i)
            transform_ciexyz_to_spectrum(m_aovs[i].m_color);
        break;

      case ColorSpaceSpectral:
        break;

      assert_otherwise;
    }

    m_color_space = ColorSpaceSpectral;
}

void ShadingResult::composite_over_linear_rgb(const ShadingResult& background)
{
    //
    // Shading results use premultiplied alpha.
    //
    // References and interesting resources on alpha compositing:
    //
    //   http://keithp.com/~keithp/porterduff/p253-porter.pdf
    //   http://en.wikipedia.org/wiki/Alpha_compositing
    //   http://dvd-hq.info/alpha_matting.php
    //   http://my.opera.com/emoller/blog/2012/08/28/alpha-blending
    //

    assert(m_color_space == ColorSpaceLinearRGB);
    assert(background.m_color_space == ColorSpaceLinearRGB);

    const Alpha contrib = Alpha(1.0f) - m_main.m_alpha;
    m_main.m_color[0] += contrib[0] * background.m_main.m_color[0];
    m_main.m_color[1] += contrib[0] * background.m_main.m_color[1];
    m_main.m_color[2] += contrib[0] * background.m_main.m_color[2];
    m_main.m_alpha += contrib * background.m_main.m_alpha;

    const size_t aov_count = m_aovs.size();

    for (size_t i = 0; i < aov_count; ++i)
    {
        const ShadingFragment& background_aov = background.m_aovs[i];
        ShadingFragment& aov = m_aovs[i];

        const Alpha contrib = Alpha(1.0f) - aov.m_alpha;
        aov.m_color[0] += contrib[0] * background_aov.m_color[0];
        aov.m_color[1] += contrib[0] * background_aov.m_color[1];
        aov.m_color[2] += contrib[0] * background_aov.m_color[2];
        aov.m_alpha += contrib * background_aov.m_alpha;
    }
}

void ShadingResult::apply_alpha_premult_linear_rgb()
{
    assert(m_color_space == ColorSpaceLinearRGB);

    m_main.m_color[0] *= m_main.m_alpha[0];
    m_main.m_color[1] *= m_main.m_alpha[0];
    m_main.m_color[2] *= m_main.m_alpha[0];

    const size_t aov_count = m_aovs.size();

    for (size_t i = 0; i < aov_count; ++i)
    {
        ShadingFragment& aov = m_aovs[i];
        aov.m_color[0] *= aov.m_alpha[0];
        aov.m_color[1] *= aov.m_alpha[0];
        aov.m_color[2] *= aov.m_alpha[0];
    }
}

}   // namespace renderer
