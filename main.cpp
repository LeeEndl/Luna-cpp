#include <dpp/dpp.h>
#include <coroutine>
std::unique_ptr<dpp::cluster> bot = std::make_unique<dpp::cluster>("MTAwNDUxNDkzNTA1OTAwNTQ3MA.______.", dpp::i_all_intents);
std::unique_ptr<std::unordered_map<dpp::snowflake, std::future<void>>> cmd_sender = std::make_unique<std::unordered_map<dpp::snowflake, std::future<void>>>();
std::unique_ptr<std::unordered_map<dpp::snowflake, std::future<void>>> btn_sender = std::make_unique<std::unordered_map<dpp::snowflake, std::future<void>>>();

struct giveaway {
	time_t ends;
	dpp::snowflake host;
	std::vector<dpp::snowflake> entries;
	uint64_t winners;
}; std::unique_ptr<std::unordered_map<size_t, giveaway>> _giveaway = std::make_unique<std::unordered_map<size_t, giveaway>>();

void button_pressed(const dpp::button_click_t& event) {
	std::async(std::launch::async, std::function<void()>([&event]()
		{
			std::unique_ptr<std::vector<std::string>> i = dpp::utility::index(event.custom_id, '.');
			if (i->at(0) == "giveaway") {
				giveaway gw = _giveaway->at(stoull(i->at(1)));
				if (std::ranges::find(gw.entries, event.command.member.user_id) not_eq gw.entries.end())
					event.reply(dpp::message("> You have already entered this giveaway!").set_flags(dpp::message_flags::m_ephemeral));
				else gw.entries.emplace_back(event.command.member.user_id);
				_giveaway->at(stoull(i->at(1))) = gw;
				event.reply();
			}
		}));
	std::this_thread::sleep_for(std::chrono::seconds(1));
	btn_sender->erase(event.command.member.user_id);
}

void release_command(const dpp::slashcommand_t& lam)
{
	std::unique_ptr<dpp::slashcommand_t> event = std::make_unique<dpp::slashcommand_t>(std::move(lam));
	std::async(std::launch::async, std::function<void()>([&event]()
		{
			if (event->command.get_command_name() == "purge" and event->command.member.get_user()->get_permission(event->command.guild_id) & dpp::p_manage_messages) {
				event->reply(dpp::message(event->command.channel_id, dpp::embed().set_description("> ...")));
				bot->messages_get(event->command.channel_id, 0, event->command.id, 0, std::clamp((int)get<int64_t>(event->get_parameter("amount")), 1, 100), [&](const dpp::confirmation_callback_t& mg_cb) {
					bot->message_delete_bulk(std::move(std::get<dpp::message_map>(mg_cb.value)), event->command.channel_id, [&event, mg_cb](const dpp::confirmation_callback_t& mdb_cb) {
						std::unique_ptr<std::string> what = std::make_unique<std::string>(mdb_cb.is_error() ?
							std::format("> {0}", mdb_cb.get_error().message) :
							std::format("> Deleted **{0}** messages", std::get<dpp::message_map>(mg_cb.value).size()));
						event->edit_original_response(dpp::message(std::move(*what)).set_flags(dpp::message_flags::m_ephemeral));
						});
					});
			}
			if (event->command.get_command_name() == "gcreate") {
				giveaway gw = { time(0), event->command.member.user_id, {}, 1 };
				bot->message_create(dpp::message(event->command.channel_id, dpp::embed()
					.set_title("prize title")
					.set_description(std::format("prize desc. \n\nEnds: {0} \nHosted by: <@{1}> \nEntries: {2} \nWinners: {3}",
						dpp::utility::timestamp(gw.ends, dpp::utility::tf_short_datetime), (uint64_t)gw.host, gw.entries.size(), gw.winners))
					.set_timestamp(time(0)))
					.add_component(dpp::component().add_component(dpp::component()
						.set_emoji(u8"🎉").set_id(std::format(".giveaway.{0}", _giveaway->size())))));
				event->reply(dpp::message(std::format("> The giveaway was successfully created! ID: **{0}**", _giveaway->size())).set_flags(dpp::m_ephemeral));
				_giveaway->emplace(_giveaway->size(), gw);
			}
		}));
	std::this_thread::sleep_for(std::chrono::seconds(1));
	cmd_sender->erase(event->command.member.user_id);
}

int main()
{
	bot->on_log(dpp::utility::cout_logger());
	bot->on_ready([](const dpp::ready_t& event) {
		std::unique_ptr<std::vector<dpp::slashcommand>> cmds = std::make_unique<std::vector<dpp::slashcommand>>();
		cmds->emplace_back(dpp::slashcommand().set_name("purge").set_description("mass delete messages")
			.add_option(dpp::command_option(dpp::co_integer, "amount", "amount of messages to delete", true)));
		cmds->emplace_back(dpp::slashcommand().set_name("gcreate").set_description("create a giveaway"));

		bot->global_bulk_command_create(std::move(*cmds));
		});
	bot->on_slashcommand([](const dpp::slashcommand_t& event) {
		if (cmd_sender->contains(event.command.member.user_id)) return;
		cmd_sender->emplace(event.command.member.user_id, std::async(std::launch::async, release_command, event));
		});
	bot->on_button_click([](const dpp::button_click_t& event) {
		if (btn_sender->contains(event.command.member.user_id)) return;
		btn_sender->emplace(event.command.member.user_id, std::async(std::launch::async, button_pressed, event));
		});
	bot->start(dpp::start_type::st_wait);
}
