#include <opencv2/opencv.hpp>
#include <dpp/message.h>
#include <filesystem>
#include <random>
#define cache_channel 1176057541013282896

class image {
	dpp::snowflake id{};
	cv::Mat img{};
public:
	bool update(cv::Mat new_img = cv::Mat()) {
		if (not new_img.empty()) this->img = new_img;
		return cv::imwrite(cv::String(std::format(".\\cache\\{0}.jpg", static_cast<uint64_t>(this->id))), this->img);
	}
	/* @param dem y, x e.g. { 100, 100 } */
	image(dpp::snowflake id, std::vector<int> dem) {
		this->id = id;
		update(cv::Mat::zeros(dem[0], dem[1], CV_8UC3) + cv::Scalar(100, 100, 100, 100));
	}
	std::string raw() {
		return dpp::utility::read_file(std::format(".\\cache\\{0}.jpg", static_cast<uint64_t>(this->id)));
	}
	std::string path() {
		return std::format(".\\cache\\{0}.jpg", static_cast<uint64_t>(this->id));
	}
	image& background(std::string file_name) {
		cv::Mat image = cv::imread(cv::String(this->path()));
		cv::Mat background = cv::imread(cv::String(std::format(".\\cache\\{0}.jpg", file_name)));
		cv::resize(image, image, background.size());
		cv::addWeighted(image, 0.5, background, (1.0 - 0.5), 0.0, this->img);
		update();
	}
	image& add_line(std::vector<int> pt1, std::vector<int> pt2, std::vector<double> RGB, int thickness = 1) {
		cv::line(this->img, cv::Point(pt1[0], pt1[1]), cv::Point(pt2[0], pt2[1]), cv::Scalar(RGB[0], RGB[1], RGB[2]), thickness);
		update();
	}
	~image() {
		/* this deletes the physical copy and keeps the cloud copy (via discord) */
		std::filesystem::remove(std::format(".\\cache\\{0}.jpg", static_cast<uint64_t>(this->id)));
	}
};


std::unique_ptr<std::vector<std::string>> index(const std::string& source, const char& find)
{
	std::unique_ptr<std::vector<std::string>> i = std::make_unique<std::vector<std::string>>();
	std::string_view preview(source);
	size_t pos = 0;
	while ((pos = preview.find(find)) not_eq -1) {
		if (pos not_eq 0) i->emplace_back(std::string(preview.substr(0, pos)));
		preview.remove_prefix(pos + 1);
	}
	if (not preview.empty()) i->emplace_back(std::string(preview));
	return std::move(i);
}
template<typename T> T rand(T min, T max) {
	std::uniform_int_distribution<T> integer(min, max);
	std::default_random_engine random;
	return integer(random);
}
time_t string_to_time(std::string str) {
	try
	{
		time_t t = time(0);
		str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
		std::unique_ptr<std::vector<std::string>> is = index(str, ','); // TODO: make it so ',' are not needed. 1h, 30m -> 1h 30m
		for (std::string& i : std::move(*is)) {
			if (std::ranges::find(i | std::views::reverse, 's') not_eq i.rend()) t += std::stoull(dpp::rtrim(i));
			if (std::ranges::find(i | std::views::reverse, 'm') not_eq i.rend()) t += std::stoull(dpp::rtrim(i)) * 60;
			if (std::ranges::find(i | std::views::reverse, 'h') not_eq i.rend()) t += std::stoull(dpp::rtrim(i)) * 60 * 60;
			if (std::ranges::find(i | std::views::reverse, 'd') not_eq i.rend()) t += std::stoull(dpp::rtrim(i)) * 60 * 60 * 24;
		}
		return t;
	}
	catch (...) {
		return time(0);
	}
}
