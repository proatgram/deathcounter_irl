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

#pragma once

#include <dpp/dpp.h>
#include <pqxx/pqxx>

namespace death {
    class bot {
        public:
            bot(const bot&) = delete;
            bot(bot&&) = delete;
            auto operator=(const bot&) -> bot& = delete;
            auto operator=(bot&&) -> bot& = delete;
            ~bot();
        
            explicit bot(const std::string &bot_token, const std::string &db_hostname, const std::string &db_name, const std::string &db_user, const std::string &db_passwd, const std::string &db_port);

            void start();

        private:
            std::string_view m_bot_token;
            dpp::cluster m_cluster;
            std::string m_connection_uri;
            pqxx::connection m_db;

            std::atomic_bool m_guild_cache_ready;
            
            void register_commands();

            void command_deaths(const dpp::slashcommand_t &event);

            void check_database();

            void add_guild_to_database(const dpp::guild &guild);
    };
} // namespace death
