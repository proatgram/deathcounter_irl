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
    bot::bot(const std::string_view &bot_token) :
        m_bot_token(bot_token),
        m_cluster(std::string(bot_token))
    {

    }

    void bot::start() {
        m_cluster.on_ready([this]([[maybe_unused]] const dpp::ready_t &event) {
            if (dpp::run_once<struct register_bot_commands>()) {
                m_cluster.global_command_create(dpp::slashcommand("deaths", "List the deaths of users.", m_cluster.me.id));
                m_cluster.global_command_create(dpp::slashcommand("deathwish", "Increments the death count on a user.", m_cluster.me.id));
            }
        });

        m_cluster.on_slashcommand([](const dpp::slashcommand_t &event) {
            if (event.command.get_command_name() == "deaths") {
                std::string deaths = "# Deaths\n-------\n";

            }
        });

         m_cluster.start(dpp::st_wait);
    }
} // namespace death
