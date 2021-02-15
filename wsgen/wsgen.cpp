#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

struct page_template
{
    std::filesystem::path src_path;
    std::filesystem::path rel_path;
    std::filesystem::path out_path;
};

void text_to_htm(char* dst, const char* src, bool literal)
{
    const unsigned char* pos = (const unsigned char*)src;
    while (*pos)
    {
        switch (*pos)
        {
        case '&':
            sprintf(dst, "&amp;");
            dst += 5;
            break;
        case '<':
            sprintf(dst, "&lt;");
            dst += 4;
            break;
        case '>':
            sprintf(dst, "&gt;");
            dst += 4;
            break;
        case 195:
            switch (*++pos)
            {
            case 156:
                sprintf(dst, "&Uuml;");
                dst += 6;
                break;
            case 188:
                sprintf(dst, "&uuml;");
                dst += 6;
                break;
            }
            break;
        default:
            *dst = (char)*pos;
            ++dst;
        }
        ++pos;
    }
    *dst = 0;
}

void generate_pages(std::vector<page_template>& templates, const char* header, const char* footer)
{
    static char str_buffer[UINT16_MAX];
    static char htm_buffer[UINT16_MAX];
    for (page_template& htpl : templates)
    {
        std::ifstream input;
        std::ofstream output;
        bool out_literal = false;
        input.open(htpl.src_path, std::ifstream::in);  
        output.open(htpl.out_path, std::ifstream::out);        
        if (header) output << header;
        while (input.good())
        {
            input.getline(str_buffer, UINT16_MAX);
            if (!*str_buffer)
                continue;
            if (out_literal)
            {
                if (strcmp(str_buffer, ">>>") == 0)
                {
                    output << "</pre>\n";
                    out_literal = false;
                }
                else
                {
                    text_to_htm(htm_buffer, str_buffer, out_literal);
                    output << '\n' << htm_buffer;
                }
            }
            else
            {
                if (strcmp(str_buffer, "<<<") == 0)
                {
                    output << "<pre>";
                    out_literal = true;
                }
                else if (*str_buffer == '!')
                {
                    text_to_htm(htm_buffer, str_buffer, out_literal);
                    const char* str = htm_buffer;
                    int level = 0;
                    while (*str == '!')
                    {
                        ++level;
                        ++str;
                    }
                    if (level > 5) level = 5;
                    output << "<h" << level << ">" << str <<"</h" << level << ">\n";
                }
                else
                {
                    text_to_htm(htm_buffer, str_buffer, out_literal);
                    output << "<p>" << htm_buffer << "</p>\n";
                }
            }
        }
        if (footer) output << footer;
        output.close();
        input.close();
    }
}

void load_file_as_string(const char* str_path, char** out)
{
    if (std::filesystem::exists(str_path))
    {
        std::filesystem::path src_path(str_path);
        std::filesystem::directory_entry entry(src_path);
        if (entry.is_regular_file())
        {
            std::ifstream input;
            std::streamsize offs = 0, size = entry.file_size();
            char* buffer = new char[size + 1];
            input.open(src_path, std::ifstream::in);
            while (input.good())
            {
                input.getline(&buffer[offs], size);
                std::streamsize count = input.gcount();
                buffer[offs + count - 1] = '\n';
                offs += count;
                size -= count;
            }
            input.close();
            *out = buffer;
        }
    }   
}

void collect_templates(const char* srd_dir, const char* dst_dir, std::vector<page_template>& out)
{
    std::filesystem::path src_path = (srd_dir) 
        ? std::filesystem::path(srd_dir) 
        : std::filesystem::current_path();
    std::filesystem::path dst_path = (dst_dir) 
        ? std::filesystem::path(dst_dir) 
        : src_path;
    for (auto& p : std::filesystem::recursive_directory_iterator(src_path))
    {
        if (p.is_regular_file() && (p.path().extension().compare(".htpl") == 0))
        {   
            page_template htpl;
            htpl.src_path = p.path();
            htpl.rel_path = std::filesystem::relative(p.path(), src_path).replace_extension(".html");
            htpl.out_path = dst_path / htpl.rel_path;
            out.emplace_back(htpl);
        }        
    }
}

int main(int argc, char** argv)
{
    const char* src_dir = nullptr;
    const char* dst_dir = nullptr;
    for (int i = 1; i < argc; i++)
    { 
        const char* arg = argv[i];
        if (arg[0] == '-')
            switch (arg[1])
            {
            case 'S':
                src_dir = argv[++i];
                break;
            case 'D':
                dst_dir = argv[++i];
                break;
            default:
                std::cerr << "Invalid argument: '" << arg << "'" << std::endl;
                return -1;
            }
    }
    char* header = nullptr;
    char* footer = nullptr;
    std::vector<page_template> templates;
    try
    {
        load_file_as_string("header.htpl", &header);
        load_file_as_string("footer.htpl", &footer);
        collect_templates(src_dir, dst_dir, templates);
        generate_pages(templates, header, footer);
        delete[] header;
        delete[] footer;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }    
    return 0;
}
