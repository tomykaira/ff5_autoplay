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

std::vector<int> findNumbers(cv::Mat &search_img0);
