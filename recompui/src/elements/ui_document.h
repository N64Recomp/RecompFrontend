#pragma once
#include "core/ui_context.h"
#include "elements/ui_element.h"

namespace recompui {
    class Element;
    class ContextId;

    class Document : public Element {
    friend class ContextId;
    protected:
        std::string_view get_type_name() override { return "Document"; }
    public:
        Document(Rml::Element *base);
    };

} // namespace recompui
