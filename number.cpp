#include <iostream>
#include <cstdio>
#include <stdint.h>
#include <stdlib.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

int
main(int argc, char *argv[])
{
	cv::Mat search_img0 = cv::imread(argv[1], 1);

	// テンプレート画像
	cv::Mat tmp_img = cv::imread(argv[2], 1);
	if(!tmp_img.data) return -1;

	cv::namedWindow("search image", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);

	// 探索画像
	cv::Mat search_img;
	search_img0.copyTo( search_img );

	cv::Mat result_img;
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
		if ( maxVal < 0.9 ) break;

		roi_rect.x = max_pt.x;
		roi_rect.y = max_pt.y;
		std::cout << "(" << max_pt.x << ", " << max_pt.y << "), score=" << maxVal << std::endl;
		// 探索結果の場所に矩形を描画
		cv::rectangle(search_img0, roi_rect, cv::Scalar(0,255,255), 1);
		cv::rectangle(search_img, roi_rect, cv::Scalar(0,0,255), CV_FILLED);
	}

	cv::imshow("search image", search_img0);
	cv::waitKey(0);
}
