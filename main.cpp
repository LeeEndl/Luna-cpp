#include <dpp/dpp.h>
std::unique_ptr<dpp::cluster> bot = std::make_unique<dpp::cluster>("TOKEN", dpp::i_all_intents);
std::unique_ptr<std::unordered_map<dpp::snowflake, std::future<void>>> cmd_sender = std::make_unique<std::unordered_map<dpp::snowflake, std::future<void>>>();
std::unique_ptr<std::unordered_map<dpp::snowflake, std::future<void>>> button_sender = std::make_unique<std::unordered_map<dpp::snowflake, std::future<void>>>();

void button_pressed(const dpp::button_click_t& event) {

	std::this_thread::sleep_for(std::chrono::seconds(1));
	button_sender->erase(event.command.member.user_id);
}

std::vector<std::string> commands = { "!purge " };
void release_command(const dpp::message_create_t& event)
{
	std::promise<dpp::message> p_reply;
	event.reply(dpp::message(event.msg.channel_id, dpp::embed().set_description("> ...")), false, [&](const dpp::confirmation_callback_t& callback) {
		p_reply.set_value(std::get<dpp::message>(callback.value));
		});
	dpp::message reply = p_reply.get_future().get();
	std::async(std::launch::async, std::function<void()>([&event, &reply]()
		{
			if (event.msg.content.find(commands[0]) not_eq -1 and event.msg.member.get_user()->get_permission(event.msg.guild_id) & dpp::p_manage_messages) {
				std::string& provided = dpp::utility::index(event.msg.content, ' ')[1];
				int amount = (std::ranges::all_of(provided, ::isdigit)) ? stoi(dpp::utility::index(event.msg.content, ' ')[1]) : 0;
				bot->messages_get(event.msg.channel_id, 0, event.msg.id, 0, std::clamp(amount, 1, 100), [&](const dpp::confirmation_callback_t& callback) {
					bot->message_delete_bulk(std::get<dpp::message_map>(callback.value), event.msg.channel_id, [&event, callback, &reply](const dpp::confirmation_callback_t& callback_) {
						(callback_.is_error()) ?
							reply.embeds[0].set_description(std::format("> {0}", callback_.get_error().message)) :
							reply.embeds[0].set_description(std::format("> Deleted **{0}** messages", std::get<dpp::message_map>(callback.value).size()));
						bot->message_edit(reply);
						});
					});
			}
		}));
	std::this_thread::sleep_for(std::chrono::seconds(1));
	cmd_sender->erase(event.msg.member.user_id);
}

int main()
{
	bot->on_log(dpp::utility::cout_logger());
	bot->on_message_create([](const dpp::message_create_t& event) {
		if (std::ranges::none_of(commands, [&event](const std::string& command) { return event.msg.content.find(command) not_eq -1; }) or
			std::ranges::count_if(*cmd_sender, [&event](const auto& element) { return element.first == event.msg.member.user_id; }) or
			not event.msg.webhook_id.empty() or event.msg.member.get_user()->is_bot()) return;
		cmd_sender->emplace(event.msg.member.user_id, std::async(std::launch::async, release_command, event));
		});
	bot->on_button_click([](const dpp::button_click_t& event) {
		if (std::ranges::count_if(*button_sender, [&event](const auto& element) { return element.first == event.command.member.user_id; })) return;
		button_sender->emplace(event.command.member.user_id, std::async(std::launch::async, button_pressed, event));
		});
	bot->start(dpp::start_type::st_wait);
}
