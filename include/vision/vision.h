#pragma once

#include "vision/defined.h"


namespace vision {

	class CVUtil {
	public:
		/** @brief 将图像缩放到最大边长的大小 */
		static void MAX_SIZE(cv::Mat& image);
		/** @brief 将图像缩放到最大边长的大小 */
		static void MIN_SIZE(cv::Mat& image);

		/**
		* @brief 对输入图像进行细化,骨骼化
		*
		* @param src 输入图像,用cvThreshold函数处理过的8位灰度图像格式，元素中只有0与1,1代表有元素，0代表为空白
		* @param maxIterations 限制迭代次数，如果不进行限制，默认为-1，代表不限制迭代次数，直到获得最终结果
		*
		* @return 为对src细化后的输出图像,格式与src格式相同，元素中只有0与1,1代表有元素，0代表为空白
		*/
		static cv::Mat THIN(const cv::Mat& src, const int maxIterations = -1);

		/**
		 * @brief 增强算法的原理在于先统计每个灰度值在整个图像中所占的比例
		 * 然后以小于当前灰度值的所有灰度值在总像素中所占的比例，作为增益系数
		 * 对每一个像素点进行调整。由于每一个值的增益系数都是小于它的所有值所占
		 * 的比例和。所以就使得经过增强之后的图像亮的更亮，暗的更暗。
		 *
		 * @param src
		 * @param dst
		 */
		static void IMAGE_STRETCH_BY_HISTOGRAM(const cv::Mat& src, cv::Mat& dst);

		/**
		 * @brief 二值化并细化图像
		 *
		 * @param image
		 * @return 新图像
		 */
		static cv::Mat THRESHOLD(const cv::Mat& image);

		/**
		 * @brief 获取图像中的宝藏区域
		 *
		 * \param image 一个等宽高彩色图
		 * \return 宝藏区域数组
		 */
		static std::vector<bounds_t> TREASURE_RANGE(const cv::Mat& src);

		static std::vector<cv::Rect> COLOR_BOUNDS(const cv::Mat& hsv, const ColorRange& range);

		static std::vector<cv::Rect> COLOR_BOUNDS(const cv::Mat& hsv, const cv::Mat& mask);

		/**
		 * @brief
		 *
		 * @param bin
		 * @return
		 */
		static std::vector<cv::Rect> CONTOURS_TO_BOUNDS(const cv::Mat& bin);

		static position_t TO_MAZE(const bounds_t& rect, const bounds_t& frame);
		static position_t TO_MAZE(const position_t& pos, const bounds_t& frame);

		static cv::Mat RECOGONIZE_MAZE(const cv::Mat& source, int result_width, int result_height);

		static Treasure RECOGNIZE_TREASURE(cv::Mat& source, const ColorRange& card_range, const ColorRange& pattern_range);

		static bool IS_IN_MAZE(position_t pos);
		static position_t GET_MAZE_CENTER();
		static position_t GET_MAZE_SYMMETRY(const position_t& pos);
		/**
		 * @brief
		 *
		 * @param image
		 * @param start_color
		 * @param end_color
		 * @param[out] maze
		 *
		 * @return start point, end point, treasures
		 */
		static std::optional<std::tuple<position_t, position_t, std::vector<position_t>>> CONVERT_IMAGE_TO_MAZE(cv::Mat image, ColorRange start_color, ColorRange end_color, MazeGrid(&maze)[GRID_NUM.x][GRID_NUM.y]);
	public:
		static std::vector<bounds_t> _RECOGNIZE_CARD_COLOR(cv::Mat& source, const ColorRange& CardRange);

		static Treasure::Form _RECOGNIZE_PATTERN(const cv::Mat& source, const ColorRange& PatternRange);

		static bool _IS_CIRCLE(const std::vector<std::vector<cv::Point>>& contours);

		static bool _IS_TRIANGLE(const std::vector<std::vector<cv::Point>>& contours);
	};

};

#ifdef NGS_USE_HPP
#include "vision/vision.hpp"
#endif
