/*
    MIT License

    Copyright (c) 2017, namreeb (legal@namreeb.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#define NAME    "wowned"
#define VERSION "v0.4"

#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <hadesmem/injector.hpp>

int main(int argc, char *argv[])
{
    std::cout << NAME << " " << VERSION << " injector" << std::endl;

    std::wstring program;
    bool enableConsole;
    int method;

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "display help message")
        ("console,c", "enable wow console")
        ("program,p", boost::program_options::wvalue<std::wstring>(&program)->default_value(L"wow.exe", "wow.exe"), "path to wow binary")
        ("1", "exploit method one")
        ("2", "exploit method two");

    try
    {
        boost::program_options::variables_map vm;

        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        enableConsole = !!vm.count("console");

        if (!vm.count("1") && !vm.count("2"))
        {
            std::cerr << "ERROR: No method chosen" << std::endl;
            std::cerr << desc << std::endl;

            return EXIT_FAILURE;
        }

        if (vm.count("1") && vm.count("2"))
        {
            std::cerr << "ERROR: Cannot use both methods simultaneously" << std::endl;
            std::cerr << desc << std::endl;

            return EXIT_FAILURE;
        }

        method = !!vm.count("1") ? 1 : 2;
    }
    catch (boost::program_options::error const &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;

        return EXIT_FAILURE;
    }

    try
    {
        std::vector<std::wstring> createArgs;

        if (enableConsole)
            createArgs.push_back(L"-console");

        const hadesmem::CreateAndInjectData injectData =
            hadesmem::CreateAndInject(program, L"", std::begin(createArgs), std::end(createArgs),
                L"auth_bypass.dll", method == 1 ? "Load1" : "Load2",
                hadesmem::InjectFlags::kPathResolution|hadesmem::InjectFlags::kAddToSearchOrder);

        std::cout << "Injected.  Process ID: " << injectData.GetProcess().GetId() << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << std::endl << "ERROR: " << std::endl;
        std::cerr << boost::diagnostic_information(e) << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
