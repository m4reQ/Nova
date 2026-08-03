#include <Nova/input/InputSystem.hpp>
#include <cwctype>

void Nova::InputSystem::BeginTextInput() noexcept
{
    if (!textInputEnabled_)
    {
        textBuffer_.clear();
        textInputEnabled_ = true;
    }
}

std::wstring Nova::InputSystem::EndTextInput() noexcept
{
    const std::wstring result(textBuffer_.begin(), textBuffer_.end());

    textInputEnabled_ = false;
    textBuffer_.clear();

    return result;
}

void Nova::InputSystem::AppendTextChar(wchar_t character) noexcept
{
    if (textInputEnabled_)
    {
        if (character == L'\b')
        {
            if (textBuffer_.size() > 0)
                textBuffer_.pop_back();

            return;
        }

        if (std::iswprint(character))
            textBuffer_.emplace_back(character);
    }
}
