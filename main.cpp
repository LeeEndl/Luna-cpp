#include <dpp/nlohmann/json.hpp>
#include <dpp/dpp.h>
#include <fstream>

std::vector<std::pair<dpp::snowflake, std::future<void>>> request;
std::vector<std::future<void>> process;
std::vector<std::string> commands = { "!ping", "!kick " };

std::vector<std::string> index(std::string source, const char& find)
{
	std::string temp = "";
	std::vector<std::string> i;
	for (auto c : source) c not_eq find ? temp += c : c == find && not temp.empty() ? i.push_back(temp), temp = "" : "";
	if (not temp.empty()) i.push_back(temp);
	return i;
}

std::string trim_mention(std::string str) {
	std::string i = str;
	i.erase(std::remove_if(i.begin(), i.end(), [&](const char& c) { return c == '<' or c == '>' or c == '!' or c == '@'; }), i.end());
	return i;
}

void message_create(const dpp::message_create_t& event)
{
	uint64_t i = std::count_if(request.begin(), request.end(), [&](std::pair<dpp::snowflake, std::future<void>>& req) {
		return event.msg.member.user_id == req.first;
		});
	if (i >= 2) return;
	std::async(std::launch::async,
		(event.msg.content == "!ping") ? std::function<void()>([&event]()
			{
				event.reply("> :ping_pong: pong! *" + std::to_string(bot.rest_ping * 100) + "ms*");
			}) :
		(event.msg.content.find("!kick ") not_eq -1) ? std::function<void()>([&event]()
			{
				std::string id = index(event.msg.content, ' ')[1];
				bot.user_get(dpp::snowflake(trim_mention(id)), [&](const dpp::confirmation_callback_t& callback) {
					id = std::to_string(std::get<dpp::user_identified>(callback.value).id);
					if (callback.is_error()) event.reply("> invalid user id");
					else {
						if (dpp::find_guild(event.msg.guild_id)->base_permissions(dpp::find_user(event.msg.member.user_id)) & dpp::p_kick_members)
							bot.guild_member_kick(event.msg.guild_id, dpp::snowflake(stoull(id)));
						else event.reply("> you do not have permission: `Kick Members`");
					}
					});
			})
				: std::function<void()>());
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
		if (std::none_of(commands.begin(), commands.end(), [&](std::string& command) { return event.msg.content.find(command); }) or not event.msg.webhook_id.empty() or
			event.msg.member.get_user()->is_bot()) return;
		request.emplace_back(std::make_pair(event.msg.member.user_id, std::async(std::launch::async, message_create, event)));
		});
	bot.start(dpp::start_type::st_wait);
}