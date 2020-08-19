#include "strafe_helper_includes.h"
#include "../cgame/cg_local.h"


extern "C" {

void shi_drawFilledRectangle(const float x, const float y,
                             const float w, const float h, const vec4_t color)
{
	CG_FillRect(x, y, w, h, color);
}

static int getFontSizeFromScale(int scale)
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

void shi_drawString(float x, float y, const char* string, float scale, const vec4_t color)
{
	CG_DrawProportionalString(
		x, y, string, getFontSizeFromScale(static_cast<int>(scale)) | CG_CENTER, color);
}

} // extern "C"
