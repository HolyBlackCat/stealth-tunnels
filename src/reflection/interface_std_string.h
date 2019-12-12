#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <type_traits>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_container.h"
#include "utils/escape.h"
#include "utils/robust_math.h"

namespace Refl
{
    class Interface_StdString : public InterfaceBasic<std::string>
    {
      public:
        void ToString(const std::string &object, Stream::Output &output, const ToStringOptions &options) const override
        {
            Strings::EscapeFlags flags = Strings::EscapeFlags::escape_double_quotes;
            if (options.multiline)
                flags = flags | Strings::EscapeFlags::multiline;

            output.WriteByte('"');
            Strings::Escape(object, output.GetOutputIterator(), flags);
            output.WriteByte('"');
        }

        void FromString(std::string &object, Stream::Input &input, const FromStringOptions &options) const override
        {
            (void)options;
            input.Discard('"');
            std::string temp_str;
            while (true)
            {
                char ch = input.ReadChar();
                if (ch == '"')
                    break;

                temp_str += ch;
                if (ch == '\\')
                    temp_str += input.ReadChar();
            }

            try
            {
                object = Strings::Unescape(temp_str);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }
        }

        void ToBinary(const std::string &object, Stream::Output &output) const override
        {
            impl::container_length_binary_t len;
            if (Robust::conversion_fails(object.size(), len))
                Program::Error(output.GetExceptionPrefix() + "The string is too long.");

            output.WriteWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order, len);
            output.WriteString(object);
        }

        void FromBinary(std::string &object, Stream::Input &input, const FromBinaryOptions &options) const override
        {
            std::size_t len;
            if (Robust::conversion_fails(input.ReadWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order), len))
                Program::Error(input.GetExceptionPrefix() + "The string is too long.");

            object = {};
            object.reserve(len < options.max_reserved_size ? len : options.max_reserved_size);
            while (len-- > 0)
                object += input.ReadChar();
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, std::enable_if_t<std::is_same_v<T, std::string>>>
    {
        using type = Interface_StdString;
    };

    template <>
    struct impl::ForceNotContainer<std::string> : std::true_type {};
}
