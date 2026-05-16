#include <cstdio>

namespace quine
{
    [[nodiscard]] const char* source()
    {
        return "#include <cstdio>%c%cnamespace quine%c{%c    [[nodiscard]] const char* source()%c    {%c        return %c%s%c;%c    }%c}%c%cint main()%c{%c    std::printf(quine::source(), 10, 10, 10, 10, 10, 10, 34, quine::source(), 34, 10, 10, 10, 10, 10, 10, 10, 10, 10);%c    return 0;%c}%c";
    }
}

int main()
{
    std::printf(
        quine::source(),
        10,
        10,
        10,
        10,
        10,
        10,
        34,
        quine::source(),
        34,
        10,
        10,
        10,
        10,
        10,
        10,
        10,
        10,
        10);
    return 0;
}