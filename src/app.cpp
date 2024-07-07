// ============================================================================
// <one line to give the program's name and a brief idea of what it does.>
//
//  Copyright (C) <yyyy> <Author Name> <author@mail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// ============================================================================


// ============================================================================
//
// <Put documentation here>
//
// <yyyy>-<mm>-<dd> <Author Name> <author@mail.com>
//
// * <programfilename>.cpp: created.
//
// ============================================================================


// ============================================================================
//
// References (this section should be deleted in the release version)
//
// * For coding style visit Google C++ Style Guide page at
//   <https://google.github.io/styleguide/cppguide.html>.
//
// * For command line arguments parsing using clipp consult documentation and
//   examples at <https://github.com/muellan/clipp>.
//
// * For filesystem operations (C++17) visit 'filesystem' reference at:
//   <https://en.cppreference.com/w/cpp/filesystem>.
//
// * For SQLite3 C/C++ interface documentation visit
//   <https://www.sqlite.org/c3ref/intro.html>.
//
// * For libsodium library documentation visit
//   <https://libsodium.gitbook.io/doc/>.
//
// * For CSV parser library documentation visit
//   <https://github.com/vincentlaucsb/csv-parser>
//
// ============================================================================


// ============================================================================
// Preprocessor directives section
// ============================================================================
#define SODIUM_STATIC 1  // Required for 'sodium.h' to work properly if using
                         // static linking of the library


// ============================================================================
// Headers include section
// ============================================================================

// Related header

// "C" system headers

// Standard Library headers
#include <cmath>       // required by std::pow, std::sqrt, ...
#include <cstdlib>     // required by EXIT_SUCCESS, EXIT_FAILURE
#include <exception>   // required by std::current_exception
#include <filesystem>  // Used for testing directory and file status
#include <fstream>     // Required for file I/O operations
#include <iomanip>     // required by std::setw
#include <iostream>    // required by cin, cout, ...
#include <map>         // required by std::map
#include <string>      // required by std::string

// External libraries headers
#include <clipp.hpp>  // command line arguments parsing
#include <sqlite3.h>  // SQLite3 database library
#include <sodium.h>   // Libsodium library
#include <csv.hpp>    // CSV parser library


// ============================================================================
// Define namespace aliases
// ============================================================================

namespace fs = std::filesystem;


// ============================================================================
// Global constants section
// ============================================================================

const std::string kAppName = "cli_app";
const std::string kVersionString = "0.1";
const std::string kYearString = "yyyy";
const std::string kAuthorName = "Ljubomir Kurij";
const std::string kAuthorEmail = "ljubomir_kurij@protonmail.com";
const std::string kAppDoc = "\
Framework for developing command line applications using \'clipp\' command\n\
line argument parsing library.\n\n\
Mandatory arguments to long options are mandatory for short options too.\n";
const std::string kLicense = "\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";


// ============================================================================
// Global variables section
// ============================================================================

static std::string exec_name = kAppName;


// ============================================================================
// Utility function prototypes
// ============================================================================

void printShortHelp(std::string = kAppName);
void printUsage(
        const clipp::group&,
        const std::string = kAppName,
        const clipp::doc_formatting& = clipp::doc_formatting{}
    );
void printVersionInfo();
void showHelp(
        const clipp::group&,
        const std::string = kAppName,
        const std::string = kAppDoc
    );


// ============================================================================
// Main Function Section
// ============================================================================

int main(int argc, char *argv[])
{
    // Determine the exec name under wich program is beeing executed
    fs::path exec_path {argv[0]};
    exec_name = exec_path.filename().string();

    // Here we define the structure for holding the passed command line otions.
    // The structure is also used to define the command line options and their
    // default values.
    struct CLIOptions {
        bool show_help;
        bool print_usage;
        bool show_version;
        std::string input_file;
        std::vector<std::string> unsupported;
    };

    // Define the default values for the command line options
    CLIOptions user_options {
        false,  // show_help
        false,  // print_usage
        false,  // show_version
        "",     // input_file
        {}      // unsupported options aggregator
        };

    // Option filters definitions
    auto istarget = clipp::match::prefix_not("-");  // Filter out strings that
                                                    // start with '-' (options)

    // Set command line options
    auto parser_config = (
        // Define the command line options and their default values.
        // - Must have more than one option.
        // - The order of the options is important.
        // - The order of the options in the group is important.
        // - Take care not to omitt value filter when parsing file and directory
        //   names. Otherwise, the parser will treat options as values.
        // - Define positional arguments first
        // - Define positional srguments as optional to enforce the priority of
        //   help, usage and version switches. Then enforce the required
        //   positional arguments by checking if their values are set.
        (
            clipp::opt_value(istarget, "INPUT_FILE", user_options.input_file),
            clipp::option("-h", "--help").set(user_options.show_help)
                .doc("show this help message and exit"),
            clipp::option("--usage").set(user_options.print_usage)
                .doc("give a short usage message"),
            clipp::option("-V", "--version").set(user_options.show_version)
                .doc("print program version")
        ).doc("general options:"),
        clipp::any_other(user_options.unsupported)
    );

    // Execute the main code inside a try block to catch any exceptions and
    // to ensure that all of the code exits at exactly the same point
    try {
        // Parse command line options
        auto result = clipp::parse(argc, argv, parser_config);

        // Check if the unsupported options were passed
        if (!user_options.unsupported.empty()) {
            std::cerr << kAppName << ": Unsupported options: ";
            for (const auto& opt : user_options.unsupported) {
                std::cerr << opt << " ";
            }
            std::cerr << std::endl;
            printShortHelp(exec_name);

            throw EXIT_FAILURE;
        }

        // Check if the help switch was triggered. We give help switch the
        // highest priority, so if it is triggered we don't need to check
        // anything else.
        if (user_options.show_help) {
            showHelp(parser_config, exec_name);

            throw EXIT_SUCCESS;
        }

        // Check if the usage switch was triggered. Usge switch has the second
        // highest priority, so if it is triggered we don't need to check
        // anything else.
        if (user_options.print_usage) {
            auto fmt = clipp::doc_formatting {}
                .first_column(0)
                .last_column(79);

            printUsage(parser_config, exec_name, fmt);

            throw EXIT_SUCCESS;
        }

        // Check if the version switch was triggered. Version switch has the
        // third highest priority.
        if (user_options.show_version) {
            printVersionInfo();

            throw EXIT_SUCCESS;
        }

        // No high priority switch was triggered. Now we check if the input
        // file was passed. If not we print the usage message and exit.
        if (user_options.input_file.empty()) {
            auto fmt = clipp::doc_formatting {}
                .first_column(0)
                .last_column(79)
                .merge_alternative_flags_with_common_prefix(true);
            std::cout << "Usage: ";
            printUsage(parser_config, exec_name, fmt);

            std::cout << std::endl;

            // Print short help message
            printShortHelp(exec_name);

            throw EXIT_FAILURE;
        }

        // Input file was passed. Now we check if the file exists, is
        // readable and is a regular file and not an empty file.
        // Check if the file exists
        if(!fs::exists(user_options.input_file)) {
            std::cerr << kAppName
                << ": File does not exist: "
                << user_options.input_file
                << std::endl;
            throw EXIT_FAILURE;
        }

        // Check if the file is a regular file
        if(!fs::is_regular_file(user_options.input_file)) {
            std::cerr << kAppName
                << ": Not a regular file: "
                << user_options.input_file
                << std::endl;
            throw EXIT_FAILURE;
        }

        // Check if the file is empty
        if(fs::file_size(user_options.input_file) == 0) {
            std::cerr << kAppName
                << ": Empty file: "
                << user_options.input_file
                << std::endl;
            throw EXIT_FAILURE;
        }

        // Open the file in binary mode for wider compatibility
        std::ifstream file(
            user_options.input_file,
            std::ios::binary
            );

        // Check if the file was opened successfully
        // (if we can read it)
        if(!file.is_open()) {
            std::cerr << kAppName
                << ": Error opening file: "
                << user_options.input_file
                << std::endl;
            throw EXIT_FAILURE;
        }

        // Everything went well. Print success message and close the file
        std::cout << kAppName
            << ": File `"
            << user_options.input_file
            << "` opened successfully!"
            << std::endl;
        file.close();

        // Check if the file is a CSV file

        // Create a CSV parser object. Set the variable column policy to throw
        // an exception if the number of columns is not consistent (not an CSV
        // file)
        csv::CSVFormat format;
        format.variable_columns(csv::VariableColumnPolicy::THROW);
        csv::CSVReader reader(user_options.input_file, format);
        try {
            for (auto it = reader.begin(); it != reader.end(); ++it);
        } catch (std::runtime_error& e) {
            std::cerr << kAppName
                << ": File `"
                << user_options.input_file
                << "` is not a CSV file: Variable number of columns!"
                << std::endl;
            throw EXIT_FAILURE;
        }

        // Try to initialize the libsodium library
        if (sodium_init() < 0) {
            std::cerr << kAppName
                << ": Error initializing libsodium library!"
                << std::endl;

            throw EXIT_FAILURE;
        }

        // Sodium library initialized successfully. Print success message
        std::cout << kAppName
            << ": Libsodium library initialized successfully!"
            << std::endl;

        // Create a SQLite3 database connection
        sqlite3* DB; 
    
        // Try to open the database. If it does not exist, it will be created
        int ret_code = sqlite3_open("example.db", &DB); 
  
        // Check if the database was opened successfully
        if (0 != ret_code) { 
            // Something went wrong. Print error message and exit
            std::cerr << kAppName
                << ": Error creating DB: "
                << sqlite3_errmsg(DB)
                << std::endl; 

            throw EXIT_FAILURE;
        } 
    
        // Everything went well. Print success message
        std::cout << kAppName
            << ": Database opened successfully!"
            << std::endl; 

        // Close the database
        sqlite3_close(DB); 

        // Return success
        throw EXIT_SUCCESS;

    } catch (int result) {
        // Return the result of the main code
        return result;

    } catch (...) {
        // We have an unhandled exception. Print error message and exit
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception& e) {
            std::cerr << kAppName << ": Unhandled exception: " << e.what()
                << std::endl;
        }

        // Return an error code
        return EXIT_FAILURE;
    }

    // The code should never reach this point. If it does, print an error
    // message and exit
    std::cerr << kAppName << ": Unhandled program exit!" << std::endl;

    return EXIT_FAILURE;
}


// ============================================================================
// Function definitions
// ============================================================================

inline void printShortHelp(std::string exec_name) {
    std::cout << "Try '" << exec_name << " --help' for more information.\n";
}

inline void printUsage(
        const clipp::group& group,
        const std::string prefix,
        const clipp::doc_formatting& fmt)
{
    std::cout << clipp::usage_lines(group, prefix, fmt) << "\n";
}


void printVersionInfo() {
    std::cout << kAppName << " " << kVersionString << " Copyright (C) "
        << kYearString << " " << kAuthorName << "\n"
        << kLicense;
}


void showHelp(
        const clipp::group& group,
        const std::string exec_name,
        const std::string doc
        ) {
    auto fmt = clipp::doc_formatting {}.first_column(0).last_column(79);
    clipp::man_page man;

    man.prepend_section(
        "USAGE",
        clipp::usage_lines(group, exec_name, fmt).str()
        );
    man.append_section("", doc);
    man.append_section("", clipp::documentation(group, fmt).str());
    man.append_section("", "Report bugs to <" + kAuthorEmail + ">.");

    std::cout << man;
}
