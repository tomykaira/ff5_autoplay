#include <iostream>
#include <vector>

#include <cstdio>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define DIFF(end, start) (((end).tv_sec - (start).tv_sec)*1000*1000 + (end).tv_usec - (start).tv_usec)

// 数字のサイズ + 余裕
#define SIZE 20

class NumberLocation {
public:
	int x;
	int y;
	int num;

	NumberLocation(int x, int y, int num) :
		x(x), y(y), num(num)
	{
	}
};

static bool compareLocation(const NumberLocation& left, const NumberLocation& right)
{
	return (left.y < right.y)
		|| (left.y == right.y && left.x < right.x)
		|| (left.y == right.y && left.x == right.x && left.num < right.num);
}

std::vector<int> CompositeNumbers(const std::vector<NumberLocation> found)
{
	int current_x = 0;
	int current_y = 0;
	int current_value = 0;
	bool first_hit = true;

	std::vector <int> result;

	std::vector<NumberLocation>::const_iterator it = found.begin();
	while (it != found.end()) {
		if (it->y != current_y || it->x >= current_x + SIZE) {
			if (!first_hit) {
				result.push_back(current_value);
			}
			first_hit = false;

			current_y = it->y;
			current_value = it->num;
		} else {
			current_value = current_value*10 + it->num;
		}
		current_x = it->x;
		++it;
	}

	if (!first_hit) {
		result.push_back(current_value);
	}

	return result;
}

int
main(int argc, char *argv[])
{
	cv::Mat search_img0 = cv::imread(argv[1], 1);

	cv::namedWindow("search image", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);

	// 探索画像
	cv::Mat search_img;
	search_img0.copyTo( search_img );

	cv::Mat result_img;

	struct timeval start, end;
	gettimeofday(&start, NULL);

	std::vector<NumberLocation> found;

	for (double minScore = 1.0; minScore > 0.9; minScore -= 0.02) {

		for (int num = 0; num < 10; ++num) {
			char template_file[1024];
			char num_string[1024];
			sprintf(template_file, "number_data/%d.bmp", num);
			sprintf(num_string, "%d", num);

			// テンプレート画像
			cv::Mat tmp_img = cv::imread(template_file, 1);
			if(!tmp_img.data) return -1;

			std::cout << "number " << num << std::endl;

			// 最大 50 個検出する
			for ( int i=0; i<50; i++ ) {
				// テンプレートマッチング
				cv::matchTemplate(search_img, tmp_img, result_img, CV_TM_CCOEFF_NORMED);
				// 最大のスコアの場所を探す
				cv::Rect roi_rect(0, 0, tmp_img.cols, tmp_img.rows);
				cv::Point max_pt;
				double maxVal;
				cv::minMaxLoc(result_img, NULL, &maxVal, NULL, &max_pt);

				// 一定スコア以下の場合は処理終了
				if ( maxVal < minScore ) break;

				found.push_back(NumberLocation(max_pt.x, max_pt.y, num));

				roi_rect.x = max_pt.x;
				roi_rect.y = max_pt.y;
				std::cout << "\t(" << max_pt.x << ", " << max_pt.y << "), score=" << maxVal << std::endl;

				if (maxVal < 0.999999)
					cv::putText(search_img0, num_string, cvPoint(roi_rect.x, roi_rect.y), cv::FONT_HERSHEY_SIMPLEX, 0.6,
					            CV_RGB(255, 0, 0));


				// 探索結果の場所に矩形を描画
				cv::rectangle(search_img0, roi_rect, cv::Scalar(0,255/10*num,255), 1);
				cv::rectangle(search_img, roi_rect, cv::Scalar(0,0,255), CV_FILLED);
			}

		}

	}

	std::sort(found.begin(),found.end(), compareLocation);

	std::vector<int> extracted = CompositeNumbers(found);

	gettimeofday(&end, NULL);
	std::cout << "elapsed time: " << DIFF(end, start) << std::endl;

	std::vector<int>::iterator it = extracted.begin();
	while (it != extracted.end()) {
		std::cout << *it << std::endl;
		++it;
	}

	cv::imshow("search image", search_img0);
	cv::waitKey(0);
}
