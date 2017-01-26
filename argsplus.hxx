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

    /** A class of "matchers", specifying short and long flags that can possibly
     * be matched.
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

        virtual ~Base(){};

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

        bool Matched() const { return _matched; }

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

        virtual ~OptionBase(){};

        template <typename T>
        bool Match(const T &flag) const {
            return _matcher.Match(flag);
        }
    };

    class ValueOptionBase : public OptionBase {
        private:
        ValueOptionBase(const ValueOptionBase &) = delete;

        public:
        ValueOptionBase(const String &name, Matcher &&matcher)
            : OptionBase(name, std::move(matcher)) {}

        ValueOptionBase(ValueOptionBase &&other)
            : OptionBase(std::move(other)) {}

        virtual ~ValueOptionBase(){};

        virtual bool ParseValue(const String &value) = 0;
    };

    template <typename Type>
    class Option : public ValueOptionBase {
        private:
        Type _value;

        Option(const Option &) = delete;

        public:
        Option(const String &name, Matcher &&matcher)
            : ValueOptionBase(name, std::move(matcher)) {}

        Option(Option &&other)
            : ValueOptionBase(std::move(other)),
              _value(std::move(other.value)) {}

        virtual ~Option(){};

        const Type &Default() const { return _value; }

        Option &Default(const Type &defaultvalue) {
            _value = defaultvalue;
            return *this;
        }
        const Type &Value() const { return _value; }

        Option &Value(const Type &value) {
            _value = value;
            return *this;
        }

        virtual bool ParseValue(const String &value) override {
            std::basic_istringstream<Char> ss(value);
            ss >> _value;
            if (ss.rdbuf()->in_avail() > 0 || ss.fail()) {
                return false;
            }
            return true;
        }
    };

    class PositionalBase : public Base {
        private:
        PositionalBase(const PositionalBase &) = delete;

        public:
        PositionalBase(const String &name) : Base(name) {}

        PositionalBase(PositionalBase &&other) : Base(std::move(other)) {}

        virtual ~PositionalBase(){};

        virtual bool ParseValue(const String &value) = 0;
    };

    template <typename Type>
    class Positional : public PositionalBase {
        private:
        Type _value;

        Positional(const Positional &) = delete;

        public:
        Positional(const String &name) : PositionalBase(name) {}

        Positional(Positional &&other)
            : PositionalBase(std::move(other)), _value(std::move(other.value)) {
        }

        virtual ~Positional(){};

        const Type &Default() const { return _value; }

        Positional &Default(const Type &defaultvalue) {
            _value = defaultvalue;
            return *this;
        }
        const Type &Value() const { return _value; }

        Positional &Value(const Type &value) {
            _value = value;
            return *this;
        }

        virtual bool ParseValue(const String &value) override {
            std::basic_istringstream<Char> ss(value);
            ss >> _value;
            if (ss.rdbuf()->in_avail() > 0 || ss.fail()) {
                return false;
            }
            return true;
        }
    };

    String _prog;
    String _description;
    String _epilog;
    String _long_prefix;
    String _short_prefix;
    String _long_separator;
    String _option_terminator;
    String _error;
    String _long_error;
    bool _joined_short;
    bool _joined_long;
    bool _separate_short;
    bool _separate_long;

    // Need pointers for virtual functions
    List<std::unique_ptr<OptionBase>> _options;
    List<std::unique_ptr<PositionalBase>> _positionals;

    template <typename T>
    OptionBase *MatchOption(const T &flag) {
        for (const auto &option : _options) {
            if (option->Match(flag)) {
                return option.get();
            }
        }
        return nullptr;
    }

    PositionalBase *GetNextPositional() {
        for (const auto &positional : _positionals) {
            if (!positional->Matched()) {
                return positional.get();
            }
        }
        return nullptr;
    }

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

    template <typename Value>
    Positional<Value> &AddPositional(const String &name) {
        auto pos = new Positional<Value>(name);
        _positionals.emplace_back(pos);
        return *pos;
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
                const auto arg =
                    (separator != argchunk.npos ? String(argchunk, 0, separator)
                                                : argchunk);

                if (auto option = MatchOption(arg)) {
                    option->Matched(true);
                    if (auto value_option =
                            dynamic_cast<ValueOptionBase *>(option)) {
                        if (separator != argchunk.npos) {
                            if (_joined_long) {
                                if (!value_option->ParseValue(argchunk.substr(
                                        separator + _long_separator.size()))) {
                                    _error.assign("Flag '");
                                    _error.append(arg);
                                    _error.append(
                                        "' received an invalid value");
                                    return false;
                                }
                            } else {
                                _error.assign("Flag '");
                                _error.append(arg);
                                _error.append(
                                    "' was passed a joined argument, but these are disallowed");
                                return false;
                            }
                        } else {
                            ++it;
                            if (it == end) {
                                _error.assign("Flag '");
                                _error.append(arg);
                                _error.append(
                                    "' requires an argument but received none");
                                return false;
                            }

                            if (_separate_long) {
                                if (!value_option->ParseValue(*it)) {
                                    _error.assign("Flag '");
                                    _error.append(arg);
                                    _error.append(
                                        "' received an invalid value");
                                    return false;
                                }
                            } else {
                                _error.assign("Flag '");
                                _error.append(arg);
                                _error.append(
                                    "' was passed a separate argument, but these are disallowed");
                                return false;
                            }
                        }
                    } else if (separator != argchunk.npos) {
                        _error.assign(
                            "Passed an argument into a non-argument flag: ");
                        _error.append(chunk);
                        return false;
                    }
                } else {
                    _error.assign("Flag could not be matched: ");
                    _error.append(arg);
                    return false;
                }
                // Check short args
            } else if (!terminated && chunk.find(_short_prefix) == 0 &&
                chunk.size() > _short_prefix.size()) {
                const auto argchunk = chunk.substr(_short_prefix.size());
                for (auto argit = std::begin(argchunk);
                     argit != std::end(argchunk); ++argit) {
                    const auto arg = *argit;

                    if (auto option = MatchOption(arg)) {
                        option->Matched(true);
                        if (auto value_option =
                                dynamic_cast<ValueOptionBase *>(option)) {
                            const String value(++argit, std::end(argchunk));
                            if (!value.empty()) {
                                if (_joined_short) {
                                    if (!value_option->ParseValue(value)) {
                                        _error.assign("Flag '");
                                        _error.append(1, arg);
                                        _error.append(
                                            "' received an invalid value");
                                        return false;
                                    }
                                } else {
                                    _error.assign("Flag '");
                                    _error.append(1, arg);
                                    _error.append(
                                        "' was passed a joined argument, but these are disallowed");
                                    return false;
                                }
                            } else {
                                ++it;
                                if (it == end) {
                                    _error.assign("Flag '");
                                    _error.append(1, arg);
                                    _error.append(
                                        "' requires an argument but received none");
                                    return false;
                                }

                                if (_separate_short) {
                                    if (!value_option->ParseValue(*it)) {
                                        _error.assign("Flag '");
                                        _error.append(1, arg);
                                        _error.append(
                                            "' received an invalid value");
                                        return false;
                                    }
                                } else {
                                    _error.assign("Flag '");
                                    _error.append(1, arg);
                                    _error.append(
                                        "' was passed a separate argument, but these are disallowed");
                                    return false;
                                }
                            }
                            // Because this argchunk is done regardless, because
                            // a value option flag was just encountered
                            break;
                        }
                    } else {
                        _error.assign("Flag could not be matched: ");
                        _error.append(1, arg);
                        return false;
                    }
                }
            } else {
                if (auto pos = GetNextPositional()) {
                    if (!pos->ParseValue(chunk)) {
                        _error.assign("Positional '");
                        _error.append(pos->Name());
                        _error.append("' received an invalid value");
                        return false;
                    }
                    pos->Matched(true);
                } else {
                    _error.assign(
                        "Passed in argument, but no positional arguments were ready to receive it: ");
                    _error.append(chunk);
                    return false;
                }
            }
        }
        return true;
    }

    /** Parse all arguments.
     *
     * \param args an iterable of the arguments
     * \return the iterator after the last parsed value.  Only useful for
     * kick-out
     */
    template <typename T>
    bool ParseArgs(const T &args) {
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

    const String &Error() const { return _error; }

    ArgumentParser &Error(const String &error) { _error = error; }
};
}

#endif
