/*
    A Discord bot to mainly detect deaths and count them.
    Copyright (C) 2023  Timothy Hutchins

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "bot.hpp"

#include "config.hpp"

namespace death {
    template <typename T1, typename T2>
    struct more_second {
        using type = std::pair<T1, T2>;
        auto operator ()(type const& first, type const& second) const -> bool {
            return first.second > second.second;
        }
    };
    bot::bot(const std::string &bot_token, const std::string &db_hostname, const std::string &db_name, const std::string &db_user, const std::string &db_passwd, const std::string &db_port) :
        m_bot_token(bot_token),
        m_cluster(bot_token, dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members),
        m_connection_uri("postgresql://" + db_user + ':' + db_passwd + '@' + db_hostname + ':' + db_port + '/' + db_name),
        m_db(m_connection_uri)
    {

    }

    bot::~bot() {
        m_db.close();
    }

    void bot::start() {
        m_cluster.on_log(dpp::utility::cout_logger());

        m_cluster.on_ready([this]([[maybe_unused]] const dpp::ready_t &event) {
            register_commands();
        });

        m_cluster.on_slashcommand([this](const dpp::slashcommand_t &event) {
            if (event.command.get_command_name() == "deaths") {
                command_deaths(event);
            }
            if (event.command.get_command_name() == "deathwish") {
                event.reply(event.command.get_issuing_user().get_mention() + " Manual death request? To confirm react with :skull:");
            }
        });

        m_cluster.on_message_reaction_add([this](const dpp::message_reaction_add_t &event) {
            dpp::message message(m_cluster.message_get_sync(event.message_id, event.channel_id));
            if (message.interaction.name.find("deathwish") != std::string::npos) {
                if (message.interaction.usr.id == event.reacting_user.id && event.reacting_emoji.name.find("💀") != std::string::npos) {
                    dpp::message confirmation_reply(message.channel_id, "Confirmed! increasing deathcount. :skull: +1 :c", dpp::message_type::mt_reply);
                    confirmation_reply.set_guild_id(message.guild_id)
                        .set_reference(message.id, message.guild_id, message.channel_id);
                    m_cluster.message_create_sync(confirmation_reply);
                    pqxx::work work(m_db);
                    try {
                        pqxx::result result(work.exec(
                        "UPDATE " +
                        work.quote_name(std::to_string(event.reacting_guild->id)) +
                        " SET user_deathcount = user_deathcount + 1 " +
                        "WHERE user_id = " +
                        work.quote(std::to_string(event.reacting_user.id)) + ";"
                        ));
                        work.commit();
                    }
                    catch (const std::exception &err) {
                        std::cerr << err.what() << std::endl;
                    }
                }
            }
        });

        m_cluster.on_guild_create([this](const dpp::guild_create_t &event) {
            add_guild_to_database(*event.created);
        });

        m_cluster.on_guild_members_chunk([this](const dpp::guild_members_chunk_t &event) {
            add_guild_members_to_table(*event.members, event.adding->id);
        });

        m_cluster.on_message_create([this](const dpp::message_create_t &event) {
            if (event.msg.author.id != m_cluster.me.id) {
                if (event.msg.content.find(":muffinghost:") != std::string::npos) {
                    event.reply(":muffinghost: detected! :skull: +1 :c");
                    pqxx::work work(m_db);
                    try {
                        pqxx::result result(work.exec(
                        "UPDATE " +
                        work.quote_name(std::to_string(event.msg.guild_id)) +
                        " SET user_deathcount = user_deathcount + 1 " +
                        "WHERE user_id = " +
                        work.quote(std::to_string(event.msg.author.id)) + ";"
                        ));
                        work.commit();
                    }
                    catch (const std::exception &err) {
                        std::cerr << err.what() << std::endl;
                    }
                }
            }
        });
        
         m_cluster.start(dpp::st_wait); // NOLINT
    }

    void bot::register_commands() {
        if (dpp::run_once<struct register_bot_commands>()) {
            m_cluster.global_command_create(dpp::slashcommand("deaths", "List the deaths of users.", m_cluster.me.id));
            m_cluster.global_command_create(dpp::slashcommand("deathwish", "Increments the death count on a user.", m_cluster.me.id));
        }
    }

    void bot::command_deaths(const dpp::slashcommand_t &event) {
        std::vector<std::pair<std::string, int>> users;
        pqxx::work work(m_db);
        try {
            pqxx::result result(work.exec("SELECT * FROM " + work.quote_name(std::to_string(event.command.guild_id))));
            work.commit();
            for (const auto &[user_name, user_displayname, user_nickname, user_id, user_deathcount] : result.iter<std::string, std::string, std::string, std::string, int>()) {
                users.emplace_back((!user_nickname.empty() ? user_nickname : (!user_displayname.empty() ? user_displayname : user_name)), user_deathcount);
            }
        }
        catch (const std::exception &err) {
            std::cerr << err.what() << std::endl;
        }
        std::sort(users.begin(), users.end(), more_second<std::string, int>());
        std::stringstream deaths;

        deaths << "# DEATHS :skull:\n```\n";

        deaths << "| " << std::setw(users.at(0).first.size() + 12) << std::left << "User" << " | " << "Deaths" << " |" << std::endl;
        for (const auto &[user, count] : users) {
            deaths << "| " << std::setw(users.at(0).first.size() + 12) << std::left << user << " | " << std::setw(6) << count << " |" << std::endl;
        }
        deaths << "```" << std::endl;;
            
        event.reply(deaths.str());
    }

    void bot::check_database() { // NOLINT
        dpp::cache<dpp::guild> *cache = dpp::get_guild_cache();
        // IMPORTANT: Lock Mutext to iterate over it, can't have race conditions can we :D
        std::shared_lock lock(cache->get_mutex());

        std::unordered_map<dpp::snowflake, dpp::guild*> &guilds = cache->get_container();

        for (const auto &[snowflake, guild] : guilds) {
            std::cout << guild->name << std::endl;
        }
    }

    void bot::add_guild_to_database(const dpp::guild &guild) {
        pqxx::work work(m_db);
        std::stringstream command;
        command << "CREATE TABLE IF NOT EXISTS "
        << work.quote_name(std::to_string(guild.id))
        << " ("
        << "user_name character varying NOT NULL,"
        << "user_displayname character varying NOT NULL,"
        << "user_nickname character varying NOT NULL,"
        << "user_id character varying NOT NULL,"
        << "user_deathcount integer NOT NULL,"
        << "unique(user_id, user_name)"
        << ");";

        try {
            pqxx::result result(work.exec0(command.str()));
            work.commit();
        }
        catch (const std::exception &err) {
            std::cerr << err.what() << std::endl;
        }
    }

    void bot::add_guild_members_to_table(const dpp::members_container &members, const dpp::snowflake &guild_id) {
        for (const auto &[snowflake, member] : members) {
            pqxx::work work(m_db);
            std::stringstream command;

            command << "INSERT INTO " << work.quote_name(std::to_string(guild_id))
                << "AS tb"
                << " ("
                << "user_name, "
                << "user_displayname, "
                << "user_nickname, "
                << "user_id, "
                << "user_deathcount"
                << ") "
                << "VALUES"
                << " ("
                << work.quote(member.get_user()->username) << ", "
                << work.quote(member.get_user()->global_name) << ", "
                << work.quote(member.nickname) << ", "
                << work.quote(std::to_string(member.user_id)) << ", "
                << "0"
                << ") "
                << "ON CONFLICT (user_id, user_name) DO UPDATE SET "
                << "(user_nickname, user_id, user_deathcount) = "
                << "(tb.user_nickname, tb.user_id, tb.user_deathcount);";
            try {
                pqxx::result result(work.exec(command.str()));
                work.commit();
            }
            catch (const std::exception &err) {
                std::cerr << err.what() << std::endl;
            }
        }
    }

} // namespace death
