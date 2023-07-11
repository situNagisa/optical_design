#pragma once

#include "NGS/math/math.h"
#include "NGS/extend/extend.h"
#include <opencv2/opencv.hpp>

namespace vision {
	using bounds_t = ngs::Rectanglef;
	using position_t = ngs::Point2f;
}

namespace ngs_cv {
	using bounds_t = ngs::Rectanglei;
	using position_t = ngs::Point2i;

	inline cv::Rect Convert(const bounds_t& rect) {
		return { rect.x,rect.y,rect.width,rect.height };
	}
	inline bounds_t Convert(const cv::Rect& rect) {
		return { rect.x, rect.y, rect.width, rect.height };
	}
	inline cv::Rect Transform(const vision::bounds_t& rect) {
		return { (int)rect.x,(int)rect.y,(int)rect.width,(int)rect.height };
	}
	inline vision::bounds_t Transform(const cv::Rect& rect) {
		return { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };
	}

	inline cv::Point Convert(const position_t& point) {
		return { point.x,point.y };
	}
	inline position_t Convert(const cv::Point& point) {
		return { point.x,point.y };
	}
	inline cv::Point Transform(const vision::position_t& position) {
		return { (int)position.x,(int)position.y };
	}
	inline vision::position_t Transform(const cv::Point& position) {
		return { (float)position.x,(float)position.y };
	}

	inline ngs::HSV Convert(const std::array<int, 3>& hsv) {
		return { (ngs::byte)hsv[0],hsv[1] / (float)0xFF,hsv[2] / (float)0xFF };
	}
}

namespace vision {

	constexpr ngs::Point2s MAZE_SIZE{ 10, 10 };
	constexpr ngs::Point2s GRID_NUM(2 * MAZE_SIZE.x + 1, 2 * MAZE_SIZE.y + 1);

	struct ColorRange {
		std::array<int, 3>
			low, high;

		cv::Mat GetMask(const cv::Mat& hsv)const;
		std::array<int, 3> GetColor()const;
	};
	namespace color_defined {
		constexpr ColorRange
			red = { {145,0x25,0x50},{210,0xFF,0xFF} },
			green = { {37,0x20,0x20},{80,0xFF,0xFF} },
			blue = { {85,0x40,0x50},{125,0xFF,0xFF} },
			yellow = { {8,0x20,0x50},{35,0xFF,0xFF} }
		;
	}

	struct Treasure {
		ngs::HSV card;
		ngs::HSV pattern;
		enum class Form {
			unknown,
			circle,
			triangle,
		} form;

		operator bool()const { return form == Form::unknown; }
	};

	enum class MazeGrid : ngs::byte {
		unknown = 0,

		wall,
		road,

		treasure,
		treasure_target,
		treasure_unknown,

		start,
		end,
	};
};