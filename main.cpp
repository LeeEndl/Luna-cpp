#include <dpp/nlohmann/json.hpp>
#include <dpp/dpp.h>
#include <fstream>

std::vector<std::pair<dpp::snowflake, std::future<void>>> request;
std::vector<std::future<void>> process;
std::vector<std::string> commands = { "!ping", "!kick " };

/**
	* @brief Get a user's permissions
	* @param guild_id permissions in a guild
	* @param user_id of whoms permissions
*/
dpp::permission user_get_permission(dpp::snowflake guild_id, dpp::snowflake user_id) {
	return dpp::find_guild(guild_id)->base_permissions(dpp::find_user(user_id));
}

void message_create(const dpp::message_create_t& event)
{
	if (std::count_if(request.begin(), request.end(), [&](std::pair<dpp::snowflake, std::future<void>>& req) {
		return event.msg.member.user_id == req.first;
		}) > 1) return;
	std::async(std::launch::async,
		(event.msg.content == "!ping") ? std::function<void()>([&event]()
			{
				event.reply("> :ping_pong: pong! *" + std::to_string(bot.rest_ping * 100) + "ms*");
			}) :
		(event.msg.content.find("!kick ") not_eq -1) ? std::function<void()>([&event]()
			{
				std::string id = dpp::utility::index(event.msg.content, ' ')[1];
				bot.user_get(dpp::snowflake(dpp::utility::trim_mention(id)), [&](const dpp::confirmation_callback_t& callback) {
					id = std::to_string(std::get<dpp::user_identified>(callback.value).id);
					if (callback.is_error()) event.reply("> invalid user id");
					else {
						if (user_get_permission(event.msg.guild_id, event.msg.member.user_id) & dpp::p_kick_members)
							bot.guild_member_kick(event.msg.guild_id, dpp::snowflake(stoull(id)));
						else event.reply("> you do not have permission: `Kick Members`");
					}
					});
			}) : std::function<void()>());
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