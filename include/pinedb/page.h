#ifndef A_PAGE_H
#define A_PAGE_H

#include "bufferpool.h"
#include "datapacker.h"


namespace pinedb
{
    // A catalog page holds data about the tables/collections
    // Format:
    // Page identifier        | 1 byte
    // Next catalog page id   | 4 byte
    // Number of records used | 1 byte
    // A set of records which follow the following format
    // [Table name 128 bytes (including terminal \0)] | [Table id 4 bytes]

    // A catalog record is fixed length of 132 bytes, so in a 4096 byte page, 30 such records can be
    // stored, after which the next record has to be examined

    class CatalogPage
    {
        uint8_t num_records_used = 0;
        page_id_type next_catalog_id = 0;
        std::map<std::string, uint32_t> table_name_table_id_map;

      public:
        static const uint8_t PAGE_IDENTIFIER = 0xC;
        static const uint8_t MAX_RECORDS = 30; // (4096-6) / 132

        void save(uint8_t *buffer)
        {
            num_records_used = table_name_table_id_map.size();
            auto initial_offset = datapacker::bytes::encode_le(buffer, PAGE_IDENTIFIER,
                                                               next_catalog_id, num_records_used);
            if (table_name_table_id_map.size() > 30)
            {
                throw std::logic_error("More than 30 records cannot be stored in one catalog page");
            }
            buffer = buffer + initial_offset;

            auto iter = table_name_table_id_map.begin();
            for (int i = 0; i < num_records_used; ++i)
            {
                int offset = i * 132;
                std::string table_name = iter->first;
                uint32_t table_id = iter->second;

                // Encode the table name
                for (size_t j = 0; j < 128; ++j)
                {
                    if (j < table_name.size())
                        buffer[offset + j] = static_cast<uint8_t>(table_name[j]);
                    else
                        buffer[offset + j] = 0;
                }

                // Encode the table id
                datapacker::bytes::encode_le(buffer + offset + 128, table_id);
                ++iter;
            }
        }

        bool load(uint8_t *buffer)
        {
            uint8_t identifier;
            buffer += datapacker::bytes::decode_le(buffer, identifier, next_catalog_id, num_records_used);
            if (identifier != PAGE_IDENTIFIER)
                return false;
            for (int i = 0; i < num_records_used; ++i)
            {
                int offset = i * 132;
                std::string table_name(128, '\0');
                for (int j = 0; j < 128; ++j)
                {
                    // If it is null, do not add it to the string
                    if (buffer[offset + j] == 0)
                    {
                        table_name.resize(j);
                        break;
                    }
                    table_name[j] = (static_cast<char>(buffer[offset + j]));
                }
                uint32_t table_id;
                datapacker::bytes::decode_le(buffer + offset + 128, table_id);
                table_name_table_id_map[table_name] = table_id;
            }

            return true;
        }

        auto begin() { return table_name_table_id_map.begin(); }

        auto end() { return table_name_table_id_map.end(); }

        auto cbegin() const { return table_name_table_id_map.cbegin(); }

        auto cend() const { return table_name_table_id_map.cend(); }

        uint32_t &operator[](const std::string &table_name)
        {
            return table_name_table_id_map[table_name];
        }

        bool exists(const std::string &table_name)
        {
            return table_name_table_id_map.find(table_name) != table_name_table_id_map.end();
        }

        auto erase(std::map<std::string, uint32_t>::iterator iter)
        {
            return table_name_table_id_map.erase(iter);
        }
    };
} // namespace pinedb
#endif