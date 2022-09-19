#include "strafe_helper/strafe_helper_customization.h"

#include "../cgame/cg_local.h"
#include "../cgame/cg_media.h"

#include <cassert>


namespace {

struct StrafeHelperColor {
    vec4_t data;
};

StrafeHelperColor getColorForElement(const shc_ElementId element_id)
{
	switch (element_id) {
	case shc_ElementId_AcceleratingAngles:
		return {{0.0f, 0.5f, 0.125f, 0.375f}};
	case shc_ElementId_OptimalAngle:
		return {{0.0f, 1.0f, 0.25f, 0.75f}};
	case shc_ElementId_CenterMarker:
		return {{1.0f, 1.0f, 1.0f, 0.75f}};
	case shc_ElementId_SpeedText:
		return {{1.0f, 1.0f, 1.0f, 0.9f}};
	}
	assert(false);
	return {{1.0, 1.0, 1.0, 1.0}};
}

int getFontSizeFromScale(int scale)
{
	switch (scale) {
	case 1:
		return CG_TINYFONT;
	case 2:
		return CG_SMALLFONT;
	case 3:
		return CG_BIGFONT;
	default:
		return 0;
	}
}

}  // anonymous namespace


extern "C" {

void shc_drawFilledRectangle(const float x, const float y,
                             const float w, const float h,
                             const shc_ElementId element_id)
{
	const auto color = getColorForElement(element_id);
	CG_FillRect(x, y, w, h, color.data);
}

void shc_drawString(float x, float y, const char* string,
                    float scale, const shc_ElementId element_id)
{
	const auto color = getColorForElement(element_id);
	CG_DrawProportionalString(
		x, y, string, getFontSizeFromScale(static_cast<int>(scale)) | CG_CENTER, color.data);
}

} // extern "C"
