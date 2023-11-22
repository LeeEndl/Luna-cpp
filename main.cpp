#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <image.hpp>
#include <utility.hpp>
using namespace std::chrono;
std::unique_ptr<dpp::cluster> bot = std::make_unique<dpp::cluster>("MTAwNDUxNDkzNTA1OTAwNTQ3MA.______.", dpp::i_all_intents);
std::unordered_map<dpp::snowflake, std::future<void>> cmd_sender;
std::unordered_map<dpp::snowflake, std::future<void>> btn_sender;
std::vector<std::future<void>> active_code;

struct giveaway {
	std::string description{};
	uint64_t ends{}, winners{}, host{};
	std::vector<uint64_t> entries{}, sub_entries{};
	dpp::message message{};
	void message_update(std::string winners = "") {
		this->message.embeds[0]
			.set_description(std::format("{0} \n\nEnd{1}: {2} ({3}) \nHosted by: <@{4}> \nEntries: **{5}** \nWinners: **{6}**",
				this->description, (system_clock::from_time_t(this->ends) <= system_clock::now()) ? "ed" : "s",
				dpp::utility::timestamp(this->ends, dpp::utility::tf_relative_time), dpp::utility::timestamp(this->ends, dpp::utility::tf_short_datetime),
				this->host, this->entries.size(), (winners.empty()) ? std::to_string(this->winners) : winners));
		bot->message_edit(this->message);
		std::ofstream{ std::format(".\\giveaways\\{0}", static_cast<uint64_t>(this->message.id)) } << this->to_json();
	}
	nlohmann::json to_json() const {
		return {
		{"desc", this->description},
		{"ends", this->ends},
		{"w", this->winners},
		{"h", this->host},
		{"e", this->entries},
		{"m_id", static_cast<uint64_t>(this->message.id)},
		{"m_cid", static_cast<uint64_t>(this->message.channel_id)} };
	}
};
std::unique_ptr<std::unordered_map<dpp::snowflake, giveaway>> _giveaway = std::make_unique<std::unordered_map<dpp::snowflake, giveaway>>();
std::function<void(dpp::snowflake id)> pending_giveaway = [](dpp::snowflake id)
	{
		std::unique_ptr<giveaway> gw = std::make_unique<giveaway>(_giveaway->at(id));
		std::this_thread::sleep_until(system_clock::from_time_t(gw->ends));
		gw.reset(new giveaway(_giveaway->at(id)));
		gw->message.components[0].components[0].set_disabled(true);
		gw->winners = std::clamp(static_cast<int>(gw->winners), 1, static_cast<int>(gw->entries.size()));
		std::string winners{};
		if (not gw->entries.empty()) {
			gw->sub_entries = gw->entries;
			for (int64_t i = 0; i < gw->winners; i++)
			{
				size_t result = rand<size_t>(0, gw->sub_entries.size() - 1);
				winners += std::format("<@{0}>, ", gw->sub_entries[result]);
				gw->sub_entries.erase(std::ranges::find(gw->sub_entries, gw->sub_entries[result]));
			}
			winners.resize(winners.size() - 2);
		}
		gw->message_update(winners);
		_giveaway->erase(id);
		std::filesystem::remove(std::format(".\\giveaways\\{0}", static_cast<uint64_t>(id)));
	};

static void button_pressed(std::unique_ptr<dpp::button_click_t> event) {
	std::unique_ptr<std::vector<std::string>> i = index(event->custom_id, '.');
	if (i->at(0) == "giveaway") {
		std::unique_ptr<giveaway> gw = std::make_unique<giveaway>(_giveaway->at(stoull(i->at(1))));
		if (std::ranges::find(gw->entries, static_cast<uint64_t>(event->command.member.user_id)) not_eq gw->entries.end())
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
	btn_sender.erase(event->command.member.user_id);
}

static void command_sent(std::unique_ptr<dpp::slashcommand_t> event)
{
	if (event->command.get_command_name() == "purge")
	{
		bot->messages_get(event->command.channel_id, 0, event->command.id, 0, std::clamp((int)get<int64_t>(event->get_parameter("amount")), 1, 100),
			[&event](const dpp::confirmation_callback_t& mg_cb)
			{
				if (mg_cb.is_error() or std::get<dpp::message_map>(mg_cb.value).empty()) return;
				std::vector<dpp::snowflake> message_vector;
				for (const auto& [id, m] : std::move(std::get<dpp::message_map>(mg_cb.value))) message_vector.emplace_back(id);
				bot->message_delete_bulk(message_vector, event->command.channel_id,
					[&event, mg_cb](const dpp::confirmation_callback_t& mdb_cb)
					{
						std::unique_ptr<dpp::message> msg = std::make_unique<dpp::message>();
						std::string what = mdb_cb.is_error() ?
							std::format("> {0}", mdb_cb.get_error().message) :
							std::format("> Deleted **{0}** messages", std::get<dpp::message_map>(mg_cb.value).size());
						msg->set_content(std::move(what)).set_flags(dpp::m_ephemeral);
						event->reply(std::move(*msg));
					});
			});
	}
	if (event->command.get_command_name() == "gcreate")
	{
		giveaway gw = {
			get<std::string>(event->get_parameter("description")),
			string_to_time(get<std::string>(event->get_parameter("duration"))), get<int64_t>(event->get_parameter("winners")),
			event->command.member.user_id
		};
		if (system_clock::from_time_t(gw.ends) <= system_clock::now())
			event->reply(std::make_unique<dpp::message>("> invalid duration. e.g. **1h, 30m**")
				->set_flags(dpp::m_ephemeral));
		else
		{
			bot->message_create(std::make_unique<dpp::message>(event->command.channel_id,
				std::make_unique<dpp::embed>()
				->set_title({ get<std::string>(event->get_parameter("title")) }))
				->add_component(std::make_unique<dpp::component>()->add_component(std::make_unique<dpp::component>()
					->set_emoji(u8"🎉").set_id("nullptr"))),
				[&event, &gw](const dpp::confirmation_callback_t& callback)
				{
					if (callback.is_error()) return;
					gw.message = std::move(*std::make_unique<dpp::message>(std::get<dpp::message>(callback.value)));
					gw.description = std::move(get<std::string>(event->get_parameter("description"))); // TODO
					gw.message.components[0].components[0].set_id(std::format(".giveaway.{0}", (uint64_t)gw.message.id));
					gw.message_update();
					_giveaway->emplace(gw.message.id, gw);
					active_code.emplace_back(std::async(std::launch::async, pending_giveaway, gw.message.id));
					event->reply(std::make_unique<dpp::message>(std::format("> The giveaway was successfully created! ID: **{0}**", static_cast<uint64_t>(gw.message.id)))
						->set_flags(dpp::m_ephemeral));
				});
		}
	}
	std::this_thread::sleep_for(1s);
	cmd_sender.erase(event->command.member.user_id);
}

int main()
{
	bot->on_ready([](const dpp::ready_t& event)
		{
			for (const auto& file : std::filesystem::directory_iterator(".\\giveaways\\"))
			{
				nlohmann::json j = nlohmann::json::parse(std::ifstream{ file.path().string() });
				bot->message_get(dpp::snowflake(j["m_id"].get<uint64_t>()), dpp::snowflake(j["m_cid"].get<uint64_t>()), [j](const dpp::confirmation_callback_t& callback) {
					if (callback.is_error()) return;
					giveaway gw = { j["desc"], j["ends"], j["w"], j["h"], j["e"], {}, std::get<dpp::message>(callback.value) };
					_giveaway->emplace(gw.message.id, std::move(gw));
					active_code.emplace_back(std::async(std::launch::async, pending_giveaway, gw.message.id));
					});
			}
			std::vector<dpp::slashcommand> cmds = {
				dpp::slashcommand("purge", "mass delete messages", bot->me.id)
					.set_default_permissions(dpp::p_administrator)
					.add_option(dpp::command_option(dpp::co_integer, "amount", "amount of messages to delete", true).set_max_value(100).set_min_value(1)),

				dpp::slashcommand("gcreate", "create a giveaway", bot->me.id)
					.set_default_permissions(dpp::p_administrator)
					.add_option(dpp::command_option(dpp::co_string, "title", "what you're giveawaying", true))
					.add_option(dpp::command_option(dpp::co_string, "description", "describe the giveaway", true))
					.add_option(dpp::command_option(dpp::co_string, "duration", "the length of the giveaway e.g. 1h, 30m", true))
					.add_option(dpp::command_option(dpp::co_integer, "winners", "amount of winners", true).set_min_value(1))
			};
			bot->global_bulk_command_create(std::move(cmds));
		});
	bot->on_slashcommand([](const dpp::slashcommand_t& event)
		{
			if (cmd_sender.contains(event.command.member.user_id)) return;
			cmd_sender.try_emplace(event.command.member.user_id, std::async(std::launch::async, command_sent, std::make_unique<dpp::slashcommand_t>(std::move(event))));
		});
	bot->on_button_click([](const dpp::button_click_t& event)
		{
			if (btn_sender.contains(event.command.member.user_id)) return;
			btn_sender.try_emplace(event.command.member.user_id, std::async(std::launch::async, button_pressed, std::make_unique<dpp::button_click_t>(std::move(event))));
		});
	bot->on_log(dpp::utility::cout_logger());
	bot->start(dpp::start_type::st_wait);
}