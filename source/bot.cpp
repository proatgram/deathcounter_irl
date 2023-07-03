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

    bot::bot(const std::string &bot_token, const std::string &db_hostname, const std::string &db_name, const std::string &db_user, const std::string &db_passwd, const std::string &db_port) :
        m_bot_token(bot_token),
        m_cluster(bot_token),
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
        });

        m_cluster.on_guild_create([this](const dpp::guild_create_t &event) {
            add_guild_to_database(*event.created);
        });
        
         m_cluster.start(dpp::st_wait); // NOLINT
    }

    void bot::register_commands() {
        if (dpp::run_once<struct register_bot_commands>()) {
            m_cluster.global_command_create(dpp::slashcommand("deaths", "List the deaths of users.", m_cluster.me.id));
            m_cluster.global_command_create(dpp::slashcommand("deathwish", "Increments the death count on a user.", m_cluster.me.id));
        }
    }

    void bot::command_deaths(const dpp::slashcommand_t &event) { // NOLINT
        std::string deaths = "# Deaths\n-------\n";
        event.reply(deaths);
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
        << "user_nickname character varying NOT NULL,"
        << "user_id character varying NOT NULL,"
        << "user_deathcount integer NOT NULL"
        << ");";

        try {
            pqxx::result result(work.exec0(command.str()));
            work.commit();
        }
        catch (const std::exception &err) {
            std::cerr << err.what() << std::endl;
        }
    }

} // namespace death
