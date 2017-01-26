/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef ARGSPLUS_HXX
#define ARGSPLUS_HXX

#include <initializer_list>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace argsplus {
template <typename String = std::string, typename Char = char,
    template <typename...> class List = std::vector,
    template <typename...> class Set = std::unordered_set>
class ArgumentParser {
    private:
    /** A simple unified option type for unified initializer lists for the
     * Matcher class.
    */
    struct EitherFlag {
        const bool isShort;
        const Char shortFlag;
        const String longFlag;
        EitherFlag(const String &flag)
            : isShort(false), shortFlag(), longFlag(flag) {}
        EitherFlag(const Char *flag)
            : isShort(false), shortFlag(), longFlag(flag) {}
        EitherFlag(const Char flag)
            : isShort(true), shortFlag(flag), longFlag() {}

        /** Get just the long flags from an initializer list of EitherFlags
        */
        static Set<String> GetLong(std::initializer_list<EitherFlag> flags) {
            Set<String> longFlags;
            for (const EitherFlag &flag : flags) {
                if (!flag.isShort) {
                    longFlags.insert(flag.longFlag);
                }
            }
            return longFlags;
        }

        /** Get just the short flags from an initializer list of EitherFlags
        */
        static Set<Char> GetShort(std::initializer_list<EitherFlag> flags) {
            Set<Char> shortFlags;
            for (const EitherFlag &flag : flags) {
                if (flag.isShort) {
                    shortFlags.insert(flag.shortFlag);
                }
            }
            return shortFlags;
        }
    };

    /** A class of "matchers", specifying short and flags that can possibly be
     * matched.
     *
     * This is supposed to be constructed and then passed in, not used directly
     * from user code.
     */
    class Matcher {
        private:
        const Set<Char> _short_flags;
        const Set<String> _long_flags;

        public:
        /** Specify short and long flags separately as iterators
         *
         * ex: `args::Matcher(_short_flags.begin(), _short_flags.end(),
         * _long_flags.begin(), _long_flags.end())`
         */
        template <typename ShortIt, typename LongIt>
        Matcher(ShortIt _short_flags_start, ShortIt _short_flags_end,
            LongIt _long_flags_start, LongIt _long_flags_end)
            : _short_flags(_short_flags_start, _short_flags_end),
              _long_flags(_long_flags_start, _long_flags_end) {}

        /** Specify short and long flags separately as iterables
         *
         * ex: `args::Matcher(_short_flags, _long_flags)`
         */
        template <typename Short, typename Long>
        Matcher(Short &&shortIn, Long &&longIn)
            : _short_flags(std::begin(shortIn), std::end(shortIn)),
              _long_flags(std::begin(longIn), std::end(longIn)) {}

        /** Specify a mixed single initializer-list of both short and long flags
         *
         * This is the fancy one.  It takes a single initializer list of
         * any number of any mixed kinds of flags.  Chars are
         * automatically interpreted as short flags, and strings are
         * automatically interpreted as long flags:
         *
         *     args::Matcher{'a'}
         *     args::Matcher{"foo"}
         *     args::Matcher{'h', "help"}
         *     args::Matcher{"foo", 'f', 'F', "FoO"}
         */
        Matcher(std::initializer_list<EitherFlag> in)
            : _short_flags(EitherFlag::GetShort(in)),
              _long_flags(EitherFlag::GetLong(in)) {}

        Matcher(Matcher &&other)
            : _short_flags(std::move(other._short_flags)),
              _long_flags(std::move(other._long_flags)) {}

        ~Matcher() {}

        bool Match(const Char &flag) const {
            return _short_flags.find(flag) != std::end(_short_flags);
        }

        bool Match(const String &flag) const {
            return _long_flags.find(flag) != std::end(_long_flags);
        }
    };

    class Base {
        private:
        String _name;
        String _help;
        bool _matched;

        Base(const Base &) = delete;

        public:
        Base(const String &name) : _name(name), _matched(false) {}

        Base(Base &&other)
            : _name(std::move(other._name)), _help(std::move(other._help)) {}

        const String &Name() const { return _name; }

        Base &Name(const String &name) {
            _name = name;
            return *this;
        }

        const String &Help() const { return _help; }

        Base &Help(const String &help) {
            _help = help;
            return *this;
        }

        bool Matched() const {
            return _matched;
        }

        Base &Matched(bool matched) {
            _matched = matched;
            return *this;
        }

    };

    class OptionBase : public Base {
        private:
        const Matcher _matcher;

        OptionBase(const OptionBase &) = delete;

        public:
        OptionBase(const String &name, Matcher &&matcher)
            : Base(name), _matcher(std::move(matcher)) {}

        OptionBase(OptionBase &&other) : Base(std::move(other)) {}
    };

    template <typename Type>
    class Option : public OptionBase {
        private:
        Type _value;

        Option(const Option &) = delete;

        public:
        Option(const String &name, Matcher &&matcher)
            : OptionBase(name, std::move(matcher)) {
            std::cout << "Out of bool" << std::endl;
        }

        Option(Option &&other)
            : OptionBase(std::move(other)), _value(std::move(other.value)) {}

        const Type &Default() const {
            return _value;
        }

        Option &Default(const Type &defaultvalue) {
            _value = defaultvalue;
            return *this;
        }
        const Type &Value() const {
            return _value;
        }

        Option &Value(const Type &value) {
            _value = value;
            return *this;
        }
    };

    String _prog;
    String _description;
    String _epilog;
    String _long_prefix;
    String _short_prefix;
    String _long_separator;
    String _option_terminator;
    bool _joined_short;
    bool _joined_long;
    bool _separate_short;
    bool _separate_long;

    // Need pointers for virtual functions
    List<std::unique_ptr<OptionBase>> _options;

    public:
    ArgumentParser(const String &description = String{},
        const String &epilog = String{}, const String &prog = String{})
        : _prog(prog), _description(description), _epilog(epilog),
          _long_prefix("--"), _short_prefix("-"), _long_separator("="),
          _option_terminator("--"), _joined_short(true), _joined_long(true),
          _separate_short(true), _separate_long(true) {}

    template <typename Value>
    Option<Value> &AddOption(const String &name, Matcher matcher) {
        // Create an option object, add the pointer to a unique pointer
        // (implying ownership) to the option array, then return a
        // reference to it.
        auto opt = new Option<Value>(name, std::move(matcher));
        _options.emplace_back(opt);
        return *opt;
    }

    /** Parse all arguments.
     *
     * \param begin an iterator to the beginning of the argument list
     * \param end an iterator to the past-the-end element of the argument list
     * \return the iterator after the last parsed value.  Only useful for
     * kick-out
     */
    template <typename It>
    bool ParseArgs(It begin, It end) {
        bool terminated = false;

        // Check all arg chunks
        for (auto it = begin; it != end; ++it) {
            const auto &chunk = *it;

            if (!terminated && chunk == _option_terminator) {
                terminated = true;
                // If a long arg was found
            } else if (!terminated && chunk.find(_long_prefix) == 0 &&
                chunk.size() > _long_prefix.size()) {
                const auto argchunk = chunk.substr(_long_prefix.size());
                // Try to separate it, in case of a separator:
                const auto separator = _long_separator.empty()
                    ? argchunk.npos
                    : argchunk.find(_long_separator);
                // If the separator is in the argument, separate it.
                const auto arg = (separator != argchunk.npos
                        ? std::string(argchunk, 0, separator)
                        : argchunk);

                if (auto base = Match(arg)) {
                    if (auto argbase = dynamic_cast<ValueFlagBase *>(base)) {
                        if (separator != argchunk.npos) {
                            if (_joined_long) {
                                argbase->ParseValue(argchunk.substr(
                                    separator + _long_separator.size()));
                            } else {
                                std::ostringstream problem;
                                problem
                                    << "Flag '" << arg
                                    << "' was passed a joined argument, but these are disallowed";
                                throw std::runtime_error(problem.str());
                            }
                        } else {
                            ++it;
                            if (it == end) {
                                std::ostringstream problem;
                                problem
                                    << "Flag '" << arg
                                    << "' requires an argument but received none";
                                throw std::runtime_error(problem.str());
                            }

                            if (_separate_long) {
                                argbase->ParseValue(*it);
                            } else {
                                std::ostringstream problem;
                                problem
                                    << "Flag '" << arg
                                    << "' was passed a separate argument, but these are disallowed";
                                throw std::runtime_error(problem.str());
                            }
                        }
                    } else if (separator != argchunk.npos) {
                        std::ostringstream problem;
                        problem
                            << "Passed an argument into a non-argument flag: "
                            << chunk;
                        throw std::runtime_error(problem.str());
                    }

                    if (base->KickOut()) {
                        return ++it;
                    }
                } else {
                    std::ostringstream problem;
                    problem << "Flag could not be matched: " << arg;
                    throw std::runtime_error(problem.str());
                }
                // Check short args
            } else if (!terminated && chunk.find(_short_prefix) == 0 &&
                chunk.size() > _short_prefix.size()) {
                const auto argchunk = chunk.substr(_short_prefix.size());
                for (auto argit = std::begin(argchunk);
                     argit != std::end(argchunk); ++argit) {
                    const auto arg = *argit;

                    if (auto base = Match(arg)) {
                        if (auto argbase =
                                dynamic_cast<ValueFlagBase *>(base)) {
                            const std::string value(
                                ++argit, std::end(argchunk));
                            if (!value.empty()) {
                                if (_joined_short) {
                                    argbase->ParseValue(value);
                                } else {
                                    std::ostringstream problem;
                                    problem
                                        << "Flag '" << arg
                                        << "' was passed a joined argument, but these are disallowed";
                                    throw std::runtime_error(problem.str());
                                }
                            } else {
                                ++it;
                                if (it == end) {
                                    std::ostringstream problem;
                                    problem
                                        << "Flag '" << arg
                                        << "' requires an argument but received none";
                                    throw std::runtime_error(problem.str());
                                }

                                if (_separate_short) {
                                    argbase->ParseValue(*it);
                                } else {
                                    std::ostringstream problem;
                                    problem
                                        << "Flag '" << arg
                                        << "' was passed a separate argument, but these are disallowed";
                                    throw std::runtime_error(problem.str());
                                }
                            }
                            // Because this argchunk is done regardless
                            break;
                        }

                        if (base->KickOut()) {
                            return ++it;
                        }
                    } else {
                        std::ostringstream problem;
                        problem << "Flag could not be matched: '" << arg << "'";
                        throw std::runtime_error(problem.str());
                    }
                }
            } else {
                auto pos = GetNextPositional();
                if (pos) {
                    pos->ParseValue(chunk);

                    if (pos->KickOut()) {
                        return ++it;
                    }
                } else {
                    std::ostringstream problem;
                    problem
                        << "Passed in argument, but no positional arguments were ready to receive it: "
                        << chunk;
                    throw std::runtime_error(problem.str());
                }
            }
        }
        return end;
    }

    /** Parse all arguments.
     *
     * \param args an iterable of the arguments
     * \return the iterator after the last parsed value.  Only useful for
     * kick-out
     */
    template <typename T>
    auto ParseArgs(const T &args) -> decltype(std::begin(args)) {
        return ParseArgs(std::begin(args), std::end(args));
    }

    /** Convenience function to parse the CLI from argc and argv
     *
     * Just assigns the program name and vectorizes arguments for passing into
     * ParseArgs()
     *
     * \return whether or not all arguments were parsed.  This works for
     * detecting kick-out, but is generally useless as it can't do anything with
     * it.
     */
    bool ParseCLI(const int argc, const char *const *argv) {
        if (_prog.empty()) {
            _prog = String(argv[0]);
        }
        const List<String> args(argv + 1, argv + argc);
        return ParseArgs(args);
    }
};
}

#endif
