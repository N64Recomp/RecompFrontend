#include "program_config.h"

static std::string program_name = "Funny recompiled game";

namespace recompui {
    namespace programconfig {
        void set_program_name(const std::string& name) {
            program_name = name;
        }

        const std::string &get_program_name() {
            return program_name;
        }
    }
}
