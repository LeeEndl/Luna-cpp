#include <dpp/dpp.h>

dpp::cluster bot("TOKEN", dpp::i_all_intents);
std::vector<std::string> commands = { "!ping", "!purge " };
std::unordered_map<dpp::snowflake, std::future<void>> sender;

void release_command(const dpp::message_create_t& event)
{
	std::promise<dpp::message> p_reply;
	event.reply("> ...", false, [&](const dpp::confirmation_callback_t& callback) {
		p_reply.set_value(std::get<dpp::message>(callback.value));
		});
	dpp::message reply = p_reply.get_future().get();
	std::async(std::launch::async,
		(event.msg.content == "!ping") ? std::function<void()>([&event, &reply]()
			{
				bot.message_edit(reply.set_content("> :ping_pong: Pong! *" + std::to_string(static_cast<int>(bot.rest_ping * 1000)) + "ms*"));
			}) :
		(event.msg.content.find("!purge ") not_eq -1) ? std::function<void()>([&event, &reply]()
			{
				if (event.msg.member.get_user()->get_permission(event.msg.guild_id) & dpp::p_manage_messages)
				{
					std::string& provided = dpp::utility::index(event.msg.content, ' ')[1];
					int amount = (std::ranges::all_of(provided, ::isdigit)) ? stoi(dpp::utility::index(event.msg.content, ' ')[1]) : 0;
					bot.messages_get(event.msg.channel_id, 0, event.msg.id, 0, std::clamp(amount, 1, 100), [&](const dpp::confirmation_callback_t& callback) {
						bot.message_delete_bulk(std::get<dpp::message_map>(callback.value), event.msg.channel_id);
						bot.message_edit(reply.set_content("> Deleted **" + std::to_string(std::get<dpp::message_map>(callback.value).size()) + "** messages"));
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
