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

#include <iostream>
#include <string>

#include <argparse/argparse.hpp>

#include "config.hpp"
#include "bot.hpp"

auto main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int {
    argparse::ArgumentParser program = argparse::ArgumentParser(std::string(project_name), std::string(project_ver));

    program.add_description(std::string(project_description));
    program.add_epilog(std::string(project_license_notice_interactive)
        .replace(project_license_notice_interactive.find("<program>"), 9, program_invocation_name)
    );

    program.add_argument("--license")
        .action([=]([[maybe_unused]] const std::string &arg){
                std::cout << project_license << std::endl;
                exit(0);
        })
        .help("prints the license used for this project")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--api-key", "-k")
        .help("set the Discord bot API key")
        .required();

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    death::bot death_bot(program.get("--api-key"));

    death_bot.start();

    return 0;
}
