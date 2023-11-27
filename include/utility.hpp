#include <random> // random engine
#include <dpp/stringops.h> // dpp::rtrim()
#include <ranges> // std::ranges::
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

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
	static thread_local std::default_random_engine random(std::random_device{}());
	return std::uniform_int_distribution<T>(min, max)(random);
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
std::wstring to_wstring(std::string str) {
	std::wstring temp = std::wstring(str.begin(), str.end());
	return temp;
}