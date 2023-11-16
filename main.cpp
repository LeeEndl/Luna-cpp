#include <dpp/dpp.h>
using namespace std::chrono;
std::unique_ptr<dpp::cluster> bot = std::make_unique<dpp::cluster>("MTAwNDUxNDkzNTA1OTAwNTQ3MA.______.", dpp::i_all_intents);
std::unique_ptr<std::unordered_map<dpp::snowflake, std::future<void>>> cmd_sender = std::make_unique<std::unordered_map<dpp::snowflake, std::future<void>>>();
std::unique_ptr<std::unordered_map<dpp::snowflake, std::future<void>>> btn_sender = std::make_unique<std::unordered_map<dpp::snowflake, std::future<void>>>();
std::unique_ptr<std::vector<std::future<void>>> active_code = std::make_unique<std::vector<std::future<void>>>();

struct giveaway {
	std::string title{}, description{};
	time_t ends{};
	int64_t winners{};
	dpp::snowflake host{};
	std::vector<dpp::snowflake> entries{}, sub_entries{};
	dpp::message message{};

	void message_update(std::string winners = "") {
		this->message.embeds[0]
			.set_description(std::format("{0} \n\nEnd{1}: {2} ({3}) \nHosted by: <@{4}> \nEntries: **{5}** \nWinners: **{6}**",
				this->description, (system_clock::from_time_t(this->ends) <= system_clock::now()) ? "ed" : "s",
				dpp::utility::timestamp(this->ends, dpp::utility::tf_relative_time), dpp::utility::timestamp(this->ends, dpp::utility::tf_short_datetime),
				(uint64_t)this->host, this->entries.size(), (winners.empty()) ? std::to_string(this->winners) : winners));
		bot->message_edit(this->message);
	}
};
std::unique_ptr<std::unordered_map<size_t, giveaway>> _giveaway = std::make_unique<std::unordered_map<size_t, giveaway>>();
std::function<void(size_t id)> pending_giveaway = [](size_t id)
	{
		std::unique_ptr<giveaway> gw = std::make_unique<giveaway>(_giveaway->at(id));
		std::this_thread::sleep_until(system_clock::from_time_t(gw->ends));
		gw = std::make_unique<giveaway>(_giveaway->at(id));
		std::string winners = "";
		gw->message.components[0].components[0].set_disabled(true);
		gw->sub_entries = gw->entries;
		if (gw->entries.size() < gw->winners)
			gw->winners = std::clamp((int)gw->winners, 1, (int)gw->entries.size());
		if (not gw->entries.empty())
			for (int64_t i = 0; i < gw->winners; i++)
			{
				size_t result = dpp::utility::rand<size_t>(0, gw->sub_entries.size() - 1);
				winners += std::format("<@{0}>, ", (uint64_t)gw->sub_entries[result]);
				gw->sub_entries.erase(std::ranges::find(gw->sub_entries, gw->sub_entries[result]));
			}
		if (winners.size() > 2) winners.resize(winners.size() - 2);
		gw->message_update(winners);
		_giveaway->erase(id);
	};

void button_pressed(std::unique_ptr<dpp::button_click_t> event) {
	std::unique_ptr<std::vector<std::string>> i = dpp::utility::index(event->custom_id, '.');
	if (i->at(0) == "giveaway") {
		std::unique_ptr<giveaway> gw = std::make_unique<giveaway>(_giveaway->at(stoull(i->at(1))));
		if (std::ranges::find(gw->entries, event->command.member.user_id) not_eq gw->entries.end())
			event->reply(std::make_unique<dpp::message>("> You have already entered this giveaway!")
				->set_flags(dpp::m_ephemeral));
		else
		{
			gw->entries.emplace_back(event->command.member.user_id);
			_giveaway->at(stoull(i->at(1))) = *gw;
			gw->message_update();
			event->reply();
		}
	}
	std::this_thread::sleep_for(1s);
	btn_sender->erase(event->command.member.user_id);
}

void release_command(std::unique_ptr<dpp::slashcommand_t> event)
{
	if (event->command.get_command_name() == "purge")
	{
		event->reply(std::make_unique<dpp::message>("> *Deleting..*")
			->set_flags(dpp::m_ephemeral));
		bot->messages_get(event->command.channel_id, 0, event->command.id, 0, std::clamp((int)get<int64_t>(event->get_parameter("amount")), 1, 100),
			[&event](const dpp::confirmation_callback_t& mg_cb)
			{
				bot->message_delete_bulk(std::move(std::get<dpp::message_map>(mg_cb.value)), event->command.channel_id,
				[&event, mg_cb](const dpp::confirmation_callback_t& mdb_cb)
					{
						std::string what = mdb_cb.is_error() ?
							std::format("> {0}", mdb_cb.get_error().message) :
							std::format("> Deleted **{0}** messages", std::get<dpp::message_map>(mg_cb.value).size());
						event->edit_original_response(std::make_unique<dpp::message>(std::move(what))
							->set_flags(dpp::m_ephemeral));
					});
			});
	}
	if (event->command.get_command_name() == "gcreate")
	{
		giveaway gw = {
			get<std::string>(event->get_parameter("title")), get<std::string>(event->get_parameter("description")),
			dpp::utility::string_to_time(get<std::string>(event->get_parameter("duration"))), get<int64_t>(event->get_parameter("winners")),
			event->command.member.user_id, {}
			};
		if (system_clock::from_time_t(gw.ends) <= system_clock::now())
			event->reply(std::make_unique<dpp::message>(std::format("> invalid duration. e.g. **1h, 30m**", _giveaway->size()))
				->set_flags(dpp::m_ephemeral));
		else
		{
			bot->message_create(std::make_unique<dpp::message>(event->command.channel_id,
				std::make_unique<dpp::embed>()
				->set_color(dpp::colors::outter)
				.set_title(std::format("{0}", gw.title)))
				->add_component(std::make_unique<dpp::component>()->add_component(std::make_unique<dpp::component>()
					->set_emoji(u8"🎉")
					.set_id(std::format(".giveaway.{0}",
						_giveaway->size())))),
				[&event, &gw](const dpp::confirmation_callback_t& mc_cb)
				{
					gw.message = std::move(*std::make_unique<dpp::message>(std::get<dpp::message>(mc_cb.value)));
					gw.description = std::move(get<std::string>(event->get_parameter("description"))); // TODO
					gw.message_update();
					active_code->emplace_back(std::async(std::launch::async, pending_giveaway, _giveaway->size()));
					_giveaway->emplace(_giveaway->size(), gw);
				});
			event->reply(std::make_unique<dpp::message>(std::format("> The giveaway was successfully created! ID: **{0}**", _giveaway->size()))
				->set_flags(dpp::m_ephemeral));
		}
	}
	std::this_thread::sleep_for(1s);
	cmd_sender->erase(event->command.member.user_id);
}

struct command_info {
	std::string name{}, description{};
	dpp::permissions permission{};
	std::vector<dpp::command_option> options{};
};

int main()
{
	// TODO: JSONify _giveaway unordered_map
	bot->on_ready([](const dpp::ready_t& event)
		{
			std::unique_ptr<std::vector<dpp::slashcommand>> cmds = std::make_unique<std::vector<dpp::slashcommand>>();
			std::unique_ptr<std::vector<command_info>> _command_info = std::make_unique<std::vector<command_info>>(std::vector<command_info>{
				{
					"purge", "mass delete messages", dpp::p_manage_messages,
					{
						{dpp::command_option(dpp::co_integer, "amount", "amount of messages to delete", true).set_max_value(100).set_min_value(1)}
					}
				},
				{
					"gcreate", "create a giveaway", dpp::p_administrator,
					{
						{dpp::command_option(dpp::co_string, "title", "what you're giveawaying", true)},
						{dpp::command_option(dpp::co_string, "description", "describe the giveaway", true)},
						{dpp::command_option(dpp::co_string, "duration", "the length of the giveaway e.g. 1h, 30m", true)},
						{dpp::command_option(dpp::co_integer, "winners", "amount of winners", true).set_min_value(1)}
					}
				}});
			for (auto& [name, description, permission, options] : std::move(*_command_info)) {
				dpp::slashcommand cmd_handler;
				cmd_handler.set_name(name).set_description(description).set_default_permissions(permission);
				for (const auto& option : options) cmd_handler.add_option(option);
				cmds->emplace_back(cmd_handler);
			}
			bot->global_bulk_command_create(std::move(*cmds));
		});
	bot->on_slashcommand([](const dpp::slashcommand_t& event)
		{
			std::unique_ptr<dpp::slashcommand_t> _event = std::make_unique<dpp::slashcommand_t>(std::move(event));
			if (cmd_sender->contains(event.command.member.user_id)) return;
			cmd_sender->try_emplace(event.command.member.user_id, std::async(std::launch::async, release_command, std::move(_event)));
		});
	bot->on_button_click([](const dpp::button_click_t& event)
		{
			std::unique_ptr<dpp::button_click_t> _event = std::make_unique<dpp::button_click_t>(std::move(event));
			if (btn_sender->contains(event.command.member.user_id)) return;
			btn_sender->try_emplace(event.command.member.user_id, std::async(std::launch::async, button_pressed, std::move(_event)));
		});
	bot->on_log(dpp::utility::cout_logger());
	bot->start(dpp::start_type::st_wait);
}
