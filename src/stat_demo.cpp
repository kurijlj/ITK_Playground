// ============================================================================
// stat_demo - Demonstrates calculation of common statistical values from a
//            sample of values drawn from a normal distribution.
//
//  Copyright (C) 2024 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
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
// 2024-03-28 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// * stat_demo.cpp: created.
//
// ============================================================================


// ============================================================================
// Headers include section
// ============================================================================

// Related header

// "C" system headers

// Standard Library headers
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>  // Used for testing directory and file status
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>

// External libraries headers
#include <clipp.hpp>  // command line arguments parsing


// ============================================================================
// Define namespace aliases
// ============================================================================

namespace fs = std::filesystem;


// ============================================================================
// Define aliases
// ============================================================================

using u32    = std::uint_least32_t; 
using engine = std::mt19937;


// ============================================================================
// Global constants section
// ============================================================================

const std::string kAppName = "stat_demo";
const std::string kVersionString = "1.0";
const std::string kYearString = "2024";
const std::string kAuthorName = "Ljubomir Kurij";
const std::string kAuthorEmail = "ljubomir_kurij@protonmail.com";
const std::string kAppDoc = "\
Demonstrates calculation of common statistical values from a sample of\n\
values drawn from a normal distribution.\n\n\
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
        float mean_value;
        float stddev_value;
        std::vector<std::string> unsupported;
    };

    // Define the default values for the command line options
    CLIOptions user_options {
        false,  // show_help
        false,  // print_usage
        false,  // show_version
        50.0,   // mean_value
        5.0,    // stddev_value
        {}      // unsupported options aggregator
        };

    // Set command line options
    auto parser_config = (
        (
            clipp::option("-h", "--help").set(user_options.show_help)
                .doc("show this help message and exit"),
            clipp::option("--usage").set(user_options.print_usage)
                .doc("give a short usage message"),
            clipp::option("-V", "--version").set(user_options.show_version)
                .doc("print program version")
        ).doc("general options:"),
        (
            (clipp::option("-m", "--mean")
                & clipp::opt_value("mju", user_options.mean_value)
            ).doc("set the mean value of the normal distribution"),
            (clipp::option("-s", "--standard-deviation")
                & clipp::opt_value("stddev", user_options.stddev_value)
            ).doc("set the standard deviation of the normal distribution")
        ).doc("normal distribution options:"),
        clipp::any_other(user_options.unsupported)
    );

    // Execute the main code inside a try block to catch any exceptions and
    // to ensure that all of the code exits at exactly the same point
    try {
        // Parse command line options -----------------------------------------
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

        // Validate user input ------------------------------------------------

        // Check if the mean value is an integer
        if ((int) std::floor(user_options.mean_value)
                != (int) std::ceil(user_options.mean_value)) {
            std::cerr << kAppName
                << ": Mean value must be an integer."
                << std::endl;
            printShortHelp(exec_name);

            throw EXIT_FAILURE;
        }

        // Check if the standard deviation value is an integer
        if ((int) std::floor(user_options.stddev_value)
                != (int) std::ceil(user_options.stddev_value)) {
            std::cerr << kAppName
                << ": Standard deviation value must be an integer."
                << std::endl;
            printShortHelp(exec_name);

            throw EXIT_FAILURE;
        }

        // Everyting is OK. Proceed with the main code ------------------------
        std::map<int, int> table;
    
        // Initalize the random number generator with the normal distribution
        std::random_device os_seed;
        const u32 seed = os_seed();
        engine generator {seed};
        std::normal_distribution d {
            user_options.mean_value,
            user_options.stddev_value
            };

        // draw a sample from the normal distribution and round it to an integer
        auto random_int = [&d, &generator]{ return std::round(d(generator));};
    
        // Populate the table
        for(int i = 0; i < 10000; ++i)
            ++table[random_int()];
    
        // Collect statistics on the table data
    
        // Calculate the mean value
        int total_occurences = 0;
        float mean_value = 0.0;
        for (const auto& [value, occurence] : table) {
            total_occurences += occurence;
        }
        for (const auto& [value, occurence] : table) {
            float pval = (float) occurence / (float) total_occurences;
            table[value] = round(pval * 1000);
            mean_value += (float) value * pval;
        }
        mean_value = round(mean_value);
    
        // Calculate the variance
        float variance = 0.0;
        for (const auto& [value, occurence] : table) {
            float pval = (float) occurence / 1000;    
            variance += std::pow(value - mean_value, 2) * pval;
        }

        // Calculate the standard deviation
        int sigma = std::round(std::sqrt(variance));
    
        // Calculate the mode value
        int mode_value = 0;
        int max_occurence = 0;
        for (const auto& [value, occurence] : table) {
            if (occurence > max_occurence) {
                mode_value = value;
                max_occurence = occurence;
            }
        }
    
        // Reset total_occurences
        total_occurences = 0;
        for (const auto& [value, occurence] : table) {
            total_occurences += occurence;
        }
    
        // Calculate the median value
        int median_value = 0;
        int median_occurences = total_occurences / 2;
        int current_occurences = 0;
        for (const auto& [value, occurence] : table) {
            current_occurences += occurence;
            if (current_occurences >= median_occurences) {
                median_value = value;
                break;
            }
        }

        std::cout << "Normal distribution statistics:" << std::endl;
        std::cout << "User set mean value: "
            << user_options.mean_value << std::endl;
        std::cout << "User set standard deviation: "
            << user_options.stddev_value << std::endl;
        std::cout << "Calculated mean value: " << mean_value << std::endl;
        std::cout << "Calculated standard deviation: " << sigma << std::endl;
        std::cout << "Calculated mode value: " << mode_value << std::endl;
        std::cout << "Calculated median value: " << median_value << std::endl;
        std::cout << std::endl;

        for (auto [value, occurence] : table) {
            std::string vfield = std::to_string(value);
            if (mean_value == value)
                vfield = "<" + vfield + ">";
            if (mode_value == value)
                vfield = "[" + vfield + "]";
            if (median_value == value)
                vfield = "{" + vfield + "}";
            if (0 != sigma) {
                if (value == mean_value - sigma)
                    vfield = "-1s " + vfield;
                if (value == mean_value + sigma)
                    vfield = " 1s " + vfield;
            }

            // Determine the position of the value's most insignificant digit in
            // the string representation of the value
            int pos = 0;
            for (int i = vfield.size() - 1; i >= 0; --i) {
                if (std::isdigit(vfield[i])) {
                    pos = i;
                    break;
                }
            }

            // Fill the value field with spaces to be 12 characters wide, and
            // align the values
            vfield = vfield + std::string(4 - (vfield.size() - pos), ' ');
            vfield = std::string(12 - vfield.size(), ' ') + vfield;

            // Print the value field and the bar chart
            std::cout << vfield << " "
                << std::string(occurence, '*') << std::endl;

        }
    
        std::cout << std::endl;
        std::cout << "Legend: <> - mean value, [] - mode value, "
            << "{} - median value, -/+1s - 1 standard deviation" << std::endl;

        // Exit the program with a successful exit code -----------------------
        throw EXIT_SUCCESS;

    } catch (int result) {
        // Return the result of the main code ---------------------------------
        return result;

    } catch (...) {
        // We have an unhandled exception. Print error message and exit with
        // an error code ------------------------------------------------------
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
    // message and exit with an error code ------------------------------------
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
