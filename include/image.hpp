#include <opencv2/opencv.hpp>
#include <dpp/message.h>
#include <filesystem>
#define cache_channel 1176057541013282896

namespace rgb {
	const std::vector<double> 
		white{ 255, 255, 255 },
		embeded{ 43, 45, 49 }, 
		discord{ 49,51,56 };
}

class image {
	dpp::snowflake id{};
	cv::Mat img{};
public:
	cv::String path(bool directory = false) {
		if (directory) return cv::String(".\\cache\\");
		else return cv::String(std::format(".\\cache\\{0}.jpg", static_cast<uint64_t>(this->id)));
	}
	std::string raw() {
		return dpp::utility::read_file(this->path().c_str());
	}
	/* creates/overlaps a .jpg image */
	bool image_write(cv::Mat new_img = cv::Mat()) {
		if (not new_img.empty()) this->img = new_img;
		return cv::imwrite(this->path(), this->img);
	}
	std::vector<int> dim() {
		return { this->img.rows, this->img.cols };
	}
	/* 
	 * @param dim y, x e.g. { 100, 100 } 
	 * @param file_name ignore dim and RGBA and import a existing image
	 */
	image(dpp::snowflake id, std::vector<int> dim, std::vector<double> RGBA, std::string file_name = "") {
		if (RGBA.size() == 3) RGBA[4] = 255;
		this->id = id;
		image_write((file_name.empty()) ?
			cv::Mat::zeros(dim[0], dim[1], CV_8UC3) + cv::Scalar(RGBA[0], RGBA[1], RGBA[2], RGBA[3]) :
			cv::imread(cv::String(std::format(".\\cache\\{0}.jpg", file_name))));
	}
	/* adds a image within the original image */
	image& add_image(std::string file_name, std::vector<int> at) {
		try {
			cv::Mat image = this->img;
			cv::Mat background = cv::imread(cv::String(std::format(".\\cache\\{0}.jpg", file_name)));
			cv::Mat object = cv::Mat::zeros(image.size(), image.type()); // -> becomes a object within image.
			cv::resize(background, background, cv::Size(), 0.5, 0.5);
			cv::Rect roi(at[0], at[1], background.cols, background.rows);
			background.copyTo(object(roi));
			cv::addWeighted(image, 1, object, 1, 0.0, this->img);
		}
		catch (cv::Exception e) {
			std::cout << e.what() << std::endl;
		}
	}
	image& add_line(std::vector<int> pt1, std::vector<int> pt2, std::vector<double> RGBA, int thickness = 1) {
		if (RGBA.size() == 3) RGBA[4] = 255;
		cv::line(this->img, 
			cv::Point(std::clamp<int>(pt1[0], 0, this->dim()[1]), std::clamp<int>(pt1[1], 0, this->dim()[0])),
			cv::Point(std::clamp<int>(pt2[0], 0, this->dim()[1]), std::clamp<int>(pt2[1], 0, this->dim()[0])),
			cv::Scalar(RGBA[0], RGBA[1], RGBA[2], RGBA[3]), thickness);
	}
	image& add_text(std::string text, std::vector<int> at, cv::HersheyFonts font, std::vector<double> RGBA, int thickness = 1) {
		if (RGBA.size() == 3) RGBA[4] = 255;
		cv::putText(this->img, cv::String(text), cv::Point(at[0], at[1]), font, 1.0, cv::Scalar(RGBA[0], RGBA[1], RGBA[2], RGBA[3]), thickness);
	}
	~image() {
		/* this deletes the physical copy and keeps the cloud copy (via discord) */
		std::filesystem::remove(std::filesystem::path(this->path()));
	}
};
