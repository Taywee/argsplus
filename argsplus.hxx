/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef ARGSPLUS_HXX
#define ARGSPLUS_HXX

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <initializer_list>

namespace argsplus {
    template <typename String = std::string,
         typename Char = char,
         template <typename...> class List = std::vector,
         template <typename...> class Set = std::unordered_set>
    class ArgumentParser {
        private:
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

            class Base {
                private:
                    String _name;
                    String _help;

                    Base(const Base &) = delete;

                public:
                    Base(const String &name) :
                        _name(name) {}

                    Base(Base &&other) :
                        _name(std::move(other._name)),
                        _help(std::move(other._help)) {}

                    const String &Name() {
                        return _name;
                    }

                    Base &Name(const String &name) {
                        _name = name;
                        return *this;
                    }

                    const String &Help() {
                        return _help;
                    }

                    Base &Help(const String &help) {
                        _help = help;
                        return *this;
                    }
            };

            // Need pointers for virtual functions
            List<std::unique_ptr<Base>> _options;

            template <typename Type>
            class Option : public Base {
                private:
                    Type _value;

                    Option(const Option &) = delete;

                public:
                    Option(const String &name) :
                        Base(name) {}

                    Option(Option &&other) :
                        Base(std::move(other)),
                        _value(std::move(other.value)) {}

                    Base &Default(const Type &_default) {
                        _value = _default;
                        return *this;
                    }
            };

            /** A simple unified option type for unified initializer lists for the Matcher class.
            */
            struct EitherFlag
            {
                const bool isShort;
                const Char shortFlag;
                const String longFlag;
                EitherFlag(const String &flag) : isShort(false), shortFlag(), longFlag(flag) {}
                EitherFlag(const Char *flag) : isShort(false), shortFlag(), longFlag(flag) {}
                EitherFlag(const Char flag) : isShort(true), shortFlag(flag), longFlag() {}

                /** Get just the long flags from an initializer list of EitherFlags
                */
                static Set<String> GetLong(std::initializer_list<EitherFlag> flags)
                {
                    Set<String>  longFlags;
                    for (const EitherFlag &flag: flags)
                    {
                        if (!flag.isShort)
                        {
                            longFlags.insert(flag.longFlag);
                        }
                    }
                    return longFlags;
                }

                /** Get just the short flags from an initializer list of EitherFlags
                */
                static Set<Char> GetShort(std::initializer_list<EitherFlag> flags)
                {
                    Set<Char>  shortFlags;
                    for (const EitherFlag &flag: flags)
                    {
                        if (flag.isShort)
                        {
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
            class Matcher
            {
                private:
                    const Set<Char> shortFlags;
                    const Set<String> longFlags;

                public:
                    /** Specify short and long flags separately as iterators
                     *
                     * ex: `args::Matcher(shortFlags.begin(), shortFlags.end(), longFlags.begin(), longFlags.end())`
                     */
                    template <typename ShortIt, typename LongIt>
                        Matcher(ShortIt shortFlagsStart, ShortIt shortFlagsEnd, LongIt longFlagsStart, LongIt longFlagsEnd) :
                            shortFlags(shortFlagsStart, shortFlagsEnd),
                            longFlags(longFlagsStart, longFlagsEnd)
                {}

                    /** Specify short and long flags separately as iterables
                     *
                     * ex: `args::Matcher(shortFlags, longFlags)`
                     */
                    template <typename Short, typename Long>
                        Matcher(Short &&shortIn, Long &&longIn) :
                            shortFlags(std::begin(shortIn), std::end(shortIn)), longFlags(std::begin(longIn), std::end(longIn))
                {}

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
                    Matcher(std::initializer_list<EitherFlag> in) :
                        shortFlags(EitherFlag::GetShort(in)), longFlags(EitherFlag::GetLong(in)) {}

                    Matcher(Matcher &&other) : shortFlags(std::move(other.shortFlags)), longFlags(std::move(other.longFlags))
                {}

                    ~Matcher() {}
            };

        public:
            ArgumentParser(const String &description = String{}, const String &epilog = String{}) :
                _description(description),
                _epilog(epilog),
                _long_prefix("--"),
                _short_prefix("-"),
                _long_separator("="),
                _option_terminator("--"),
                _joined_short(true),
                _joined_long(true),
                _separate_short(true),
                _separate_long(true) {}

            template <typename Value>
            Option<Value> &AddArgument(const String &name, Matcher matcher) {
                // Create an option object, add the pointer to a unique pointer
                // (implying ownership) to the option array, then return a
                // reference to it.
                auto opt = new Option<Value>(name);
                _options.emplace_back(opt);
                return *opt;
            }
    };
}

#endif
