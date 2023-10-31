#include <dpp/dpp.h>

dpp::cluster bot("TOKEN", dpp::i_all_intents);
std::vector<std::string> commands = { "!ping", "!purge " };
std::unordered_map<dpp::snowflake, std::future<void>> sender;

void release_command(const dpp::message_create_t& event)
{
	dpp::message reply;
	event.reply("> ...", false, [&](const dpp::confirmation_callback_t& callback) {
		reply = std::get<dpp::message>(callback.value);
		});
	std::async(std::launch::async,
		(event.msg.content == "!ping") ? std::function<void()>([&event, &reply]()
			{
				bot.message_edit(reply.set_content("> :ping_pong: pong! *" + std::to_string(static_cast<int>(bot.rest_ping * 1000)) + "ms*"));
			}) :
		(event.msg.content.find("!purge ") not_eq -1) ? std::function<void()>([&event, &reply]()
			{
				if (event.msg.member.get_user()->get_permission(event.msg.guild_id) & dpp::p_manage_messages)
				{
					std::string& provided = dpp::utility::index(event.msg.content, ' ')[1];
					int amount = (std::ranges::all_of(provided, ::isdigit)) ? stoi(dpp::utility::index(event.msg.content, ' ')[1]) : 0;
					bot.messages_get(event.msg.channel_id, 0, reply.id, 0, std::clamp(amount, 1, 100), [&](const dpp::confirmation_callback_t& callback) {
						std::vector<dpp::snowflake> message_ids;
						for (auto& [id, msg] : std::get<dpp::message_map>(callback.value))
							if (msg.sent - (static_cast<time_t>(60 * 60 * 24) * 14) <= std::time(0)) message_ids.emplace_back(id);
						(message_ids.size() < 1) ?
							bot.message_edit(reply.set_content("> there are no messages within this channel, or there `14 days old`")) :
							bot.message_edit(reply.set_content("> deleted **" + std::to_string(message_ids.size()) + "** messages")),
							bot.message_delete_bulk(message_ids, event.msg.channel_id);
						});
				}
			}) : std::function<void()>());
	std::this_thread::sleep_for(std::chrono::seconds(1));
	sender.erase(event.msg.member.user_id);
}

int main()
{
	bot.on_log(dpp::utility::cout_logger());
	bot.on_message_create([](const dpp::message_create_t& event) {
		if (std::ranges::none_of(commands, [&](std::string& command) { return event.msg.content.find(command) not_eq -1; }) or
			not event.msg.webhook_id.empty() or event.msg.member.get_user()->is_bot()) return;
		sender.emplace(event.msg.member.user_id, std::async(std::launch::async, release_command, event));
		});
	bot.start(dpp::start_type::st_wait);
}
