#include <dpp/nlohmann/json.hpp>
#include <dpp/dpp.h>

std::vector<std::pair<dpp::snowflake, std::future<void>>> request;
std::vector<std::future<void>> process;

void message_create(const dpp::message_create_t& event)
{
	{
		uint64_t i = std::count_if(request.begin(), request.end(), [&](std::pair<dpp::snowflake, std::future<void>>& req) {
			return event.msg.member.user_id == req.first;
			});
		if (i >= 2) return;
	}
}

void clear_request()
{
	while (true)
		if (not request.empty())
			std::this_thread::sleep_for(std::chrono::milliseconds(900)), request.clear();
}

int main()
{
	process.emplace_back(std::async(std::launch::async, clear_request));
	bot.on_log([&](const dpp::log_t& event) {
		std::cout << event.message << std::endl;
		});
	bot.on_message_create([](const dpp::message_create_t& event) {
		if (event.msg.is_dm() or not event.msg.webhook_id.empty() or event.msg.member.get_user()->is_bot()) return;
		request.emplace_back(std::make_pair(event.msg.member.user_id, std::async(std::launch::async, message_create, event)));
		});

	bot.start(dpp::start_type::st_wait);
}