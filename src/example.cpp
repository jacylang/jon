#include "jon/jon.h"

using jon = jacylang::jon;
using namespace jacylang::literal;

const static auto config = R"(
extensions: ['jc']
default-command: 'compile'
arg-delimiters: '=,'
bool-values: {
    true: [
        'yes'
        'y'
        'true'
        '1'
        'on'
    ]
    false: [
        'no'
        'n'
        'false'
        '0'
        'off'
    ]
}
common-flags: [
    {
        name: 'help'
        type: 'bool'
        aliases: ['h']
    }
]
commands: [
    {
        name: 'compile'
        flags: [
            {
                name: 'print'
                type: 'string'
                description: 'Debug option that prints different intermediate representations'
                duplicates: 'merge'
                values: [
                    'dir-tree'
                    'tokens'
                    'ast'
                    'sugg'
                    'source'
                    'mod-tree'
                    'ast-names'
                    'ast-node-map'
                    'ribs'
                    'resolutions'
                    'definitions'
                    'all'
                ]
            }
            {
                name: 'compile-depth'
                type: 'string'
                values: [
                    'parser'
                    'name-resolution'
                    'lowering'
                ]
            }
            {
                name: 'dev'
                type: 'bool'
            }
            {
                name: 'log-level'
                type: 'string'
                values: [
                    'dev'
                    'debug'
                    'info'
                    'warn'
                    'error'
                ]
            }
        ]
    }
]
    )"_jon;

int main(const int, const char**) {
    using namespace jacylang::literal;

    std::cout << config.strAt("default-command") << std::endl;

    return 0;
}
