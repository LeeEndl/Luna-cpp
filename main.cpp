#include <dpp/nlohmann/json.hpp>
#include <dpp/dpp.h>
#include <fstream>

std::vector<std::pair<dpp::snowflake, std::future<void>>> request;
std::vector<std::future<void>> process;
std::vector<std::string> commands = { "!ping" };

void message_create(const dpp::message_create_t& event)
{
	uint64_t i = std::count_if(request.begin(), request.end(), [&](std::pair<dpp::snowflake, std::future<void>>& req) {
		return event.msg.member.user_id == req.first;
		});
	if (i >= 2) return;
	std::function<void()> response =
		(event.msg.content == "!ping") ? std::function<void()>([&]()
			{
				event.reply("> :ping_pong: pong! *" + std::to_string(bot.rest_ping * 100) + "ms*");
			}) : std::function<void()>();

	std::async(std::launch::async, response);
}

void clear_request()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (not request.empty()) request.clear();
	}
}

int main()
{
	process.emplace_back(std::async(std::launch::async, clear_request));
	bot.on_log([&](const dpp::log_t& event) {
		std::cout << event.message << std::endl;
		});
	bot.on_message_create([](const dpp::message_create_t& event) {
		if (not std::any_of(commands.begin(), commands.end(), [&](std::string& command) { return command == event.msg.content; }) or not event.msg.webhook_id.empty() or 
			event.msg.member.get_user()->is_bot()) return;
		request.emplace_back(std::make_pair(event.msg.member.user_id, std::async(std::launch::async, message_create, event)));
		});
	bot.start(dpp::start_type::st_wait);
}