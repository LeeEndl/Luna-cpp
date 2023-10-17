#include <dpp/nlohmann/json.hpp>
#include <dpp/dpp.h>
#include <fstream>

std::vector<std::pair<dpp::snowflake, std::future<void>>> request;
std::vector<std::future<void>> process;
std::vector<std::string> commands = { "!ping", "!kick " };

void message_create(const dpp::message_create_t& event)
{
	if (std::count_if(request.begin(), request.end(), [&](std::pair<dpp::snowflake, std::future<void>>& req) {
		return event.msg.member.user_id == req.first; }) > 1) return;
	std::async(std::launch::async,
		(event.msg.content == "!ping") ? std::function<void()>([&event]()
			{
				event.reply("> :ping_pong: pong! *" + std::to_string(static_cast<int>((bot.rest_ping + bot.get_shard(0)->websocket_ping) * 1000)) + "ms*");
			}) :
		(event.msg.content.find("!kick ") not_eq -1) ? std::function<void()>([&event]()
			{
				std::string id = dpp::utility::index(event.msg.content, ' ')[1];
				bot.user_get(dpp::snowflake(dpp::utility::trim_mention(id)), [&](const dpp::confirmation_callback_t& callback) {
					id = std::to_string(std::get<dpp::user_identified>(callback.value).id);
					if (callback.is_error()) event.reply("> invalid user id");
					else {
						if (event.msg.member.get_user()->get_permission(event.msg.guild_id) & dpp::p_kick_members and 
							~std::get<dpp::user_identified>(callback.value).get_permission(event.msg.guild_id) & dpp::p_administrator)
							bot.guild_member_kick(event.msg.guild_id, dpp::snowflake(stoull(id)));
						else event.reply("> you do not have permission: `Kick Members`, or I am unable to kick this person.");
					}
					});
			}) : std::function<void()>());
}

int main()
{
	process.emplace_back(std::async(std::launch::async, std::function<void()>([]() {
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			if (not request.empty()) request.clear();
		}
		})));
	bot.on_log([&](const dpp::log_t& event) {
		std::cout << event.message << std::endl;
		});
	bot.on_message_create([](const dpp::message_create_t& event) {
		if (std::none_of(commands.begin(), commands.end(), [&](std::string& command) { return event.msg.content.find(command); }) or not event.msg.webhook_id.empty() or
			event.msg.member.get_user()->is_bot()) return;
		request.emplace_back(std::make_pair(event.msg.member.user_id, std::async(std::launch::async, message_create, event)));
		});
	bot.start(dpp::start_type::st_wait);
}