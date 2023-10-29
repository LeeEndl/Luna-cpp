#include <dpp/dpp.h>

dpp::cluster bot("TOKEN", dpp::i_all_intents);
std::vector<std::future<void>> request;
std::vector<dpp::snowflake> ratelimit;
std::vector<std::string> commands = { "!ping", "!purge " };

void release_command(const dpp::message_create_t& event)
{
	ratelimit.emplace_back(event.msg.member.user_id);
	event.reply("> ...", false, [&](const dpp::confirmation_callback_t& reply) {
		std::async(std::launch::async,
		(event.msg.content == "!ping") ? std::function<void()>([&event, &reply]()
			{
				try {
					dpp::message msg = std::get<dpp::message>(reply.value);
					bot.message_edit(msg.set_content("> :ping_pong: pong! *" + std::to_string(static_cast<int>(bot.rest_ping * 1000)) + "ms*"));
				}
				catch (dpp::exception exc) {
					std::cout << exc.what() << std::endl;
				}
			}) :
			(event.msg.content.find("!purge ") not_eq -1) ? std::function<void()>([&event, &reply]()
				{
					dpp::message msg = std::get<dpp::message>(reply.value);
					if (event.msg.member.get_user()->get_permission(event.msg.guild_id) & dpp::p_manage_messages)
					{
						std::string& provided = dpp::utility::index(event.msg.content, ' ')[1];
						int amount = (std::ranges::all_of(provided, ::isdigit)) ? stoi(dpp::utility::index(event.msg.content, ' ')[1]) : 0;
						bot.messages_get(event.msg.channel_id, 0, 0, 0, std::clamp(amount, 1, 100), [&](const dpp::confirmation_callback_t& callback) {
							std::vector<dpp::snowflake> message_ids;
							for (auto& [id, msg] : std::get<dpp::message_map>(callback.value))
								if (msg.sent - (static_cast<time_t>(60 * 60 * 24) * 14) <= std::time(0)) message_ids.emplace_back(id);
							(message_ids.size() < 1) ?
								bot.message_edit(msg.set_content("> there are no messages within this channel, or there `14 days old`")) :
								bot.message_edit(msg.set_content("> deleted " + std::to_string(message_ids.size()) + " messages")),
								bot.message_delete_bulk(message_ids, event.msg.channel_id);
							});
					}
				}) : std::function<void()>());
		});
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ratelimit.erase(std::remove_if(ratelimit.begin(), ratelimit.end(), [&event](const auto& id) {
		return id == event.msg.member.user_id;
		}), ratelimit.end());
}

int main()
{
	bot.on_log([&](const dpp::log_t& event) {
		std::cout << event.message << std::endl;
		});
	bot.on_message_create([](const dpp::message_create_t& event) {
		if (std::ranges::none_of(commands, [&](std::string& command) { return event.msg.content.find(command) not_eq -1; }) or
			not event.msg.webhook_id.empty() or event.msg.member.get_user()->is_bot() or
			std::ranges::count_if(ratelimit, [&](dpp::snowflake& id) { return id == event.msg.member.user_id; }) > 1) return;
		request.emplace_back(std::async(std::launch::async, release_command, event));
		});
	bot.start(dpp::start_type::st_wait);
}
