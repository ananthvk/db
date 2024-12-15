#ifndef A_PAGE_H
#define A_PAGE_H

#include "bufferpool.h"
#include "datapacker.h"

namespace pinedb
{
    inline std::string read_fixed_length_string(uint8_t *buffer, size_t string_length)
    {
        std::string result;
        result.reserve(string_length);
        for (size_t i = 0; i < string_length; ++i)
        {
            if (buffer[i] == '\0')
                break;
            result.push_back(static_cast<char>(buffer[i]));
        }
        return result;
    }

    inline void write_fixed_length_string(uint8_t *buffer, const std::string &s,
                                          size_t string_length)
    {
        if (s.size() > string_length)
        {
            throw std::runtime_error("Length exceeds maximum permitted length"
                                     + std::to_string(string_length));
        }
        for (size_t i = 0; i < s.size(); ++i)
        {
            *buffer++ = static_cast<uint8_t>(s[i]);
        }
        for (auto i = s.size(); i < string_length; ++i)
        {
            *buffer++ = '\0';
        }
    }

    class PageHeader
    {
        uint8_t page_type;
        page_id_type page_id;

      public:
        PageHeader() : page_type(0), page_id(0) {}

        // Write page header
        uint8_t *write(uint8_t *buffer)
        {
            uint64_t reserved = 0;
            uint8_t reserved_byte = 0;
            return buffer
                   + datapacker::bytes::encode_le(buffer, page_type, reserved, page_id,
                                                  reserved_byte, reserved_byte, reserved_byte);
        }

        uint8_t *read(uint8_t *buffer)
        {
            uint64_t reserved = 0;
            uint8_t reserved_byte = 0;
            return buffer
                   + datapacker::bytes::decode_le(buffer, page_type, reserved, page_id,
                                                  reserved_byte, reserved_byte, reserved_byte);
        }

        uint8_t get_page_type() const { return page_type; }

        page_id_type get_page_id() const { return page_id; }

        void set_page_type(uint8_t page_type) { this->page_type = page_type; }

        void set_page_id(page_id_type page_id) { this->page_id = page_id; }
    };

    class ColumnFormat
    {
      public:
        static char validate_format(const std::string &format_string)
        {
            for (const auto ch : format_string)
            {
                if (!(ch == 'b' || ch == 'B' || ch == 's' || ch == 'S' || ch == 'i' || ch == 'I'
                      || ch == 'l' || ch == 'L' || ch == 'f' || ch == 'd' || ch == 'c'))
                    return ch;
            }
            return '\0';
        }

        static std::string human_readable_format(char ch)
        {
            std::string name;
            switch (ch)
            {
            case 'b':
                name = "unsigned byte";
                break;
            case 'B':
                name = "byte";
                break;
            case 's':
                name = "unsigned short";
                break;
            case 'S':
                name = "short";
                break;
            case 'i':
                name = "unsigned int";
                break;
            case 'I':
                name = "int";
                break;
            case 'l':
                name = "unsigned long";
                break;
            case 'L':
                name = "long";
                break;
            case 'f':
                name = "float";
                break;
            case 'd':
                name = "double";
                break;
            case 'c':
                name = "string";
                break;
            default:
                throw std::logic_error("invalid format string");
            }
            return name;
        }

        static std::vector<std::string> human_readable_format(const std::string &format_string)
        {
            std::vector<std::string> result;
            for (const auto ch : format_string)
            {
                result.push_back(human_readable_format(ch));
            }
            return result;
        }
    };

    class TableMetadataPage
    {
        uint8_t *buffer;

      public:
        static const uint8_t PAGE_TYPE = 0x54;
        static const int MAX_TABLE_NAME_LENGTH = 128;
        static const int MAX_NUMBER_OF_COLUMNS = 64;
        static const int MAX_COLUMN_NAME_LENGTH = 59;

        // The buffer passed here is not the page buffer, but the page data buffer
        // which is after the page header
        TableMetadataPage(uint8_t *buffer) : buffer(buffer) {}

        std::string get_table_name() const
        {
            return read_fixed_length_string(buffer, MAX_TABLE_NAME_LENGTH);
        }

        void set_table_name(const std::string &name)
        {
            write_fixed_length_string(buffer, name, MAX_TABLE_NAME_LENGTH);
        }

        std::string get_column_format() const
        {
            return read_fixed_length_string(buffer + MAX_TABLE_NAME_LENGTH, MAX_NUMBER_OF_COLUMNS);
        }

        void set_column_format(const std::string &format)
        {
            if (ColumnFormat::validate_format(format) != '\0')
            {
                throw std::runtime_error("invalid column data type: "
                                         + std::string(1, ColumnFormat::validate_format(format)));
            }
            write_fixed_length_string(buffer + MAX_TABLE_NAME_LENGTH, format,
                                      MAX_NUMBER_OF_COLUMNS);
        }

        void set_table_page_id(page_id_type page_id)
        {
            datapacker::bytes::encode_le(buffer + MAX_TABLE_NAME_LENGTH + MAX_NUMBER_OF_COLUMNS,
                                         page_id);
        }

        int number_of_columns() const { return get_column_format().size(); }

        page_id_type get_table_page_id() const
        {
            page_id_type page_id = 0;
            datapacker::bytes::decode_le(buffer + MAX_TABLE_NAME_LENGTH + MAX_NUMBER_OF_COLUMNS,
                                         page_id);
            return page_id;
        }

        std::string get_column_name(int index) const
        {
            if (index > MAX_NUMBER_OF_COLUMNS)
            {
                throw std::logic_error("column index exceeds max number of columns");
            }
            auto offset = MAX_TABLE_NAME_LENGTH + MAX_NUMBER_OF_COLUMNS + sizeof(page_id_type);
            offset += index * MAX_COLUMN_NAME_LENGTH;
            return read_fixed_length_string(buffer + offset, MAX_COLUMN_NAME_LENGTH);
        }

        void set_column_name(int index, const std::string &s)
        {
            if (index > MAX_NUMBER_OF_COLUMNS)
            {
                throw std::logic_error("column index exceeds max number of columns");
            }
            auto offset = MAX_TABLE_NAME_LENGTH + MAX_NUMBER_OF_COLUMNS + sizeof(page_id_type);
            offset += index * MAX_COLUMN_NAME_LENGTH;
            write_fixed_length_string(buffer + offset, s, MAX_COLUMN_NAME_LENGTH);
        }
    };

} // namespace pinedb
#endif