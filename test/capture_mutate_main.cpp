// gfxrecon-capture-mutate: write a damaged copy of a capture, for a look at what a tool does
// with it by hand. The test cases use the same function.

#include "capture_mutations.h"

#include <cstdio>
#include <string>

int main(int argc, const char** argv)
{
    if (argc != 4)
    {
        std::printf("Usage: gfxrecon-capture-mutate <mutation> <source.gfxr> <destination.gfxr>\n\nMutations:\n");
        for (CaptureMutation mutation : all_capture_mutations())
        {
            std::printf("  %s\n", to_string(mutation));
        }
        return 2;
    }
    CaptureMutation mutation{};
    if (!capture_mutation_from_string(argv[1], mutation))
    {
        std::fprintf(stderr, "Unknown mutation \"%s\". Run with no arguments for the list.\n", argv[1]);
        return 2;
    }
    std::string error;
    if (!write_mutated_capture(mutation, argv[2], argv[3], error))
    {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    return 0;
}
