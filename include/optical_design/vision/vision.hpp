#include "optical_design/vision/vision.h"

namespace vision {
	NGS_HPP_INLINE cv::Mat ColorRange::GetMask(const cv::Mat& hsv) const {
		cv::Mat mask;
		if (high[0] > 180) {
			cv::Mat mask1, mask2;
			cv::inRange(hsv, low, std::array{ 180, high[1], high[2] }, mask1);
			cv::inRange(hsv, std::array{ 0, low[1], low[2] }, std::array{ high[0] - 180, high[1], high[2] }, mask2);
			cv::bitwise_or(mask1, mask2, mask);
			return mask;
		}
		cv::inRange(hsv, low, high, mask);
		return mask;
	}

	NGS_HPP_INLINE std::array<int, 3> ColorRange::GetColor() const {
		std::array<int, 3> result = {};
		for (size_t i = 0; i < result.max_size(); i++)
		{
			result[i] = (low[i] + high[i]) / 2;
		}
		return result;
	}

	NGS_HPP_INLINE void CVUtil::MAX_SIZE(cv::Mat& image) { resize(image, image, { ngs::Max(image.rows,image.cols),ngs::Max(image.rows,image.cols) }); }

	NGS_HPP_INLINE void CVUtil::MIN_SIZE(cv::Mat& image) { resize(image, image, { ngs::Min(image.rows,image.cols),ngs::Min(image.rows,image.cols) }); }

	NGS_HPP_INLINE cv::Mat CVUtil::THIN(const cv::Mat& src, const int maxIterations) {
		assert(src.type() == CV_8UC1);
		cv::Mat dst;
		int width = src.cols;
		int height = src.rows;
		src.copyTo(dst);
		int count = 0;  //记录迭代次数  
		while (true)
		{
			count++;
			if (maxIterations != -1 && count > maxIterations) //限制次数并且迭代次数到达  
				break;
			std::vector<uchar*> mFlag; //用于标记需要删除的点  
			//对点标记  
			for (int i = 0; i < height; ++i)
			{
				uchar* p = dst.ptr<uchar>(i);
				for (int j = 0; j < width; ++j)
				{
					//如果满足四个条件，进行标记  
					//  p9 p2 p3  
					//  p8 p1 p4  
					//  p7 p6 p5  
					uchar p1 = p[j];
					if (p1 != 1) continue;
					uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
					uchar p8 = (j == 0) ? 0 : *(p + j - 1);
					uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
					uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
					uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
					uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
					uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
					uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
					if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
					{
						int ap = 0;
						if (p2 == 0 && p3 == 1) ++ap;
						if (p3 == 0 && p4 == 1) ++ap;
						if (p4 == 0 && p5 == 1) ++ap;
						if (p5 == 0 && p6 == 1) ++ap;
						if (p6 == 0 && p7 == 1) ++ap;
						if (p7 == 0 && p8 == 1) ++ap;
						if (p8 == 0 && p9 == 1) ++ap;
						if (p9 == 0 && p2 == 1) ++ap;

						if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
							mFlag.push_back(p + j);
					}
				}
			}

			//将标记的点删除  
			for (std::vector<uchar*>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
				**i = 0;

			//直到没有点满足，算法结束  
			if (mFlag.empty())
				break;
			else
				mFlag.clear();//将mFlag清空  

			//对点标记  
			for (int i = 0; i < height; ++i)
			{
				uchar* p = dst.ptr<uchar>(i);
				for (int j = 0; j < width; ++j)
				{
					//如果满足四个条件，进行标记  
					//  p9 p2 p3  
					//  p8 p1 p4  
					//  p7 p6 p5  
					uchar p1 = p[j];
					if (p1 != 1) continue;
					uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
					uchar p8 = (j == 0) ? 0 : *(p + j - 1);
					uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
					uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
					uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
					uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
					uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
					uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);

					if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
					{
						int ap = 0;
						if (p2 == 0 && p3 == 1) ++ap;
						if (p3 == 0 && p4 == 1) ++ap;
						if (p4 == 0 && p5 == 1) ++ap;
						if (p5 == 0 && p6 == 1) ++ap;
						if (p6 == 0 && p7 == 1) ++ap;
						if (p7 == 0 && p8 == 1) ++ap;
						if (p8 == 0 && p9 == 1) ++ap;
						if (p9 == 0 && p2 == 1) ++ap;

						if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
							mFlag.push_back(p + j);
					}
				}
			}

			//将标记的点删除  
			for (std::vector<uchar*>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
				**i = 0;

			//直到没有点满足，算法结束  
			if (mFlag.empty())
				break;
			else
				mFlag.clear();//将mFlag清空  
		}
		return dst;
	}

	NGS_HPP_INLINE void CVUtil::IMAGE_STRETCH_BY_HISTOGRAM(const cv::Mat& src, cv::Mat& dst) {
		//判断传入参数是否正常
		if (!(src.size().width == dst.size().width))
		{
			std::cout << "error" << std::endl;
			return;
		}
		double p[256], p1[256], num[256];

		memset(p, 0, sizeof(p));
		memset(p1, 0, sizeof(p1));
		memset(num, 0, sizeof(num));
		int height = src.size().height;
		int width = src.size().width;
		long wMulh = height * width;

		//统计每一个灰度值在整个图像中所占个数
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				uchar v = src.at<uchar>(y, x);
				num[v]++;
			}
		}

		//使用上一步的统计结果计算每一个灰度值所占总像素的比例
		for (int i = 0; i < 256; i++)
		{
			p[i] = num[i] / wMulh;
		}

		//计算每一个灰度值，小于当前灰度值的所有灰度值在总像素中所占的比例
		//p1[i]=sum(p[j]);	j<=i;
		for (int i = 0; i < 256; i++)
		{
			for (int k = 0; k <= i; k++)
				p1[i] += p[k];
		}

		//以小于当前灰度值的所有灰度值在总像素中所占的比例，作为增益系数对每一个像素点进行调整。
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++) {
				uchar v = src.at<uchar>(y, x);
				dst.at<uchar>(y, x) = p1[v] * 255 + 0.5;
			}
		}
		return;
	}

	NGS_HPP_INLINE cv::Mat CVUtil::THRESHOLD(const cv::Mat& image) {
		cv::Mat gray, bin;

		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		adaptiveThreshold(gray, bin, 0x01, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 7, 3);
		cv::resize(bin, bin, { cv::min(bin.rows,bin.cols),cv::min(bin.rows,bin.cols) });

		//创建结构元素-水平直线
		cv::Mat hLineKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(bin.cols / 16, 1));
		//创建结构元素-垂直直线
		cv::Mat vLineKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, bin.rows / 16));

		//提取水平线
		cv::Mat hLineImage;
		morphologyEx(bin, hLineImage, cv::MORPH_OPEN, hLineKernel);
		//提取垂直线0
		cv::Mat vLineImage;
		morphologyEx(bin, vLineImage, cv::MORPH_OPEN, vLineKernel);

		//提取字母
		cv::Mat alpImage;
		bitwise_or(hLineImage, vLineImage, alpImage);

		/*alpImage *= 0xFF;
		cv::imshow("", alpImage);
		cv::waitKey();*/

		//图像细化
		cv::Mat dst = THIN(alpImage);

		//dilate(dst, dst, getStructuringElement(MORPH_RECT, Size(2, 2)), Point(-1, -1), 7);

		//显示图像
		dst *= 0xFF;

		return dst;
	}

	NGS_HPP_INLINE std::vector<bounds_t> CVUtil::TREASURE_RANGE(const cv::Mat& src) {
		ngs::Point2f size = { (float)src.cols,(float)src.rows };
		cv::Mat bin;
		cv::cvtColor(src, bin, cv::ColorConversionCodes::COLOR_BGR2GRAY);
		adaptiveThreshold(bin, bin, 0xFF, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 7, 10);

		morphologyEx(bin, bin, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 1,1 }), { -1,-1 }, 2);
		//erode(bin,bin, getStructuringElement(MorphShapes::MORPH_RECT, { 2,2 }), { -1,-1 }, 3);
		//dilate(bin,bin, getStructuringElement(MorphShapes::MORPH_RECT, { 2,2 }), { -1,-1 }, 2);

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		//findContours(srcImg, contours, hierarchy,RETR_EXTERNAL, CHAIN_APPROX_SIMPLE ); //查找外轮廓，压缩存储轮廓点
		findContours(bin, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE); //查找所有轮廓
		//findContours(srcImg, contours, hierarchy,CV_RETR_CCOMP, CHAIN_APPROX_SIMPLE ); //查找所有轮廓
		//findContours(srcImg, contours, hierarchy,RETR_TREE, CHAIN_APPROX_NONE ); //查找所有轮廓，存储所有轮廓点

		// 定义一个矩形集合
		std::vector<bounds_t> rects;
		size_t imageArea = size.x * size.y;
		size_t areaThresholdMax = imageArea / (18 * 18);
		size_t areaThresholdMin = imageArea / (40 * 40);

		for (size_t i = 0; i < contours.size(); i++)
		{
			auto& contour = contours[i];
			// 如果轮廓点数小于5，则跳过
			if (contour.size() < 5) continue;

			// 计算当前轮廓的最小外接矩形
			bounds_t rect = ngs_cv::Transform(cv::boundingRect(cv::Mat(contour)));

			// 如果矩形面积大于90，则跳过
			if (rect.Area() > areaThresholdMax) continue;
			if (rect.Area() < areaThresholdMin)continue;

			// 如果长宽比小于0.6或大于1.73，则跳过
			if (!ngs::In<float>(rect.WH_Ratio(), std::numbers::inv_pi, std::numbers::ln10)) continue;

			// 将合法的矩形加入集合中
			rects.push_back(rect);
		}
		if (rects.empty())return {};
		std::sort(rects.begin(), rects.end(), [](const bounds_t& a, const bounds_t& b) {return a > b; });

		constexpr float treasure_area_threshold = 0.05;
		bool independent = false;
		while (!independent) {
			independent = true;
			for (auto i = rects.begin(); i != rects.end(); i++) {
				bounds_t& rect1 = *i;
				for (auto j = rects.begin(); j != rects.end(); j++) {
					if (i == j)continue;
					bounds_t& rect2 = *j;
					bounds_t intersect = rect1 & rect2;
					if (!intersect)continue;
					if (intersect < rect1 && intersect < rect2)continue;
					independent = false;
					rect1 = rect1 | rect2;
					j = rects.erase(j);
					break;
				}
				if (!independent)break;
			}
		}
		/*cv::Mat dst;
		cv::cvtColor(bin, dst, cv::ColorConversionCodes::COLOR_GRAY2BGR);
		for (auto& rect : rects) {
			cv::rectangle(dst, ngs_cv::Convert(rect), { 0x00,0x00,0xFF }, 1);
		}
		cv::imshow("dbg", bin);
		cv::imshow("", dst);
		cv::waitKey();*/
		return rects;
	}

	NGS_HPP_INLINE std::vector<cv::Rect> CVUtil::COLOR_BOUNDS(const cv::Mat& hsv, const ColorRange& range) { return COLOR_BOUNDS(hsv, range.GetMask(hsv)); }

	NGS_HPP_INLINE std::vector<cv::Rect> CVUtil::COLOR_BOUNDS(const cv::Mat& hsv, const cv::Mat& mask) {
		cv::Mat filter;
		cv::morphologyEx(mask, filter, cv::MorphTypes::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 5,5 }), { -1,-1 }, 1);
		//dilate(mask, mask, getStructuringElement(MorphShapes::MORPH_RECT, { 8,8 }), { -1,-1 }, 2);
		//erode(mask,mask, getStructuringElement(MorphShapes::MORPH_RECT, { 8,8 }), { -1,-1 }, 2);

		/*cv::imshow("", filter);
		cv::waitKey();*/

		return CONTOURS_TO_BOUNDS(filter);
	}

	NGS_HPP_INLINE std::vector<cv::Rect> CVUtil::CONTOURS_TO_BOUNDS(const cv::Mat& bin) {

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		findContours(bin, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
		//cvtColor(bin, bin, COLOR_GRAY2BGR);

		std::vector<cv::Rect> bounds;
		for (auto& points : contours) {
			bounds.push_back(boundingRect(points));
		}
		auto compare = [](const cv::Rect& a, const cv::Rect& b)->bool {
			return a.area() > b.area();
		};
		sort(bounds.begin(), bounds.end(), compare);

		return bounds;
	}

	NGS_HPP_INLINE position_t CVUtil::TO_MAZE(const bounds_t& rect, const bounds_t& frame) { return TO_MAZE(rect.Center(), frame); }

	NGS_HPP_INLINE position_t CVUtil::TO_MAZE(const position_t& pos, const bounds_t& frame) {
		position_t grid(frame.width / MAZE_SIZE.x, frame.height / MAZE_SIZE.y);
		position_t O{ frame.x + grid.x / 2, frame.y + grid.y / 2 };
		position_t out = pos - O;
		out.x = 2 * round(out.x / grid.x) + 1;
		out.y = 2 * round(out.y / grid.y) + 1;
		return out;
	}

	NGS_HPP_INLINE cv::Mat CVUtil::RECOGONIZE_MAZE(const cv::Mat& source, int result_width, int result_height) {
		NGS_ASSERT(!source.empty(), "source is empty");

		//外部定义,模板轮廓
		//	---------
		//	|		|
		//	|		|
		//	---------
		static const std::vector<cv::Point> temp_contour = {
			{1,0},
			{0,1},
			{0,26},
			{1,27},
			{26,27},
			{27,26},
			{27,1},
			{26,0},
		};
		constexpr ngs::Point2f image_size(500, 500);

		cv::Mat src;
		src = source;

		std::vector<cv::Point> corners;
		{
			struct Buffer {
				size_t index;
				ngs::float64 matchLevel;
				bounds_t rect;
			};

			cv::Mat canny;
			{
				cv::resize(src, src, {}, image_size.x / src.cols, image_size.y / src.cols);
				cv::cvtColor(src, canny, cv::ColorConversionCodes::COLOR_BGR2GRAY);
				cv::adaptiveThreshold(canny, canny, 0xFF, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 7, 8);
				cv::morphologyEx(canny, canny, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, { 2, 2 }), { -1,-1 }, 1);
				cv::Laplacian(canny, canny, 16);
			}


			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::findContours(canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, { 0,0 });

			constexpr auto area_threshold = 300;

			std::vector<Buffer> buffers;
			buffers.reserve(8);
			for (size_t i = 0; i < contours.size(); i++)
			{
				//过滤不符合要求的物体
				ngs::float64 matchLevel;
				bounds_t rect;
				{
					//去除不规则多边形
					std::vector<cv::Point> contour;
					contour.reserve(20);
					cv::approxPolyDP(contours[i], contour, 5, true);
					if (contour.size() > 6)continue;

					//去除匹配精度不够高的物体
					matchLevel = (
						cv::matchShapes(contours[i], temp_contour, cv::ShapeMatchModes::CONTOURS_MATCH_I1, 0)
						+ cv::matchShapes(contour, temp_contour, cv::ShapeMatchModes::CONTOURS_MATCH_I1, 0)
						) / 2;

					if (matchLevel > 1)continue;

					//去除小物体
					double area = cv::contourArea(contours[i]);
					if (area < area_threshold)continue;

					//去除长得极为抽象的物体
					rect = ngs_cv::Transform(cv::boundingRect(contour));
					if (ngs::Abs(area - rect.Area()) > area_threshold)continue;

					//去除长方形
					auto ratio = rect.WH_Ratio();
					if (!ngs::In<ngs::float32>(ratio, std::numbers::inv_pi, std::numbers::ln10))continue;

					//去除过大的物体
					const ngs::Point2f threshold = { canny.cols / 8.0f,canny.rows / 8.0f };
					if (rect.width > threshold.x || rect.height > threshold.y)continue;

				}
				buffers.emplace_back(i, matchLevel, rect);
			}
			if (buffers.size())NGS_LOGFL(debug, "找到%ld个可能的图形", buffers.size());

			//在符合要求的物体中找到最符合要求的物体
			if (buffers.size() < 4) {
				NGS_LOGFL(debug, "识别失败!找到的图形不足四个! %d", buffers.size());
				return {};
			}

			std::sort(buffers.begin(), buffers.end(), [](const Buffer& a, const Buffer& b)->bool {
				if (ngs::Abs(a.rect.Area() - b.rect.Area()) > area_threshold) return a.matchLevel < b.matchLevel;
				return a.matchLevel < b.matchLevel;
				});

			for (size_t i = 0; i < buffers.size() - 3; i++)
			{
				const Buffer&
					b1 = buffers[i],
					b2 = buffers[i + 1],
					b3 = buffers[i + 2],
					b4 = buffers[i + 3]
					;
				auto averageArea = (b1.rect.Area() + b2.rect.Area() + b3.rect.Area() + b4.rect.Area()) / 4;
				constexpr ngs::float64 rate = 0.4;
				if (
					ngs::Abs(averageArea - b1.rect.Area()) > averageArea * rate ||
					ngs::Abs(averageArea - b2.rect.Area()) > averageArea * rate ||
					ngs::Abs(averageArea - b3.rect.Area()) > averageArea * rate ||
					ngs::Abs(averageArea - b4.rect.Area()) > averageArea * rate
					)continue;


				for (size_t j = 0; j < 4; j++)
				{
					corners.push_back(ngs_cv::Transform(buffers[i + j].rect.Center()));
				}
				std::sort(corners.begin(), corners.end(), [](const cv::Point& a, const cv::Point& b)->bool {
					return a.x + a.y < b.x + b.y;
					});
			}
			NGS_LOGL(debug, "识别成功!");
		}

		if (!corners.size()) {
			NGS_LOGL(debug, "识别失败!");
			return {};
		}
		cv::Mat result(result_height, result_width, src.type());
		{
			cv::Mat matrix;
			//需要四个点
			std::vector<cv::Point2f> srcPointP;
			std::vector<cv::Point2f> dstPointP;
			//坐标顺序会一一对应变换
			srcPointP.push_back(corners[0]);
			srcPointP.push_back(corners[1]);
			srcPointP.push_back(corners[2]);
			srcPointP.push_back(corners[3]);

			dstPointP.push_back(cv::Point2f(0, 0));
			dstPointP.push_back(cv::Point2f(result_width, 0));
			dstPointP.push_back(cv::Point2f(0, result_height));
			dstPointP.push_back(cv::Point2f(result_width, result_height));

			//获得透视变换矩阵
			matrix = getPerspectiveTransform(srcPointP, dstPointP);
			//进行透视变换
			warpPerspective(src, result, matrix, result.size());
		}

		return result;
	}

	NGS_HPP_INLINE Treasure CVUtil::RECOGNIZE_TREASURE(cv::Mat& source, const ColorRange& CardRange, const ColorRange& PatternRange) {
		ngs::Point2i image_size = { 350,350 };
		cv::Mat dst;
		cv::resize(source, dst, { image_size.x ,(int)((float)image_size.x * source.rows / source.cols) });
		image_size.x = dst.cols;
		image_size.y = dst.rows;

		/*cv::imshow("", source);
		cv::waitKey();*/

		auto bounds = _RECOGNIZE_CARD_COLOR(dst, CardRange);
		if (bounds.size() == 0) {
			NGS_LOGL(warning, "未找到指定的颜色范围内的轮廓");
			return { {},{},Treasure::Form::unknown };
		}
		Treasure::Form form = Treasure::Form::unknown;
		for (auto& bound : bounds) {
			form = _RECOGNIZE_PATTERN(dst(ngs_cv::Transform(bound)), PatternRange);
			if (form != Treasure::Form::unknown)break;
		}

		Treasure treasure = {};
		treasure.card = ngs_cv::Convert(CardRange.GetColor());
		treasure.pattern = ngs_cv::Convert(PatternRange.GetColor());
		treasure.form = form;
		return treasure;
	}

	NGS_HPP_INLINE bool CVUtil::IS_IN_MAZE(position_t pos) {
		bounds_t maze = { 0,0,GRID_NUM.x,GRID_NUM.y };
		if (!ngs::Between(pos.x, maze.Left(), maze.Right()))return false;
		if (!ngs::Between(pos.y, maze.Top(), maze.Bottom()))return false;
		return true;
	}

	NGS_HPP_INLINE position_t CVUtil::GET_MAZE_CENTER() { return position_t{ (position_t::type)GRID_NUM.x / 2, (position_t::type)GRID_NUM.y / 2 }; }

	NGS_HPP_INLINE position_t CVUtil::GET_MAZE_SYMMETRY(const position_t& pos) { return (2 * GET_MAZE_CENTER()) - pos; }

	NGS_HPP_INLINE std::optional<std::tuple<position_t, position_t, std::vector<position_t>>> CVUtil::CONVERT_IMAGE_TO_MAZE(cv::Mat image, ColorRange start_color, ColorRange end_color, MazeGrid(&maze)[GRID_NUM.x][GRID_NUM.y]) {
		NGS_ASSERT(!image.empty());
		auto bin = CVUtil::THRESHOLD(image);

		bounds_t frame;
		position_t start{}, end{};
		std::vector<position_t> treasures;

		double stepX, stepY;
		//计算地图边界及每个格子的大小
		{
			NGS_LOGL(debug, "开始寻找起点终点...");
			cv::Mat hsv;
			cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
			auto start_bounds = CVUtil::COLOR_BOUNDS(
				hsv
				, start_color
			);
			auto end_bounds = CVUtil::COLOR_BOUNDS(
				hsv
				, end_color
			);
			if (start_bounds.empty() || end_bounds.empty()) {
				NGS_LOGL(warning, "未找到起点终点");
				return {};
			}
			bounds_t color_start = ngs_cv::Transform(start_bounds[0]);
			bounds_t color_end = ngs_cv::Transform(end_bounds[0]);

			NGS_LOGFL(debug, "区域有%ld个，蓝色区域有%ld个，将(%d,%d),(%d,%d)作为起点和终点"
				, start_bounds.size(), end_bounds.size()
				, (int)color_start.CenterX(), (int)color_start.CenterY()
				, (int)color_end.CenterX(), (int)color_end.CenterY());
			NGS_LOGL(debug, "开始寻找地图边界...");

			bounds_t range = color_start | color_end;
			bounds_t un;
			auto bin_bounds = CVUtil::CONTOURS_TO_BOUNDS(bin);
			for (auto& bound_cv : bin_bounds) {
				bounds_t bound = ngs_cv::Transform(bound_cv);
				if (!(bound & range))continue;
				if (un.Empty()) {
					un = bound;
					continue;
				}
				un |= bound;
			}
			auto map_center = un.Center();
			auto len = ngs::Min(un.width, un.height);
			bounds_t map{ map_center.x - len / 2, map_center.y - len / 2, len, len };

			stepX = round((double)len / MAZE_SIZE.x);
			stepY = round((double)len / MAZE_SIZE.y);

			frame.Set(
				(int)(map.x - (stepX * MAZE_SIZE.x - map.width) / 2)
				, (int)(map.y - (stepY * MAZE_SIZE.y - map.height) / 2)
				, (int)(stepX * MAZE_SIZE.x)
				, (int)(stepY * MAZE_SIZE.y)
			);
			NGS_LOGFL(debug, "地图边界为(%d,%d,%d,%d)，每个格子大小为(%d,%d)", (int)frame.x, (int)frame.y, (int)frame.width, (int)frame.height, (int)stepX, (int)stepY);


			start = CVUtil::TO_MAZE(color_start.Center(), frame);
			end = CVUtil::TO_MAZE(color_end.Center(), frame);

			if (IS_IN_MAZE(start) && IS_IN_MAZE(end)) {
				NGS_LOGL(warning, "颜色范围过大,红色蓝色都找错了!");
				return {};
			}
			if (IS_IN_MAZE(start) && !IS_IN_MAZE(end)) {
				NGS_LOGL(warning, "起点找错了，正在进行修正...");
				start = GET_MAZE_SYMMETRY(end);
			}
			if (!IS_IN_MAZE(end) && IS_IN_MAZE(end)) {
				NGS_LOGL(warning, "终点找错了，正在进行修正...");
				end = GET_MAZE_SYMMETRY(start);
			}

			start.x = ngs::Clamp<int>(start.x, 0, GRID_NUM.x - 1);
			start.y = ngs::Clamp<int>(start.y, 0, GRID_NUM.y - 1);
			end.x = ngs::Clamp<int>(end.x, 0, GRID_NUM.x - 1);
			end.y = ngs::Clamp<int>(end.y, 0, GRID_NUM.y - 1);

			NGS_LOGFL(debug, "起点：(%d,%d) 终点：(%d,%d)", (int)start.x, (int)start.y, (int)end.x, (int)end.y);
			NGS_LOGL(debug, "开始寻找宝藏区域...");
			// 获取宝藏区域
			auto treasures_t = CVUtil::TREASURE_RANGE(image);
			if (treasures_t.size() < 8) {
				NGS_LOGL(warning, "宝藏数量小于8个");
				return {};
			}

			NGS_LOGFL(debug, "宝藏区域数量%ld", treasures_t.size());
			for (auto& i : treasures_t) {
				if (!(i & frame))continue;
				position_t p = CVUtil::TO_MAZE(i, frame);
				treasures.push_back(p);
				NGS_LOGFL(debug, "宝藏区域为(%d,%d)", (int)p.x, (int)p.y);
			}
			if (treasures.size() != 8) {
				NGS_LOGL(warning, "宝藏区域数量不为8");
				return {};
			}
		}
		//将数据转换成图
		{
			NGS_LOGL(debug, "开始将墙与路写入迷宫...");

			std::vector<std::vector<int>> original_data_row;
			std::vector<std::vector<int>> original_data_col;
			{
				for (int y = 0; y < MAZE_SIZE.y; y++) {
					size_t black = 0;
					size_t n = 0;
					original_data_row.emplace_back();
					for (size_t x = frame.Left(); x < frame.Right(); x++) {
						auto bw = bin.at<uchar>(
							frame.Top() + y * stepY + stepY * 0.5
							, x
						);
						if (bw) {
							size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
							if (t) {
								original_data_row.back().push_back(t);
								n += t;
							}
							black = 0;
							continue;
						}
						black++;
					}
					if (black) {
						size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
						if (t) {
							original_data_row.back().push_back(t);
							n += t;
						}
						black = 0;
					}
					if (n != 10) {
						NGS_LOGL(warning, "转换失败");
						return {};
					}
				}

				for (int x = 0; x < MAZE_SIZE.x; x++) {
					size_t black = 0;
					size_t n = 0;
					original_data_col.emplace_back();
					for (size_t y = frame.Top(); y < frame.Bottom(); y++) {
						auto bw = bin.at<uchar>(
							y
							, frame.Left() + x * stepX + stepX * 0.5
						);
						if (bw) {
							size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
							if (t) {
								original_data_col.back().push_back(t);
								n += t;
							}
							black = 0;
							continue;
						}
						black++;
					}
					if (black) {
						size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
						if (t) {
							original_data_col.back().push_back(t);
							n += t;
						}
						black = 0;
					}
					if (n != 10) {
						NGS_LOGL(warning, "转换失败");
						return {};
					}
				}
			}

			{
				for (size_t j = 0; j < GRID_NUM.y; j++) {
					for (size_t i = 0; i < GRID_NUM.x; i++)
						maze[i][j] = MazeGrid::wall;
				}

				for (size_t j = 0; j < original_data_row.size(); j++) {
					size_t y = 2 * j + 1;
					size_t x = 0;
					maze[x][y] = MazeGrid::wall;
					for (auto num : original_data_row[j]) {
						for (size_t i = 0; i < num; i++) {
							x++;
							maze[x][y] = MazeGrid::road;
							x++;
							maze[x][y] = MazeGrid::road;
						}
						maze[x][y] = MazeGrid::wall;
					}
				}

				for (size_t j = 0; j < original_data_col.size(); j++) {
					size_t x = 2 * j + 1;
					size_t y = 0;
					maze[x][y] = MazeGrid::wall;
					for (auto num : original_data_col[j]) {
						for (size_t i = 0; i < num; i++) {
							y++;
							maze[x][y] = MazeGrid::road;
							y++;
							maze[x][y] = MazeGrid::road;
						}
						maze[x][y] = MazeGrid::wall;
					}
				}
			}
			NGS_LOGL(debug, "写入成功");
		}
		return { {start,end,treasures} };
	}

	NGS_HPP_INLINE std::vector<bounds_t> CVUtil::_RECOGNIZE_CARD_COLOR(cv::Mat& source, const ColorRange& CardRange) {
		constexpr ngs::Point2f min_rate = { 0.05,0.05 };
		constexpr ngs::Point2f max_rate = { 0.8,0.8 };

		cv::Mat hsv;
		cv::cvtColor(source, hsv, cv::COLOR_BGR2HSV);
		cv::Mat mask = CardRange.GetMask(hsv);
		cv::Mat filter;
		cv::morphologyEx(mask, filter, cv::MorphTypes::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 5,5 }), { -1,-1 }, 1);

		std::vector<std::vector<cv::Point>> CardContours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(filter, CardContours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		/*cv::imshow("", filter);
		cv::waitKey();*/

		/*for (size_t i = 0; i < CardContours.size(); i++)
		{
			cv::drawContours(source, CardContours, i, { 0xFF,0x00,0x00 }, 2);
		}
		cv::imshow("", source);
		cv::waitKey();*/

		std::vector<bounds_t> bounds;
		for (const auto& CardContour : CardContours) {
			bounds_t bound = ngs_cv::Transform(cv::boundingRect(CardContour));

			double CardArea = cv::contourArea(CardContour);
			if (!ngs::In<float>(CardArea / bound.Area(), 0.3, 1.5))continue;

			if (!ngs::In<float>(bound.width, filter.cols * min_rate.x, filter.cols * max_rate.x))continue;
			if (!ngs::In<float>(bound.height, filter.rows * min_rate.y, filter.rows * max_rate.y))continue;
			//cv::rectangle(source, ngs_cv::Convert(bound), { 0xFF,0x00,0x00 }, 2);
			if (!ngs::In<float>(bound.WH_Ratio(), 0.3, 3.5))continue;

			/*cv::imshow("", source);
			cv::waitKey();*/

			bounds.push_back(bound);
		}
		NGS_LOGL(debug, "检测到卡片%d个", bounds.size());
		std::ranges::sort(bounds, [](const bounds_t& a, const bounds_t& b) {
			return a > b;
			});

		return bounds;
	}

	NGS_HPP_INLINE Treasure::Form CVUtil::_RECOGNIZE_PATTERN(const cv::Mat& source, const ColorRange& PatternRange) {
		constexpr ngs::Point2f min_rate = { 0.25,0.25 };
		constexpr ngs::Point2f max_rate = { 0.95,0.7 };

		/*cv::imshow("", source);
		cv::waitKey();*/

		cv::Mat hsv = source;
		cv::cvtColor(hsv, hsv, cv::COLOR_BGR2HSV);
		ngs::Point2f image_size = { 200,200 };
		cv::resize(hsv, hsv, {}, image_size.x / hsv.cols, image_size.y / hsv.cols);
		image_size.Set((float)hsv.cols, (float)hsv.rows);

		cv::Mat mask = PatternRange.GetMask(hsv);
		cv::Mat filter;
		cv::morphologyEx(mask, filter, cv::MorphTypes::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 5,5 }), { -1,-1 }, 1);


		/*cv::imshow("", filter);
		cv::waitKey();*/

		std::vector<std::vector<cv::Point>> PatternContours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(filter, PatternContours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
		cv::cvtColor(hsv, hsv, cv::COLOR_HSV2BGR);
		std::vector<std::vector<cv::Point>> validContours;
		std::vector<bounds_t> bounds;

		for (size_t i = 0; i < PatternContours.size(); i++) {
			const auto& PatternContour = PatternContours[i];
			cv::drawContours(hsv, PatternContours, i, { 0x00,0xFF,0x00 }, 2);
			cv::imshow("", hsv);
			cv::waitKey();
			bounds_t bound = ngs_cv::Transform(cv::boundingRect(PatternContour));
			if (!ngs::In(bound.width, image_size.x * min_rate.x, image_size.x * max_rate.x))continue;
			if (!ngs::In(bound.height, image_size.y * min_rate.y, image_size.y * max_rate.y))continue;

			//cv::rectangle(source, ngs_cv::Convert(bound), { 0x00,0xFF,0x00 }, 2);
			if (!ngs::In<float>(bound.WH_Ratio(), 0.5, 2))continue;
			//cv::rectangle(hsv, ngs_cv::Convert(bound), { 0x00,0xFF,0x00 }, 2);

			/*cv::imshow("", hsv);
			cv::waitKey();*/
			double PatternArea = cv::contourArea(PatternContour);
			if (!ngs::In<float>(bound.Area() / PatternArea, 0.8 * 0.8, 1.5 * 1.5))continue;

			validContours.push_back(PatternContour);
		}

		NGS_LOGL(debug, "检测到形状%d个", validContours.size());

		if (validContours.empty())
		{
			NGS_LOGL(debug, "未找到指定的颜色范围内的轮廓");
			return Treasure::Form::unknown;
		}

		for (size_t i = 0; i < validContours.size(); i++)
		{
			size_t count = 0;
			Treasure::Form form = Treasure::Form::unknown;

			if (_IS_CIRCLE(validContours)) {
				form = Treasure::Form::circle;
				count++;
			}
			if (_IS_TRIANGLE(validContours)) {
				form = Treasure::Form::triangle;
				count++;
			}
			if (count != 1)continue;
			return form;
		}
		return Treasure::Form::unknown;

	}

	NGS_HPP_INLINE bool CVUtil::_IS_CIRCLE(const std::vector<std::vector<cv::Point>>& contours) {
		for (size_t i = 0; i < contours.size(); i++)
		{
			cv::Point2f center;
			float radius;
			cv::minEnclosingCircle(contours[i], center, radius);
			double area = cv::contourArea(contours[i]);
			double perimeter = cv::arcLength(contours[i], true);
			double circularity = 4 * CV_PI * area / (perimeter * perimeter);

			if (circularity > 0.7) {
				NGS_LOGL(debug, "这是一个圆形");
				return true;
			}
			return false;
		}
		return false;
	}

	NGS_HPP_INLINE bool CVUtil::_IS_TRIANGLE(const std::vector<std::vector<cv::Point>>& contours) {
		for (size_t i = 0; i < contours.size(); i++)
		{
			std::vector<cv::Point> polygon;
			polygon.reserve(20);
			cv::approxPolyDP(contours[i], polygon, 10, true);

			if (ngs::In<size_t>(polygon.size(), 3, 5))
			{
				NGS_LOGL(debug, "这是一个三角形");
				return true;
			}
			return false;
		}
		return false;
	}



};
