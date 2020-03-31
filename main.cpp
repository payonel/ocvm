#include <string>
#include <memory>
#include <thread>
#include "model/host.h"
#include "model/client.h"
#include "io/frame.h"
#include "model/log.h"
#include "components/computer.h"
#include "drivers/fs_utils.h"

using std::cerr;

void usage()
{
    cerr << "ocvm [ENV_PATH] [OPTIONS]\n"
            "   ENV_PATH            (optional) VM env path. Default ./tmp\n"
            "OPTIONS\n"
            "  --frame=TYPE         Term emulator type. Can be 'ansi' (default) or 'basic'.\n"
            "  --log-allocs[=PATH]  Enable logging mallocs and stacks.\n"
            "                       Optional custom path, default stack.log\n"
            "  --bios=PATH          Path to custom eeprom bios code\n"
            "  --machine=PATH       Path to custom machine.lua\n"
            "  --fonts=PATH         Path to custom fonts.hex\n";
    ::exit(1);
}

struct Args
{
    vector<string> indexed;
    map<string, string> named;

    enum
    {
        LogAllocKey = 0,
        FrameKey,
        BiosKey,
        MachineKey,
        FontsKey
    };

    const string keys[FontsKey+1] =
    {
        "log-allocs",
        "frame",
        "bios",
        "machine",
        "fonts"
    };

    string get(int n) const
    {
        n--;
        if (n < 0 || n >= static_cast<int>(indexed.size()))
            return "";
        return indexed.at(n);
    }

    string get(string name) const
    {
        const auto& it = named.find(name);
        if (it == named.end())
            return "";
        return it->second;
    }

    bool valid_key(const string& key)
    {
        if (key == "help") // no error message, but report usage
            return false;

        for (size_t i = 0; i < sizeof(keys); i++)
        {
            if (key == keys[i])
                return true;
        }

        cerr << "bad argument [" << key << "]\n";
        return false;
    }

    string defaults(const string& key)
    {
        if (key == keys[LogAllocKey])
            return "stack.log";
        return "";
    }

    string client_env_path() const
    {
        string value = get(1);
        return value.empty() ? "tmp" : value;
    }

    string frame_type() const
    {
        string value = get(keys[Args::FrameKey]);
        return value.empty() ? "ansi" : value;
    }

    string stack_log() const
    {
        return get(keys[Args::LogAllocKey]);
    }

    string bios_path() const
    {
        string value = get(keys[Args::BiosKey]);
        return value.empty() ? fs_utils::make_proc_path("system/bios.lua") : value;
    }

    string machine_path() const
    {
        string value = get(keys[Args::MachineKey]);
        return value.empty() ? fs_utils::make_proc_path("system/machine.lua") : value;
    }

    string fonts_path() const
    {
        string value = get(keys[Args::FontsKey]);
        return value.empty() ? fs_utils::make_proc_path("system/font.hex") : value;
    }
};

bool valid_arg_index(size_t size)
{
    static size_t expected = 1;
    if (size > expected)
    {
        cerr << "too many arguments, expected " << expected << endl;
    }
    return size <= 1;
}

Args load_args(int argc, char** argv)
{
    Args args;
    if (argc >= 1)
        fs_utils::set_prog_name(argv[0]);

    // start from 1, argv[0] is the prog name
    for (int i = 1; i < argc; i++)
    {
        string t = argv[i];
        if (t.find("--") == 0)
        {
            string key = t.substr(2);
            string value;
            size_t equal_index = key.find("=");
            if (equal_index != string::npos)
            {
                value = key.substr(equal_index + 1);
                key = key.substr(0, equal_index);

                if (!args.valid_key(key))
                {
                    usage();
                }

                args.named[key] = value;
            }
            else
            {
                string def = args.defaults(key);
                if (def.empty())
                    usage();
                args.named[key] = def;
            }
        }
        else if (t.find("-") == 0)
            usage();
        else
        {
            args.indexed.push_back(t);
            if (!valid_arg_index(args.indexed.size()))
            {
                usage();
            }
        }
    }
    return args;
}

void prepareMachineDirectory(string path)
{
    string client_env_path = fs_utils::make_pwd_path(path);
    // make the env path if it doesn't already exist
    if (!fs_utils::mkdir(client_env_path))
    {
        std::cerr << "could not create virtual machine path: " << client_env_path << std::endl;
        ::exit(1);
    }

    Logger::context({ client_env_path });
}

string runVirtualMachine(const Args& args)
{
    string clientShutdownMessage;

    prepareMachineDirectory(args.client_env_path());

    // init host config
    // // prepares component factories such as screen, keyboard, and filesystem
    Host host(args.frame_type());
    host.stackLog(args.stack_log());
    host.biosPath(args.bios_path());
    host.machinePath(args.machine_path());
    host.fontsPath(args.fonts_path());
    
    RunState run;

    do
    {
        // init client config
        Client client(&host, Logger::context().path);
        // init lua environment
        // // creates instances of host components
        if (!client.load())
            break;

        do
        {
            run = client.run();
	    std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        while (run == RunState::Continue);

        clientShutdownMessage = client.getAllCrashText();
    }
    while (run == RunState::Reboot);

    return clientShutdownMessage;
}

int main(int argc, char** argv)
{
    auto args = load_args(argc, argv);

    string result = runVirtualMachine(args);
    std::cout << result;

    return !result.empty();
}
