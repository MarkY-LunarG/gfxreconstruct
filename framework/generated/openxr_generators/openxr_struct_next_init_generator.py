#!/usr/bin/python3 -i
#
# Copyright (c) 2018 Valve Corporation
# Copyright (c) 2018-2024 LunarG, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import sys
from collections import OrderedDict
from base_generator import BaseGenerator, BaseGeneratorOptions, write


class OpenXrStructNextInitGeneratorOptions(BaseGeneratorOptions):
    """Adds the following new option:
    is_override - Specify whether the member function declarations are
                  virtual function overrides or pure virtual functions.
    Options for generating C++ class declarations for OpenXR parameter processing.
    """

    def __init__(
        self,
        blacklists=None,  # Path to JSON file listing apicalls and structs to ignore.
        platform_types=None,  # Path to JSON file listing platform (WIN32, X11, etc.) defined types.
        filename=None,
        directory='.',
        prefix_text='',
        protect_file=False,
        protect_feature=True,
        extraOpenXrHeaders=[]
    ):
        BaseGeneratorOptions.__init__(
            self,
            blacklists,
            platform_types,
            filename,
            directory,
            prefix_text,
            protect_file,
            protect_feature,
            extraOpenXrHeaders=extraOpenXrHeaders
        )


class OpenXrStructNextInitGenerator(BaseGenerator):
    """OpenXrStructNextInitGenerator - subclass of BaseGenerator.
    Generates C++ member declarations for a utility header for initializng the 'next' structure chains.
    """

    def __init__(
        self, err_file=sys.stderr, warn_file=sys.stderr, diag_file=sys.stdout
    ):
        BaseGenerator.__init__(
            self,
            process_cmds=False,
            process_structs=True,
            feature_break=False,
            err_file=err_file,
            warn_file=warn_file,
            diag_file=diag_file
        )

        # Map of struct types to associated XrStructureType.
        self.type_values = dict()

        self.SKIP_NEXT_STRUCT_TYPES = [
            'XR_TYPE_BASE_IN_STRUCTURE', 'XR_TYPE_BASE_OUT_STRUCTURE'
        ]

    def beginFile(self, gen_opts):
        """Method override."""
        BaseGenerator.beginFile(self, gen_opts)

        write('#include "decode/struct_pointer_decoder.h"', file=self.outFile)
        write(
            '#include "generated/generated_openxr_struct_decoders.h"',
            file=self.outFile
        )
        write('#include "util/defines.h"', file=self.outFile)
        self.newline()
        write('GFXRECON_BEGIN_NAMESPACE(gfxrecon)', file=self.outFile)
        write('GFXRECON_BEGIN_NAMESPACE(decode)', file=self.outFile)
        self.newline()

    def endFile(self):
        """Method override."""
        self.newline()
        write(
            'static void InitializeOutputStructNextImpl(const XrBaseInStructure* in_next, XrBaseOutStructure* output_struct)',
            file=self.outFile
        )
        write('{', file=self.outFile)
        write('    while(in_next)', file=self.outFile)
        write('    {', file=self.outFile)
        write('        switch(in_next->type)', file=self.outFile)
        write('        {', file=self.outFile)
        for struct in self.type_values:
            struct_type = self.type_values[struct]
            if not struct_type in self.SKIP_NEXT_STRUCT_TYPES:
                write(
                    '            case {}:'.format(struct_type),
                    file=self.outFile
                )
                write('            {', file=self.outFile)
                write(
                    '                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<{}>());'
                    .format(struct),
                    file=self.outFile
                )
                write('                break;', file=self.outFile)
                write('            }', file=self.outFile)
        write('            default:', file=self.outFile)
        write('                break;', file=self.outFile)
        write('        }', file=self.outFile)
        write(
            '        output_struct = output_struct->next;', file=self.outFile
        )
        write(
            '        output_struct->type = in_next->type;', file=self.outFile
        )
        write('        in_next = in_next->next;', file=self.outFile)
        write('    }', file=self.outFile)
        write('}', file=self.outFile)

        self.newline()
        write('template <typename T>', file=self.outFile)
        write(
            'void InitializeOutputStructNext(StructPointerDecoder<T> *decoder)',
            file=self.outFile
        )
        write('{', file=self.outFile)
        write('    if(decoder->IsNull()) return;', file=self.outFile)
        write(
            '    size_t len = decoder->GetOutputLength();', file=self.outFile
        )
        write('    auto input = decoder->GetPointer();', file=self.outFile)
        write(
            '    auto output = decoder->GetOutputPointer();',
            file=self.outFile
        )
        write('    for( size_t i = 0 ; i < len; ++i )', file=self.outFile)
        write('    {', file=self.outFile)
        write(
            '        const auto* in_next = reinterpret_cast<const XrBaseInStructure*>(input[i].next);',
            file=self.outFile
        )
        write('        if( in_next == nullptr ) continue;', file=self.outFile)
        write(
            '        auto* output_struct = reinterpret_cast<XrBaseOutStructure*>(&output[i]);',
            file=self.outFile
        )
        write(
            '        InitializeOutputStructNextImpl(in_next, output_struct);',
            file=self.outFile
        )
        write('    }', file=self.outFile)
        write('}', file=self.outFile)

        self.newline()
        write('GFXRECON_END_NAMESPACE(decode)', file=self.outFile)
        write('GFXRECON_END_NAMESPACE(gfxrecon)', file=self.outFile)

        # Finish processing in superclass
        BaseGenerator.endFile(self)

    #
    # Indicates that the current feature has C++ code to generate.
    def need_feature_generation(self):
        if self.feature_struct_members:
            return True
        return False

    def genStruct(self, typeinfo, typename, alias):
        """Method override."""
        BaseGenerator.genStruct(self, typeinfo, typename, alias)

        if not alias:
            struct_type = self.make_structure_type_enum(typeinfo, typename)
            if struct_type and 'BASE_HEADER' not in struct_type:
                self.type_values[typename] = struct_type
