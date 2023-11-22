#include <opencv2/opencv.hpp>
#include <dpp/message.h>
#include <filesystem>
#define cache_channel 1176057541013282896

class image {
	dpp::snowflake id{};
	cv::Mat img{};
public:
	cv::String path() {
		return cv::String(std::format(".\\cache\\{0}.jpg", static_cast<uint64_t>(this->id)));
	}
	std::string raw() {
		return dpp::utility::read_file(this->path().c_str());
	}
	bool image_write(cv::Mat new_img = cv::Mat()) {
		if (not new_img.empty()) this->img = new_img;
		return cv::imwrite(this->path(), this->img);
	}
	/* @param dem y, x e.g. { 100, 100 } */
	image(dpp::snowflake id, std::vector<int> dem) {
		this->id = id;
		image_write(cv::Mat::zeros(dem[0], dem[1], CV_8UC3) + cv::Scalar(0, 0, 0, 240));
	}
	image& background(std::string file_name) {
		cv::Mat image = cv::imread(this->path());
		cv::Mat background = cv::imread(cv::String(std::format(".\\cache\\{0}.jpg", file_name)));
		cv::resize(image, image, background.size());
		cv::addWeighted(image, 0.5, background, (1.0 - 0.5), 0.0, this->img);
	}
	image& add_line(std::vector<int> pt1, std::vector<int> pt2, std::vector<double> RGBA, int thickness = 1) {
		if (RGBA.size() == 3) RGBA[4] = 240;
		cv::line(this->img, cv::Point(pt1[0], pt1[1]), cv::Point(pt2[0], pt2[1]), cv::Scalar(RGBA[0], RGBA[1], RGBA[2], RGBA[3]), thickness);
	}
	~image() {
		/* this deletes the physical copy and keeps the cloud copy (via discord) */
		std::filesystem::remove(std::filesystem::path(this->path()));
	}
};
