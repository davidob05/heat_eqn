#include "cli.hpp"
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace heat{
    namespace {
        // each parser throws std::invalid_argument naming the flag if the value is malformed
        double parse_double(const std::string& flag, const std::string& value){
            std::size_t used = 0;
            double result = 0.0;
            try{
                result = std::stod(value, &used);
            } catch(const std::exception&){
                throw std::invalid_argument(flag + ": '" + value + "' is not a number");
            }
            if(used != value.size()) throw std::invalid_argument(flag + ": '" + value + "' is not a number");
            return result;
        }

        long long parse_integer(const std::string& flag, const std::string& value){
            std::size_t used = 0;
            long long result = 0;
            try{
                result = std::stoll(value, &used);
            } catch(const std::exception&){
                throw std::invalid_argument(flag + ": '" + value + "' is not an integer");
            }
            if(used != value.size()) throw std::invalid_argument(flag + ": '" + value + "' is not an integer");
            return result;
        }

        std::size_t parse_size(const std::string& flag, const std::string& value){
            const long long result = parse_integer(flag, value);
            if(result < 0) throw std::invalid_argument(flag + ": must not be negative");
            return static_cast<std::size_t>(result);
        }

    }

    void print_usage(std::ostream& stream, const std::string& program){
        stream << "Usage: " << program << " [options]\n"
               << "  --n-intervals <int>   number of intervals along the rod\n"
               << "  --length <double>     rod length\n"
               << "  --alpha <double>      diffusivity\n"
               << "  --T <double>          target time\n"
               << "  --r-req <double>      requested r\n"
               << "  --mode <int>          sine mode of the initial state\n"
               << "  --snapshots <int>     number of snapshots after the initial state\n"
               << "  --output <path>       output CSV path\n"
               << "  --plot                plot the results after the run (off by default)\n"
               << "  --help                show this message\n"
               << "Values may be given as '--flag value' or '--flag=value'.\n";
    }

    CliOptions parse_args(const std::vector<std::string>& args){
        CliOptions options;

        for(std::size_t i = 0 ; i < args.size() ; i++ ){
            std::string flag = args[i];
            std::string value;
            bool has_value = false;

            // split '--flag=value'
            const std::size_t eq = flag.find('=');
            if(flag.rfind("--", 0) == 0 && eq != std::string::npos){
                value = flag.substr(eq + 1);
                flag = flag.substr(0, eq);
                has_value = true;
            }

            if(flag == "--help" || flag == "-h"){
                options.show_help = true;
                continue;
            }

            if(flag == "--plot"){
                if(has_value) throw std::invalid_argument(flag + ": takes no value");
                options.plot = true;
                continue;
            }

            // everything below needs a value
            if(!has_value){
                if(i + 1 >= args.size()) throw std::invalid_argument(flag + ": missing value");
                value = args[++i];
            }

            if(flag == "--n-intervals")      options.params.n_intervals = parse_size(flag, value);
            else if(flag == "--length")      options.params.length = parse_double(flag, value);
            else if(flag == "--alpha")       options.params.alpha = parse_double(flag, value);
            else if(flag == "--T")           options.params.T = parse_double(flag, value);
            else if(flag == "--r-req")       options.params.r_req = parse_double(flag, value);
            else if(flag == "--mode")        options.params.mode = static_cast<int>(parse_integer(flag, value));
            else if(flag == "--snapshots")   options.params.snapshots = parse_size(flag, value);
            else if(flag == "--output")      options.output_path = value;
            else throw std::invalid_argument("unknown option '" + flag + "'");
        }
        return options;
    }
}
