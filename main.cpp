#include <string>
#include <memory>
#include "model/host.h"
#include "model/client.h"
#include "io/frame.h"
#include "model/log.h"
#include "components/computer.h"

using std::cerr;

void usage()
{
    cerr << "ocvm [ENV_PATH] [--frame=FRAME_TYPE] [--depth=DEPTH] [--log-allocs[=STACK_LOG]]\n"
            "  ENV_PATH     VM env path. Optional. defaults to ./tmp\n"
            "  FRAME_TYPE   Term emulator(ansi or basic). defaults to ansi\n"
            "  DEPTH        Color depth(1, 4, or 8). defaults to 8\n"
            "  STACK_LOG    Log allocations and stack traces to STACK_LOG.\n"
            "                 If enabled, defaults to stack.log\n";
    ::exit(1);
}

struct Args
{
    vector<string> indexed;
    map<string, string> named;

    enum
    {
        LogAllocKey,
        FrameKey,
        DepthKey
    };

    const string keys[3] =
    {
        "log-allocs",
        "frame",
        "depth"
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

string get_env_path(const string& value)
{
    return value.empty() ? "tmp" : value;
}

string get_framer_type(const string& value)
{
    return value.empty() ? "ansi" : value;
}

EDepthType get_depth(const string& value)
{
    if (value == "1")
        return EDepthType::_1;
    if (value == "4")
        return EDepthType::_4;
    else if (value == "" || value == "8")
        return EDepthType::_8;

    cerr << "invalid depth: " << value << endl;
    usage();
    return EDepthType::_1; // won't reach here, usage exits
}

int main(int argc, char** argv)
{
    auto args = load_args(argc, argv);
    string client_env_path = get_env_path(args.get(1));
    string framer_type = get_framer_type(args.get("frame"));
    EDepthType depth = get_depth(args.get("depth"));
    string stack_log = args.get("log-allocs");

    std::unique_ptr<Framer> framer(Factory::create_framer(framer_type));

    // create profile shell (houses screen component [list?])
    if (!framer)
    {
        cerr << "no [" << framer_type << "] framer available\n";
        return 1;
    }

    framer->setInitialDepth(depth);

    // init host config
    // // prepares component factories such as screen, keyboard, and filesystem
    Host host(framer.get());
    host.stackLog(stack_log);
    RunState run;

    do
    {
        if (!framer->open())  // open the ui
        {
            cerr << "framer open failed\n";
            break;
        }

        framer->add(Logger::getFrame());

        // init client config
        Client client(&host, client_env_path);
        // init lua environment
        // // creates instances of host components
        if (!client.load())
            break;

        do
        {
            if (!framer->update())
            {
                break;
            }
            run = client.run();
        }
        while (run == RunState::Continue);

        framer->close();
    }
    while (run == RunState::Reboot);

    return 0;
}
