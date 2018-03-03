#include "FileFormats/Gff.hpp"
#include "Utility/Assert.hpp"

namespace {

bool ReadAllButes(const char* path, std::vector<std::byte>* out)
{
    FILE* file = std::fopen(path, "rb");

    if (file)
    {
        std::fseek(file, 0, SEEK_END);
        std::size_t fileLen = ftell(file);
        std::fseek(file, 0, SEEK_SET);

        std::size_t outSize = out->size();
        out->resize(outSize + fileLen);
        std::size_t read = std::fread(out->data() + outSize, 1, fileLen, file);
        ASSERT(read == fileLen);

        std::fclose(file);
        return true;
    }

    return false;
}

int GffExample(char* path);
void GffExamplePrintVarsAndTag(FileFormats::Gff::Friendly::Gff const& gff);
void GffExamplePrintGff_r(FileFormats::Gff::Friendly::GffStruct const& element, int depth = 0);

int GffExample(char* path)
{
    std::vector<std::byte> gffData;
    bool file = ReadAllButes(path, &gffData);

    if (!file)
    {
        std::printf("Failed to open file %s.\n", path);
        return 1;
    }

    FileFormats::Gff::Raw::Gff rawGff;

    // Construct a raw Gff file from the loaded bytes.
    // This isn't something that a normal user would want to traverse - this is something that
    // advanced users may wish to play with, however.
    bool loaded = FileFormats::Gff::Raw::Gff::ReadFromBytes(gffData.data(), &rawGff);

    std::printf("GFF FileType: %.4s\n", rawGff.m_Header.m_FileType);
    std::printf("GFF FileVersion: %.4s\n", rawGff.m_Header.m_FileVersion);

    if (!loaded)
    {
        std::printf("Failed to load the GFF file. Check the FileType and FileVersion and ensure the file is well formed.\n");
        return 1;
    }

    // Construct a friendly Gff file from the raw file we loaded earlier.
    // This provides a much more user friendly interface around the Gff and does not expose
    // implementation details nor require an advanced understanding of the format to use.
    FileFormats::Gff::Friendly::Gff gff(rawGff);

    std::printf("\nPrinting tags and variables.\n");
    GffExamplePrintVarsAndTag(gff);

    std::printf("\nPrinting complete Gff data structure.\n");
    GffExamplePrintGff_r(gff.GetTopLevelStruct());

    return 0;
}

using namespace FileFormats::Gff;
using namespace FileFormats::Gff::Friendly;

void GffExamplePrintVarsAndTag(Gff const& gff)
{
    Type_CExoString tag;
    if (gff.GetTopLevelStruct().ReadField("Tag", &tag))
    {
        std::printf("\nTag: %s\n", tag.m_String.c_str());
    }

    Type_List varTable;
    if (gff.GetTopLevelStruct().ReadField("VarTable", &varTable))
    {
        std::vector<Type_Struct> const& entries = varTable.GetStructs();

        for (std::size_t i = 0; i < entries.size(); ++i)
        {
            GffStruct const& entry = entries[i];
            std::printf("\nVariable #%llu\n", i);

            Type_CExoString name;
            entry.ReadField("Name", &name);
            std::printf(" Name: %s\n", name.m_String.c_str());

            Type_DWORD type;
            entry.ReadField("Type", &type);
            std::printf(" Type: %u\n", type);

            if (type == 1)
            {
                Type_INT value;
                entry.ReadField("Value", &value);
                std::printf(" Value: %i\n", value);
            }
            else if (type == 2)
            {
                Type_FLOAT value;
                entry.ReadField("Value", &value);
                std::printf(" Value: %f\n", value);
            }
            else if (type == 3)
            {
                Type_CExoString value;
                entry.ReadField("Value", &value);
                std::printf(" Value: %s\n", value.m_String.c_str());
            }
            else if (type == 4)
            {
                Type_DWORD value;
                entry.ReadField("Value", &value);
                std::printf(" Value: %x\n", value);
            }
            else if (type == 5)
            {
                std::printf(" Value: <location>\n");
            }
            else
            {
                std::printf(" Value: <unknown>\n");
                ASSERT_FAIL();
            }
        }
    }
}

void GffExamplePrintGff_r(GffStruct const& element, int depth)
{
    char lineBuffer[512];

    if (depth == 0)
    {
        std::printf("\nRoot");
        ++depth;
    }

    for (auto const& kvp : element.GetFields())
    {
        int start = std::sprintf(lineBuffer, "\n%*c%s: ", depth, ' ', kvp.first.c_str());

        // kvp.first = field name
        // kvp.second.first = Raw::GffField::Type
        // kvp.second.second = std::any (we don't care about this one as ReadField will extract for us.

        if (kvp.second.first == Raw::GffField::BYTE)
        {
            Type_BYTE value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[BYTE] %u", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::CHAR)
        {
            Type_CHAR value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[CHAR] %c", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::WORD)
        {
            Type_WORD value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[WORD] %u", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::SHORT)
        {
            Type_SHORT value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[SHORT] %d", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::DWORD)
        {
            Type_DWORD value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[DWORD] %u", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::INT)
        {
            Type_INT value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[INT] %d", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::DWORD64)
        {
            Type_DWORD64 value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[DWORD64] %llu", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::INT64)
        {
            Type_INT64 value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[INT64] %lld", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::FLOAT)
        {
            Type_FLOAT value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[FLOAT] %f", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::DOUBLE)
        {
            Type_DOUBLE value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[DOUBLE] %f", value);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::CExoString)
        {
            Type_CExoString value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[CExoString] %s", value.m_String.c_str());
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::ResRef)
        {
            Type_CResRef value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[ResRef] %.*s", value.m_Size, value.m_String);
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::CExoLocString)
        {
            Type_CExoLocString value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[CExoLocString] StrRef: %u, SubString count: %zu", value.m_StringRef, value.m_SubStrings.size());
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::VOID)
        {
            Type_VOID value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[VOID] Binary size: %zu", value.m_Data.size());
            std::printf(lineBuffer);
        }
        else if (kvp.second.first == Raw::GffField::Struct)
        {
            Type_Struct value;
            element.ReadField(kvp, &value);
            std::sprintf(lineBuffer + start, "[Struct] Field count: %zu", value.GetFields().size());
            std::printf(lineBuffer);

            GffExamplePrintGff_r(value, depth + 1);
        }
        else if (kvp.second.first == Raw::GffField::List)
        {
            Type_List value;
            element.ReadField(kvp, &value);

            std::vector<GffStruct> const& structs = value.GetStructs();
            std::sprintf(lineBuffer + start, "[List] Struct count: %zu", structs.size());
            std::printf(lineBuffer);

            for (std::size_t i = 0; i < structs.size(); ++i)
            {
                std::printf("\n%*c%s #%zu:", depth, ' ', kvp.first.c_str(), i);
                GffExamplePrintGff_r(structs[i], depth + 1);
            }
        }
        else
        {
            ASSERT_FAIL_MSG("Unknown Gff type: %d", kvp.second.first);
        }
    }
}

}

int main(int argc, char** argv)
{
    ASSERT(argc == 2);
    return GffExample(argv[1]);
}