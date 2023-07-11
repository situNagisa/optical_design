#pragma once

#include "NGS/extend/extend.h"
#include "NGS/math/math.h"
#include "NGS/embedded/embedded.h"
#if NGS_PLATFORM == NGS_WINDOWS
#include "NGS/nsw/NSW.h"
#elif NGS_PLATFORM == NGS_LINUX
#include "NGS/nsl/NSL.h"
#endif
#include "vision/vision.h"

#define OPT_BEGIN namespace opt{
#define OPT_END }

OPT_BEGIN

enum class Team {
	_1,
	_2,
};

using position_t = ngs::Point2i;
using position_id_t = ngs::uint32;

inline position_id_t ConvertPosition(position_t pos) {
	NGS_ASSERT(pos.x <= 0xFFFF && pos.y <= 0xFFFF);
	return pos.x << 16 | pos.y;
}
inline position_t ConvertPosition(position_id_t id) { return { id >> 16, id & 0xFFFF }; }

OPT_END

namespace opt_vision {
	inline vision::position_t Transform(const opt::position_t& position) {
		return { (vision::position_t::type)position.x, (vision::position_t::type)position.y };
	}
	inline opt::position_t Transform(const vision::position_t& position) {
		return { (opt::position_t::type)position.x, (opt::position_t::type)position.y };
	}
}